#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
SamplerState sampler_states[4] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex = texture_maps[0].Sample(sampler_states[LINEAR_BORDER_BLACK], pin.texcoord);
    float alpha = tex.a;

    if (pin.texcoord.y < wipe_threshold)
    {
        tex.rgb = float3(0, 0, 0);
    }
    if (pin.texcoord.y > (1.0f - wipe_threshold))
    {
        tex.rgb = float3(0, 0, 0);
    }

    return float4(tex.rgb, alpha);
}