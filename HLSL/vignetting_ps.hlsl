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
    float4 tex = texture_maps[0].Sample(sampler_states[ANISOTROPIC], sample_point);
    float vignette = length(float2(vignetting_pos.x, vignetting_pos.y) - pin.texcoord);
    vignette = clamp(vignette - vignetting_scope, 0, 1);
    tex.rgb -= vignette;
    return tex;
}