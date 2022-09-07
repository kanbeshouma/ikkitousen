#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D color_map : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    //----zoom blur----//
    float2 uv        = pin.texcoord;
    float2 focus     = uv - reference_position.xy;
    float4 out_color = { 0.0f, 0.0f, 0.0f, 1.0f };

    for (int i = 0; i < focus_detail; i++)
    {
        float power = 1.0 - zoom_power * (1.0 / 1280.0f) * float(i);
        out_color.rgb += color_map.Sample(sampler_states[ANISOTROPIC], focus * power + reference_position.xy).rgb;
    }
    out_color.rgb *= 1.0 / float(focus_detail);

    return out_color;
}