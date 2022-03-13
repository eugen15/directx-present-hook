// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#include <Windows.h>

#include "pixel-shader.h"
#include "vertex-shader.h"

#include "misc-helpers.h"
#include "d3d12-renderer.h"


D3D12Renderer::D3D12Renderer() {
  // TODO
}

D3D12Renderer::~D3D12Renderer() {
  Uninitialize();
}

HRESULT D3D12Renderer::Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight) {
  HRESULT hr;

  hr = D3D12BaseHelper::Initialize(windowHandle, viewportWidth, viewportHeight);
  if (FAILED(hr)) {
    return hr;
  }

  swapChainFrameIndex_ = swapChain_->GetCurrentBackBufferIndex();

  viewport_.TopLeftX = 0;
  viewport_.TopLeftY = 0;
  viewport_.Width = static_cast<FLOAT>(viewportWidth);
  viewport_.Height = static_cast<FLOAT>(viewportHeight);
  viewport_.MinDepth = D3D12_MIN_DEPTH;
  viewport_.MaxDepth = D3D12_MAX_DEPTH;

  scissorRect_.left = 0;
  scissorRect_.top = 0;
  scissorRect_.right = static_cast<LONG>(viewportWidth);
  scissorRect_.bottom = static_cast<LONG>(viewportHeight);

  hr = CreateRenderTargetViews();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreateRootSignature();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreatePipelineState();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreateCommandList();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreatePicture();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreateSyncronizationObjects();
  if (FAILED(hr)) {
    return hr;
  }

  // Wait until assets have been uploaded to the GPU.
  hr = WaitForGPU();
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

void D3D12Renderer::Uninitialize() {

}

HRESULT D3D12Renderer::Render() {
  HRESULT hr;

  hr = PrepareVertices();
  if (FAILED(hr)) {
    return hr;
  }

  hr = PopulateCommandList();
  if (FAILED(hr)) {
    return hr;
  }

  ID3D12CommandList* ppCommandLists[] = {commandList_.Get()};
  commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  hr = swapChain_->Present(1, 0);
  if (FAILED(hr)) {
    return hr;
  }

  hr = WaitForGPU();
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

HRESULT D3D12Renderer::PrepareVertices() {
  HRESULT hr;

  Vertex vertices[NumVertices] = {
    { DirectX::XMFLOAT3(-1.0f, -1.0f, 0), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-1.0f, 1.0f, 0), DirectX::XMFLOAT2(0.0f, 0.0f) },
    { DirectX::XMFLOAT3(1.0f, -1.0f, 0), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(1.0f, -1.0f, 0), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-1.0f, 1.0f, 0), DirectX::XMFLOAT2(0.0f, 0.0f) },
    { DirectX::XMFLOAT3(1.0f, 1.0f, 0), DirectX::XMFLOAT2(1.0f, 0.0f) },
  };

  FLOAT pictureWidth = static_cast<FLOAT>(pictureWidth_);
  FLOAT pictureHeight = static_cast<FLOAT>(pictureHeight_);

  FLOAT viewportCenterX = static_cast<FLOAT>(viewportWidth_ / 2);
  FLOAT viewportCenterY = static_cast<FLOAT>(viewportHeight_ / 2);

  vertices[0].position_.x = (pictureLeft_ - viewportCenterX) / viewportCenterX;
  vertices[0].position_.y = -1 * ((pictureTop_ + pictureHeight) - viewportCenterY) / viewportCenterY;
  vertices[1].position_.x = (pictureLeft_ - viewportCenterX) / viewportCenterX;
  vertices[1].position_.y = -1 * (pictureTop_ - viewportCenterY) / viewportCenterY;
  vertices[2].position_.x = ((pictureLeft_ + pictureWidth) - viewportCenterX) / viewportCenterX;
  vertices[2].position_.y = -1 * ((pictureTop_ + pictureHeight) - viewportCenterY) / viewportCenterY;
  vertices[3].position_.x = vertices[2].position_.x;
  vertices[3].position_.y = vertices[2].position_.y;
  vertices[4].position_.x = vertices[1].position_.x;
  vertices[4].position_.y = vertices[1].position_.y;
  vertices[5].position_.x = ((pictureLeft_ + pictureWidth) - viewportCenterX) / viewportCenterX;
  vertices[5].position_.y = -1 * (pictureTop_ - viewportCenterY) / viewportCenterY;

  const UINT vertexBufferSize = sizeof(vertices);

  D3D12_HEAP_PROPERTIES heapProperties = {};
  heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
  heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heapProperties.CreationNodeMask = 1;
  heapProperties.VisibleNodeMask = 1;

  D3D12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
  hr = device_->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer_));
  if (FAILED(hr)) {
    return hr;
  }

  // Copy the triangle data to the vertex buffer.
  UINT8* vertexDataBegin = nullptr;
  CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
  hr = vertexBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&vertexDataBegin));
  if (FAILED(hr)) {
    return hr;
  }

  std::memcpy(vertexDataBegin, vertices, sizeof(vertices));
  vertexBuffer_->Unmap(0, nullptr);

  // Initialize the vertex buffer view.
  vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
  vertexBufferView_.StrideInBytes = sizeof(Vertex);
  vertexBufferView_.SizeInBytes = vertexBufferSize;

  // Update coordinates
  pictureLeft_ += 5;
  if (pictureLeft_ > static_cast<FLOAT>(viewportWidth_)) {
    pictureLeft_ = -static_cast<FLOAT>(pictureWidth_);
  }

  return hr;
}

