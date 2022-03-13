// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#include <Windows.h>

#include "pixel-shader.h"
#include "vertex-shader.h"

#include "misc-helpers.h"
#include "d3d11-renderer.h"

D3D11Renderer::D3D11Renderer() {
  // TODO
}

D3D11Renderer::~D3D11Renderer() {
  Uninitialize();
}

HRESULT D3D11Renderer::Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight) {
  HRESULT hr;
  
  hr = D3D11BaseHelper::Initialize(windowHandle, viewportWidth, viewportHeight);
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreateRenderTargetView();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreateMiscAssets();
  if (FAILED(hr)) {
    return hr;
  }

  hr = CreatePicture();
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}
void D3D11Renderer::Uninitialize() {
  // TODO
}

HRESULT D3D11Renderer::Render() {
  HRESULT hr;

  Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
  hr = PrepareVertices(&vertexBuffer);
  if (FAILED(hr)) {
    return hr;
  }  

  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  deviceContext_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);

  UINT nStride = sizeof(Vertex);
  UINT nOffset = 0;
  deviceContext_->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &nStride, &nOffset);

  FLOAT blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
  deviceContext_->OMSetBlendState(blendState_.Get(), blendFactor, 0xFFFFFFFF);

  ID3D11RenderTargetView* renderTargetViews[] = {renderTargetView_.Get()};
  deviceContext_->OMSetRenderTargets(1, renderTargetViews, nullptr);

  ID3D11ShaderResourceView* shaderResourceViews[] = {shaderResourceView_.Get()};
  deviceContext_->PSSetShaderResources(0, 1, shaderResourceViews);

  ID3D11SamplerState* samplerStates[] = {samplerState_.Get()};
  deviceContext_->PSSetSamplers(0, 1, samplerStates);

  deviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  deviceContext_->IASetInputLayout(inputLayout_.Get());
  deviceContext_->VSSetShader(vertexShader_.Get(), nullptr, 0);
  deviceContext_->PSSetShader(pixelShader_.Get(), nullptr, 0);
  
  // Draw
  deviceContext_->Draw(NumVertices, 0);

  // Present
  hr = swapChain1_->Present(1, 0);
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

HRESULT D3D11Renderer::PrepareVertices(ID3D11Buffer** vertexBuffer) {
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

  D3D11_BUFFER_DESC vertexBufferDesc = {};
  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = sizeof(Vertex) * NumVertices;
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA iniData = {};
  iniData.pSysMem = vertices;

  hr = device_->CreateBuffer(&vertexBufferDesc, &iniData, vertexBuffer);
  if (FAILED(hr)) {
    return hr;
  }

  // Update coordinates
  pictureLeft_ += 5;
  if (pictureLeft_ > static_cast<FLOAT>(viewportWidth_)) {
    pictureLeft_ = -static_cast<FLOAT>(pictureWidth_);
  }

  return hr;
}

HRESULT D3D11Renderer::CreateRenderTargetView() {
  HRESULT hr;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
  hr = swapChain1_->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
  if (FAILED(hr)) {
    return hr;
  }

  hr = device_->CreateRenderTargetView(backBuffer.Get(),
    nullptr, &renderTargetView_);
  if (FAILED(hr)) {
    return hr;
  }

  D3D11_VIEWPORT viewPort = {};
  viewPort.Width = static_cast<FLOAT>(swapChainDesc1_.Width);
  viewPort.Height = static_cast<FLOAT>(swapChainDesc1_.Height);
  viewPort.MinDepth = 0.0f;
  viewPort.MaxDepth = 1.0f;
  viewPort.TopLeftX = 0;
  viewPort.TopLeftY = 0;

  deviceContext_->RSSetViewports(1, &viewPort);

  return hr;
}

HRESULT D3D11Renderer::CreateMiscAssets() {
  HRESULT hr;

  // Sampler State
  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  hr = device_->CreateSamplerState(&samplerDesc, &samplerState_);
  if (FAILED(hr)) {
    return hr;
  }

  // Blender State
  D3D11_BLEND_DESC blendStateDesc = {};
  blendStateDesc.AlphaToCoverageEnable = FALSE;
  blendStateDesc.IndependentBlendEnable = FALSE;
  blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
  blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  hr = device_->CreateBlendState(&blendStateDesc, &blendState_);
  if (FAILED(hr)) {
    return hr;
  }

  // Shaders
  UINT size = ARRAYSIZE(g_VS);

  hr = device_->CreateVertexShader(g_VS, size, nullptr, &vertexShader_);
  if (FAILED(hr)) {
    return hr;
  }

  D3D11_INPUT_ELEMENT_DESC layout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
  };

  UINT numElements = ARRAYSIZE(layout);
  hr = device_->CreateInputLayout(layout, numElements, g_VS, size, &inputLayout_);
  if (FAILED(hr)) {
    return hr;
  }

  size = ARRAYSIZE(g_PS);
  hr = device_->CreatePixelShader(g_PS, size, nullptr, &pixelShader_);
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

HRESULT D3D11Renderer::CreatePicture() {
  std::vector<UINT8> pictureData = MiscHelpers::GenerateSquareRGBAPicture(pictureWidth_);

  D3D11_SUBRESOURCE_DATA subresourceData = {};
  subresourceData.pSysMem = pictureData.data();
  subresourceData.SysMemPitch = pictureWidth_ * 4;
  subresourceData.SysMemSlicePitch = 0;

  pictureTextureDesc_ = {};
  pictureTextureDesc_.MipLevels = 1;
  pictureTextureDesc_.ArraySize = 1;
  pictureTextureDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  pictureTextureDesc_.SampleDesc.Count = 1;
  pictureTextureDesc_.SampleDesc.Quality = 0;
  pictureTextureDesc_.Usage = D3D11_USAGE_DEFAULT;
  pictureTextureDesc_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  pictureTextureDesc_.CPUAccessFlags = 0;
  pictureTextureDesc_.MiscFlags = 0;
  pictureTextureDesc_.Width = pictureWidth_;
  pictureTextureDesc_.Height = pictureHeight_;

  HRESULT hr = device_->CreateTexture2D(&pictureTextureDesc_, &subresourceData, &pictureTexture_);
  if (FAILED(hr)) {
    return hr;
  }

  shaderResourceViewDesc_ = {};
  shaderResourceViewDesc_.Format = pictureTextureDesc_.Format;
  shaderResourceViewDesc_.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc_.Texture2D.MostDetailedMip = pictureTextureDesc_.MipLevels - 1;
  shaderResourceViewDesc_.Texture2D.MipLevels = pictureTextureDesc_.MipLevels;

  hr = device_->CreateShaderResourceView(pictureTexture_.Get(),
    &shaderResourceViewDesc_, &shaderResourceView_);
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}
