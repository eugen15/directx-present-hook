// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <DirectXMath.h>
#include <vector>

#include "d3d11-base-helper.h"

// A helper class which uses DirectX 12 to draw some pictures in a window.
class D3D11Renderer : public D3D11BaseHelper {
public:
  D3D11Renderer();
  ~D3D11Renderer() override;

  // Initializes the object.
  HRESULT Initialize(HWND windowHandle,
    UINT viewportWidth, UINT viewportHeight) override;

  // Unitializes the object.
  void Uninitialize() override;

  HRESULT Render();

private:

  struct Vertex final {
    DirectX::XMFLOAT3 position_;
    DirectX::XMFLOAT2 uv_;
  };

  HRESULT PrepareVertices(ID3D11Buffer** vertexBuffer);

  HRESULT CreateRenderTargetView();
  HRESULT CreateMiscAssets();

  HRESULT CreatePicture();

  static constexpr int NumVertices = 6;

  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

  // Misc assets
  Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;
  Microsoft::WRL::ComPtr<ID3D11BlendState> blendState_;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;

  const UINT pictureWidth_ = 256;
  const UINT pictureHeight_ = 256;

  FLOAT pictureLeft_ = 0;
  const FLOAT pictureTop_ = 200;

  // Picture texture
  D3D11_TEXTURE2D_DESC pictureTextureDesc_ = {};
  Microsoft::WRL::ComPtr<ID3D11Texture2D> pictureTexture_;

  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc_;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
};
