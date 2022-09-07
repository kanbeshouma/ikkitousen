#include "skinned_mesh.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float ambient = 0.0f; // ŠÂ‹«Œõ(ƒ‚ƒfƒ‹‘S‘Ì‚ð–¾‚é‚­‚·‚é)
    float4 Out = float4(color.rgb + ambient, color.a);

    return Out * pin.color * light_color;
}