// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>

#include <wrl/client.h>

// A helper class to create a DirectX 11 device
// and swap chain for a window.
class D3D11BaseHelper {
public:
  D3D11BaseHelper();
  virtual ~D3D11BaseHelper();

  // Initializes the object.
  virtual HRESULT Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight);

  // Unitializes the object.
  virtual void Uninitialize();

protected:
  HRESULT CreateDevice();
  HRESULT CreateSwapChain();

  HWND windowHandle_ = NULL;
  UINT viewportWidth_ = 0;
  UINT viewportHeight_ = 0;

  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext_;

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc1_ = {};
  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1_;
};