#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    //--------<‰æ–Ê‚Ìã‰º‚ð‚¸‚ç‚·>--------//
    // r’l‚ð‰¡•ûŒü‚É‚¸‚ç‚·
    float2 sample_point = pin.texcoord;
    if (pin.texcoord.y > 0.5f)
    {
        sample_point.x += slashing_power;
    }
    else
    {
        sample_point.x -= slashing_power;
    }
    float4 tex = texture_maps[0].Sample(sampler_states[POINT], sample_point);
    float alpha = tex.a;

    return float4(tex.rgb, alpha);
}