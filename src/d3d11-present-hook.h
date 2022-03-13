// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <Windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>

#include <cstdint>
#include <string>
#include <string_view>

// The example singleton class which shows how
// to hook the DXGI swap chain present method
// when DirectX 11 is used.
class D3D11PresentHook final {
public:
  static D3D11PresentHook* Get();

  // This method must be called only once!
  // There is no additional check inside.
  HRESULT Hook();

  // Captures some frames.
  HRESULT CaptureFrames(HWND windowHandleToCapture,
    std::wstring_view folderToSaveFrames, int maxFrames);

private:
  D3D11PresentHook();
  ~D3D11PresentHook();

  void CaptureFrame(IDXGISwapChain* swapChain);

  HRESULT SwapChainPresent(IDXGISwapChain* swapChain,
    UINT syncInterval, UINT flags);

  static HRESULT WINAPI SwapChainPresentWrapper(IDXGISwapChain* swapChain,
    UINT syncInterval, UINT flags);

  // Hook pointers.
  std::uint64_t presentPointer_ = 0;
  std::uint64_t presentTrampoline_ = 0;

  // Capture details.
  HWND windowHandleToCapture_ = NULL;
  std::wstring folderToSaveFrames_;
  int frameIndex_ = 0;
  int maxFrames_ = 0;  
};

