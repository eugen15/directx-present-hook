// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#include <format>
#include <iostream>

#include <polyhook2\CapstoneDisassembler.hpp>
#include <polyhook2\Detour\x86Detour.hpp>
#include <polyhook2\Detour\x64Detour.hpp>

#include "misc-helpers.h"
#include "base-window.h"
#include "d3d11-base-helper.h"
#include "d3d11-present-hook.h"

typedef HRESULT(WINAPI* D3D11PresentPointer)(
  IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

D3D11PresentHook* D3D11PresentHook::Get() {
  static D3D11PresentHook hook;
  return &hook;
}


D3D11PresentHook::D3D11PresentHook() {
  // TODO
}

D3D11PresentHook::~D3D11PresentHook() {
  // TODO
}

HRESULT D3D11PresentHook::Hook() {
  HRESULT hr;

  // A temporary window to create a sample swap chain.
  BaseWindow temporaryWindow(L"DirectX 11 Temporary Window");
  hr = temporaryWindow.Initialize(800, 600);
  if (FAILED(hr)) {
    return hr;
  }

  // A derived class to access the swap chain.
  class D3D11PresentHookHelper : public D3D11BaseHelper {
  public:
    inline IDXGISwapChain1* GetSwapChain() {
      return swapChain1_.Get();
    }
  };

  // Create and initialize the helper.
  D3D11PresentHookHelper d3d11Helper;
  hr = d3d11Helper.Initialize(temporaryWindow.GetHandle(), 800, 600);
  if (FAILED(hr)) {
    return hr;
  }

  // Pointer to the swap chain.
  std::uintptr_t* swapChainPointer =
    static_cast<std::uintptr_t*>(static_cast<void*>(d3d11Helper.GetSwapChain()));

  // Pointer to the swap chain virtual table.
  std::uintptr_t* virtualTablePointer =
    reinterpret_cast<std::uintptr_t*>(swapChainPointer[0]);

  // "Present" has index 8 because:
  // - "Present" is the first original virtual method of IDXGISwapChain.
  // - IDXGISwapChain is based on IDXGIDeviceSubObject
  //   which has 1 original virtual method (GetDevice).
  // - IDXGIDeviceSubObject is based on IDXGIObject
  //   which has 4 original virtual methods (SetPrivateData, SetPrivateDataInterface, GetPrivateData, GetParent).
  // - IDXGIObject is based on IUnknown
  //   which has 3 original virtual methods (QueryInterface, AddRef, Release). 
  // So 0 + 1 + 4 + 3 = 8.
  presentPointer_ = static_cast<std::uint64_t>(virtualTablePointer[8]);

  // This will be called instead of the original "Present".
  std::uint64_t presentCallback =
    reinterpret_cast<std::uint64_t>(&D3D11PresentHook::SwapChainPresentWrapper);

  // Initialize the hook objects.
#if defined(_M_X64)
  PLH::CapstoneDisassembler dis(PLH::Mode::x64);
  PLH::x64Detour* detour = new PLH::x64Detour(presentPointer_,
    presentCallback, &presentTrampoline_, dis);
#else
  PLH::CapstoneDisassembler dis(PLH::Mode::x86);
  PLH::x86Detour* detour = new PLH::x86Detour(presentPointer_,
    presentCallback, &presentTrampoline_, dis);
#endif

  // Hook!
  detour->hook();

  return S_OK;
}

HRESULT D3D11PresentHook::CaptureFrames(HWND windowHandleToCapture,
   std::wstring_view folderToSaveFrames, int maxFrames) {
  if (windowHandleToCapture_ != NULL) {
    return HRESULT_FROM_WIN32(ERROR_BUSY);
  }
  windowHandleToCapture_ = windowHandleToCapture;
  folderToSaveFrames_ = std::wstring(folderToSaveFrames);
  if (folderToSaveFrames_.size() &&
      *folderToSaveFrames_.rbegin() != '\\' &&
      *folderToSaveFrames_.rbegin() != '/') {
    folderToSaveFrames_ += '\\';
  }
  maxFrames_ = maxFrames;
  return S_OK;
}

void D3D11PresentHook::CaptureFrame(IDXGISwapChain* swapChain) {
  HRESULT hr;

  // In DirectX 11 there can be multiple ID3D11Device per a process,
  // so you need the one which was used to create the black box
  // window swap chain.
  Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
  hr = swapChain->GetDevice(__uuidof(ID3D11Device), &d3d11Device);
  if (FAILED(hr)) {
    return;
  }

  // You also need the correct device context.
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
  d3d11Device->GetImmediateContext(&d3d11DeviceContext);

  // Get the swap chain texture.
  Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11SwapChainTexture;
  hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
    &d3d11SwapChainTexture);
  if (FAILED(hr)) {
    return;
  }

  // Create the staging texture.
  D3D11_TEXTURE2D_DESC d3d11StagingTextureDesc = {};
  d3d11SwapChainTexture->GetDesc(&d3d11StagingTextureDesc);

  d3d11StagingTextureDesc.BindFlags = 0;
  d3d11StagingTextureDesc.MiscFlags = 0;
  d3d11StagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  d3d11StagingTextureDesc.Usage = D3D11_USAGE_STAGING;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11StagingTexture;
  hr = d3d11Device->CreateTexture2D(&d3d11StagingTextureDesc, nullptr,
    &d3d11StagingTexture);
  if (FAILED(hr)) {
    return;
  }

  // Copy the frame to the staging texture.
  d3d11DeviceContext->CopyResource(d3d11StagingTexture.Get(),
    d3d11SwapChainTexture.Get());

  // Map to read the data.
  D3D11_MAPPED_SUBRESOURCE mappedSubresource;
  UINT subresource = D3D11CalcSubresource(0, 0, 0);
  hr = d3d11DeviceContext->Map(d3d11StagingTexture.Get(), subresource,
    D3D11_MAP_READ_WRITE, 0, &mappedSubresource);
  if (FAILED(hr)) {
    return;
  }

  UINT frameRowPitch = mappedSubresource.RowPitch;
  UINT frameWidth = frameRowPitch / 4;
  UINT frameHeight = d3d11StagingTextureDesc.Height;

  // Convert the frame to the BMP format.
  std::vector<std::uint8_t> bmp = MiscHelpers::ConvertRGBAToBMP(
    reinterpret_cast<uint8_t*>(mappedSubresource.pData),
    frameWidth, frameHeight, frameRowPitch);

  // Save the BMP file.
  // On some machine you will see rendering freezes during this operation.
  // In a real application, probably, you will not need to save frames to a file
  // but just to place them to a buffer to generate a preview picture or analyze it.
  std::wstring filename = std::format(L"{}{}.bmp", folderToSaveFrames_, frameIndex_++);
  MiscHelpers::SaveDataToFile(filename, bmp.data(), bmp.size());
  
  // Stop capturing if enough frames.
  if (frameIndex_ >= maxFrames_) {
    windowHandleToCapture_ = NULL;
  }

  // Stop capturing if enough frames.
  d3d11DeviceContext->Unmap(d3d11StagingTexture.Get(), subresource);
}

HRESULT D3D11PresentHook::SwapChainPresent(IDXGISwapChain* swapChain,
  UINT syncInterval, UINT flags) {
  // Check if we need to capture the window.
  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  HRESULT hr = swapChain->GetDesc(&swapChainDesc);
  if (SUCCEEDED(hr)) {
    if (windowHandleToCapture_ == swapChainDesc.OutputWindow) {
      CaptureFrame(swapChain);
    }
  }
  // Call the original "Present".
  D3D11PresentPointer presentPtr =
    reinterpret_cast<D3D11PresentPointer>(presentPointer_);
  return PLH::FnCast(presentTrampoline_, presentPtr)(
    swapChain, syncInterval, flags);
}

HRESULT D3D11PresentHook::SwapChainPresentWrapper(IDXGISwapChain* swapChain,
  UINT syncInterval, UINT flags) {
  return D3D11PresentHook::Get()->SwapChainPresent(
    swapChain, syncInterval, flags);
}
