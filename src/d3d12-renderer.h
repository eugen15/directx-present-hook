// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include "d3d12-base-helper.h"

// A helper class which uses DirectX 12 to draw some pictures in a window.
class D3D12Renderer : public D3D12BaseHelper {
public:
  D3D12Renderer();
  ~D3D12Renderer() override;

  // Initializes the object.
  HRESULT Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight) override;

  // Unitializes the object.
  void Uninitialize() override;

  // Renders a new frame.
  HRESULT Render();

private:

  struct Vertex final {
    DirectX::XMFLOAT3 position_;
    DirectX::XMFLOAT2 uv_;
  };

  HRESULT PrepareVertices();
  HRESULT PopulateCommandList();

  HRESULT CreateRenderTargetViews();
  HRESULT CreateRootSignature();
  HRESULT CreatePipelineState();
  HRESULT CreateCommandList();

  HRESULT CreatePicture();

  HRESULT CreateSyncronizationObjects();

  HRESULT WaitForGPU();
  
  static constexpr UINT FrameBufferCount = 2;
  static constexpr UINT NumVertices = 6;  

  UINT swapChainFrameIndex_ = 0;

  // Render target views
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderTargetViewHeap_;
  Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetViews_[2];
  UINT rtvDescriptorSize_ = 0;

  Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

  // Picture
  const UINT pictureWidth_ = 256;
  const UINT pictureHeight_ = 256;  
  FLOAT pictureLeft_ = 0;
  const FLOAT pictureTop_ = 200;  
  
  Microsoft::WRL::ComPtr<ID3D12Resource> pictureTexture_;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> shaderResourceViewHeap_;

  // Vertex buffer
  Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

  // Synchronization objects.
  Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
  UINT frameIndex_;
  HANDLE fenceEvent_;  
  UINT64 fenceValue_;
  
  // Misc
  D3D12_VIEWPORT viewport_ = {};
  D3D12_RECT scissorRect_ = {};
};