HRESULT D3D12Renderer::PopulateCommandList() {
  HRESULT hr;

  hr = commandAllocator_->Reset();
  if (FAILED(hr)) {
    return hr;
  }

  hr = commandList_->Reset(commandAllocator_.Get(), pipelineState_.Get());
  if (FAILED(hr)) {
    return hr;
  }

  commandList_->SetGraphicsRootSignature(rootSignature_.Get());

  ID3D12DescriptorHeap* ppHeaps[] = { shaderResourceViewHeap_.Get() };
  commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

  commandList_->SetGraphicsRootDescriptorTable(0,
    shaderResourceViewHeap_->GetGPUDescriptorHandleForHeapStart());
  commandList_->RSSetViewports(1, &viewport_);
  commandList_->RSSetScissorRects(1, &scissorRect_);

  // Indicate that the back buffer will be used as a render target.
  D3D12_RESOURCE_BARRIER rtv1Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
    renderTargetViews_[frameIndex_].Get(), D3D12_RESOURCE_STATE_PRESENT,
    D3D12_RESOURCE_STATE_RENDER_TARGET);
  commandList_->ResourceBarrier(1, &rtv1Barrier);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    renderTargetViewHeap_->GetCPUDescriptorHandleForHeapStart(), frameIndex_, rtvDescriptorSize_);
  commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

  // Record commands.
  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
  commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
  commandList_->DrawInstanced(6, 1, 0, 0);

  // Indicate that the back buffer will now be used to present.
  D3D12_RESOURCE_BARRIER rtv2Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
    renderTargetViews_[frameIndex_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT);
  commandList_->ResourceBarrier(1, &rtv2Barrier);

  hr = commandList_->Close();
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

HRESULT D3D12Renderer::CreateRenderTargetViews() {
  HRESULT hr;

  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = FrameBufferCount;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr = device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&renderTargetViewHeap_));
  if (FAILED(hr)) {
    return hr;
  }

  rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    renderTargetViewHeap_->GetCPUDescriptorHandleForHeapStart());

  // Create a RTV for each frame.
  for (UINT i = 0; i < FrameBufferCount; ++i) {
    hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&renderTargetViews_[i]));
    if (FAILED(hr)) {
      return hr;
    }

    device_->CreateRenderTargetView(renderTargetViews_[i].Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, rtvDescriptorSize_);
  }

  return hr;
}

HRESULT D3D12Renderer::CreateRootSignature() {
  HRESULT hr;

  D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
  // This is the highest version the sample supports.
  // If CheckFeatureSupport succeeds, the HighestVersion
  // returned will not be greater than this.
  featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
  hr = device_->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
    &featureData, sizeof(featureData));
  if (FAILED(hr)) {
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    hr = S_OK;
  }

  CD3DX12_DESCRIPTOR_RANGE1 ranges[1] = {};
  ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,
    D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

  CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
  rootParameters[0].InitAsDescriptorTable(1, &ranges[0],
    D3D12_SHADER_VISIBILITY_PIXEL);

  D3D12_STATIC_SAMPLER_DESC sampler = {};
  sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.MipLODBias = 0;
  sampler.MaxAnisotropy = 0;
  sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
  sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
  sampler.MinLOD = 0.0f;
  sampler.MaxLOD = D3D12_FLOAT32_MAX;
  sampler.ShaderRegister = 0;
  sampler.RegisterSpace = 0;
  sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1,
    &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  Microsoft::WRL::ComPtr<ID3DBlob> signature;
  Microsoft::WRL::ComPtr<ID3DBlob> error;
  hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
    featureData.HighestVersion, &signature, &error);
  if (FAILED(hr)) {
    return hr;
  }

  hr = device_->CreateRootSignature(0, signature->GetBufferPointer(),
    signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
  if (FAILED(hr)) {
    return hr;
  }  

  return hr;
}

