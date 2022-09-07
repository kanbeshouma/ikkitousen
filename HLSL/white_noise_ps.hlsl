#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);


float white_noise(float2 coord)
{
    // フラクトサイン法
    return frac(sin(dot(coord, float2(8.7819, 3.255))) * 437.645);
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float noise = white_noise(pin.texcoord * white_noise_time) - 0.5;
    tex.rgb += float3(noise, noise, noise);
    return tex;
}