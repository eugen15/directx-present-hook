// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#include "d3d12-base-helper.h"

D3D12BaseHelper::D3D12BaseHelper() {
  // TODO
}

D3D12BaseHelper::~D3D12BaseHelper() {
  Uninitialize();
}

HRESULT D3D12BaseHelper::Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight) {
  HRESULT hr;

  windowHandle_ = windowHandle;
  viewportWidth_ = viewportWidth;
  viewportHeight_ = viewportHeight;

  Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
  hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
  if (FAILED(hr)) {
    return hr;
  }

  Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
  hr = GetHardwareAdapter(factory.Get(), &hardwareAdapter, false);
  if (SUCCEEDED(hr)) {
    hr = D3D12CreateDevice(hardwareAdapter.Get(),
      D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
    if (FAILED(hr)) {
      return hr;
    }
  } else {
    Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
    hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
    if (FAILED(hr)) {
      return hr;
    }
    hr = D3D12CreateDevice(warpAdapter.Get(),
      D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
    if (FAILED(hr)) {
      return hr;
    }
  }

  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_));
  if (FAILED(hr)) {
    return hr;
  }

  // Describe and create the swap chain.
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = 2;
  swapChainDesc.Width = viewportWidth_;
  swapChainDesc.Height = viewportHeight_;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;

  Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain1;
  hr = factory->CreateSwapChainForHwnd(commandQueue_.Get(), windowHandle_,
    &swapChainDesc, nullptr, nullptr, &dxgiSwapChain1);
  if (FAILED(hr)) {
    return hr;
  }

  hr = dxgiSwapChain1.As(&swapChain_);
  if (FAILED(hr)) {
    return hr;
  }

  // This project does not support fullscreen transitions.
  hr = factory->MakeWindowAssociation(windowHandle_, DXGI_MWA_NO_ALT_ENTER);
  if (FAILED(hr)) {
    return hr;
  }

  return hr;

}

void D3D12BaseHelper::Uninitialize() {
  // TODO
}

HRESULT D3D12BaseHelper::GetHardwareAdapter(IDXGIFactory1* factory,
  IDXGIAdapter1** adapter, bool requestHighPerformanceAdapter) {
  HRESULT hr;

  Microsoft::WRL::ComPtr<IDXGIAdapter1> testAdapter;
  *adapter = nullptr;

  Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
  hr = factory->QueryInterface(IID_PPV_ARGS(&factory6));
  if (FAILED(hr)) {
    return hr;
  }

  DXGI_GPU_PREFERENCE gpuReference = (requestHighPerformanceAdapter == true) ?
    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;

  for (UINT adapterIndex = 0;
    SUCCEEDED(hr = factory6->EnumAdapterByGpuPreference(adapterIndex,
      gpuReference, IID_PPV_ARGS(&testAdapter)));
    ++adapterIndex) {
    DXGI_ADAPTER_DESC1 desc;
    testAdapter->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      // Don't select the Basic Render Driver adapter.
      // If you want a software adapter, pass in "/warp" on the command line.
      continue;
    }

    // Check to see whether the adapter supports Direct3D 12, but don't create the
    // actual device yet.
    hr = D3D12CreateDevice(testAdapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
    if (SUCCEEDED(hr)) {
      break;
    }
  }

  if (FAILED(hr)) {
    for (UINT adapterIndex = 0;
      SUCCEEDED(hr = factory->EnumAdapters1(adapterIndex, &testAdapter));
      ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      testAdapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create the
      // actual device yet.
      hr = D3D12CreateDevice(testAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
        _uuidof(ID3D12Device), nullptr);
      if (SUCCEEDED(hr)) {
        break;
      }
    }
  }

  *adapter = testAdapter.Detach();

  return hr;
}