HRESULT D3D12Renderer::CreatePipelineState() {
  HRESULT hr;

  D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
  pipelineStateDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
  pipelineStateDesc.pRootSignature = rootSignature_.Get();
  pipelineStateDesc.VS = {g_VS, sizeof(g_VS)};
  pipelineStateDesc.PS = {g_PS, sizeof(g_PS)};
  pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
  pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
  pipelineStateDesc.SampleMask = UINT_MAX;
  pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pipelineStateDesc.NumRenderTargets = 1;
  pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  pipelineStateDesc.SampleDesc.Count = 1;
  hr = device_->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

HRESULT D3D12Renderer::CreateCommandList() {
  HRESULT hr;

  hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
    IID_PPV_ARGS(&commandAllocator_));
  if (FAILED(hr)) {
    return hr;
  }

  hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
    commandAllocator_.Get(), pipelineState_.Get(), IID_PPV_ARGS(&commandList_));
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

HRESULT D3D12Renderer::CreatePicture() {
  HRESULT hr;

  D3D12_RESOURCE_DESC textureDesc = {};
  textureDesc.MipLevels = 1;
  textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  textureDesc.Width = pictureWidth_;
  textureDesc.Height = pictureHeight_;
  textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
  textureDesc.DepthOrArraySize = 1;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.SampleDesc.Quality = 0;
  textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

  D3D12_HEAP_PROPERTIES textureHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  hr = device_->CreateCommittedResource(&textureHeapProperties, D3D12_HEAP_FLAG_NONE,
    &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pictureTexture_));
  if (FAILED(hr)) {
    return hr;
  }

  const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pictureTexture_.Get(), 0, 1);

  // Create the GPU upload buffer.
  D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
  Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;
  hr = device_->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
    &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap));
  if (FAILED(hr)) {
    return hr;
  }

  std::vector<UINT8> picture = MiscHelpers::GenerateSquareRGBAPicture(pictureWidth_);

  D3D12_SUBRESOURCE_DATA textureData = {};
  textureData.pData = picture.data();
  textureData.RowPitch = pictureWidth_ * 4;
  textureData.SlicePitch = textureData.RowPitch * pictureHeight_;

  UpdateSubresources(commandList_.Get(), pictureTexture_.Get(),
    textureUploadHeap.Get(), 0, 0, 1, &textureData);

  D3D12_RESOURCE_BARRIER textureTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
    pictureTexture_.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  commandList_->ResourceBarrier(1, &textureTransitionBarrier);

  // Describe and create a shader resource view heap for the texture.
  D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
  srvHeapDesc.NumDescriptors = 1;
  srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  hr = device_->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&shaderResourceViewHeap_));
  if (FAILED(hr)) {
    return hr;
  }

  // Describe and create a shader resource view for the texture.
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = textureDesc.Format;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  device_->CreateShaderResourceView(pictureTexture_.Get(), &srvDesc,
    shaderResourceViewHeap_->GetCPUDescriptorHandleForHeapStart());

  // Close the command list and execute it to begin the initial GPU setup.
  hr = commandList_->Close();
  if (FAILED(hr)) {
    return hr;
  }

  ID3D12CommandList* ppCommandLists[] = {commandList_.Get()};
  commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  return hr;
}

HRESULT D3D12Renderer::CreateSyncronizationObjects() {
  HRESULT hr;

  hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
  if (FAILED(hr)) {
    return hr;
  }

  fenceValue_ = 1;

  fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (fenceEvent_ == nullptr) {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  return hr;
}

HRESULT D3D12Renderer::WaitForGPU() {
  HRESULT hr;

  // Signal and increment the fence value.
  const UINT64 fenceValue = fenceValue_;
  hr = commandQueue_->Signal(fence_.Get(), fenceValue);
  if (FAILED(hr)) {
    return hr;
  }

  ++fenceValue_;

  // Wait until the previous frame is finished.
  if (fence_->GetCompletedValue() < fenceValue) {
    hr = fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
    if (FAILED(hr)) {
      return hr;
    }
    WaitForSingleObject(fenceEvent_, INFINITE);
  }

  frameIndex_ = swapChain_->GetCurrentBackBufferIndex();

  return hr;

}