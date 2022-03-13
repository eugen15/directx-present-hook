// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

Texture2D tx : register(t0);
SamplerState samLinear : register(s0);

struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float2 tex : TEXCOORD;
};

float4 PS(PixelShaderInput input) : SV_Target
{
    return tx.Sample(samLinear, input.tex);
}
