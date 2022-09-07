#include "fullscreen_quad.hlsli"
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
texture2D color_map : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    return color_map.Sample(sampler_states[POINT], pin.texcoord);
}