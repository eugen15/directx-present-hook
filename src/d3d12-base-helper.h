// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl/client.h>

#include <cstdint>

// A helper class to create a DirectX 12 device,
// command queue and swap chain for a window.
class D3D12BaseHelper {
public:
  D3D12BaseHelper();
  virtual ~D3D12BaseHelper();

  // Initializes the object.
  virtual HRESULT Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight);

  // Unitializes the object.
  virtual void Uninitialize();

protected:
  static HRESULT GetHardwareAdapter(IDXGIFactory1* factory,
    IDXGIAdapter1** adapter, bool requestHighPerformanceAdapter);

  HWND windowHandle_ = NULL;
  UINT viewportWidth_ = 0;
  UINT viewportHeight_ = 0;

  Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain_;
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
  Microsoft::WRL::ComPtr<ID3D12Device> device_;

};