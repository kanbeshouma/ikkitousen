#include "font.hlsli"

Texture2D diffuseMap : register(t0);
SamplerState diffuseMapSamplerState : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = diffuseMap.Sample(diffuseMapSamplerState, pin.texcoord).r * pin.color;

    return color;
}