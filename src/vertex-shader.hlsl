// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

struct VertexShaderInput
{
  float4 pos : POSITION;
  float2 tex : TEXCOORD;
};

struct VertexShaderOutput
{
  float4 pos : SV_POSITION;
  float2 tex : TEXCOORD;
};

VertexShaderOutput VS(VertexShaderInput input)
{
  return input;
}