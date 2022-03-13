// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#include <vector>

#include "d3d11-base-helper.h"

D3D11BaseHelper::D3D11BaseHelper() {
  // TODO
}

D3D11BaseHelper::~D3D11BaseHelper() {
  Uninitialize();
}

HRESULT D3D11BaseHelper::Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight) {
  HRESULT hr;

  windowHandle_ = windowHandle;
  viewportWidth_ = viewportWidth;
  viewportHeight_ = viewportHeight;

  hr = CreateDevice();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreateSwapChain();
  if (FAILED(hr)) {
    return hr;
  }

  Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
  hr = device_->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);
  if (FAILED(hr)) {
    return hr;
  }

  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
  hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter);
  if (FAILED(hr)) {
    return hr;
  }

  Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
  hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &factory);
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

void D3D11BaseHelper::Uninitialize() {
  // TODO
}

HRESULT D3D11BaseHelper::CreateDevice() {
  const std::vector<D3D_DRIVER_TYPE> driverTypes = {
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_WARP,
    D3D_DRIVER_TYPE_REFERENCE,
  };

  const std::vector< D3D_FEATURE_LEVEL> featureLevels = {
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_1
  };

  D3D_FEATURE_LEVEL featureLevel;
  HRESULT hr = E_FAIL;

  for (int i = 0; i < static_cast<int>(driverTypes.size()); ++i) {
    hr = D3D11CreateDevice(
      nullptr,
      driverTypes[i],
      nullptr,
      D3D11_CREATE_DEVICE_BGRA_SUPPORT,
      &featureLevels.front(),
      static_cast<UINT>(featureLevels.size()),
      D3D11_SDK_VERSION,
      &device_,
      &featureLevel,
      &deviceContext_);
    if (SUCCEEDED(hr)) {
      break;
    }
  }

  return hr;
}

HRESULT D3D11BaseHelper::CreateSwapChain() {
  HRESULT hr;

  Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice = nullptr;
  hr = device_->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);
  if (FAILED(hr)) {
    return hr;
  }

  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
  hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter);
  if (FAILED(hr)) {
    return hr;
  }

  Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
  hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory2);
  if (FAILED(hr)) {
    return hr;
  }

  swapChainDesc1_ = {};
  swapChainDesc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  swapChainDesc1_.BufferCount = 2;
  swapChainDesc1_.Width = viewportWidth_;
  swapChainDesc1_.Height = viewportHeight_;
  swapChainDesc1_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc1_.SampleDesc.Count = 1;
  swapChainDesc1_.SampleDesc.Quality = 0;

  hr = dxgiFactory2->CreateSwapChainForHwnd(device_.Get(), windowHandle_,
    &swapChainDesc1_, nullptr, nullptr, &swapChain1_);

  return hr;
}
