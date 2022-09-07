#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    // New  resolution of (nx / ny)
    float Resolution = low_resolution;
    float nx = low_resolution_number_of_divisions;
    float ny = floor(nx / Resolution);

    float2 pos;
    pos.x = floor(pin.texcoord.x * nx) / nx;
    pos.y = floor(pin.texcoord.y * ny) / ny;

    return texture_maps[0].Sample(sampler_states[ANISOTROPIC], pos);
}