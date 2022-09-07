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
    //--------< 任意の点に向かってRGBシフト >--------//
    // r値
    float2 sample_point_r = pin.texcoord;
    float2 vec_r = rgb_shift_target_point - sample_point_r;
    sample_point_r.x += vec_r.x * rgb_shift_zoom_power;
    sample_point_r.y += vec_r.y * rgb_shift_zoom_power;
    tex.r = texture_maps[0].Sample(sampler_states[LINEAR_BORDER_BLACK], sample_point_r).r;
    // g値
    float2 sample_point_g = pin.texcoord;
    float2 vec_g = rgb_shift_target_point - sample_point_g;
    sample_point_g.x += vec_g.x * rgb_shift_zoom_power * 0.5f;
    sample_point_g.y += vec_g.y * rgb_shift_zoom_power * 0.5f;
    tex.g = texture_maps[0].Sample(sampler_states[LINEAR_BORDER_BLACK], sample_point_g).g;

    return float4(tex.rgb, alpha);
}