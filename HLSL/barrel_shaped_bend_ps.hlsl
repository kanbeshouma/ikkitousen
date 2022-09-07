#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float2 sample_point = pin.texcoord;
    sample_point -= float2(0.5, 0.5);
    float distPower = pow(length(sample_point), barrel_shaped_distortion);
    sample_point *= float2(distPower, distPower);
    sample_point += float2(0.5, 0.5);

    return texture_maps[0].Sample(sampler_states[ANISOTROPIC], sample_point);
}