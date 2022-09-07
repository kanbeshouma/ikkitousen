#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps : register(t0);

Texture2D depth_map : register(t5);
SamplerState depth_sampler_state : register(s5);

//--------------------------------------------
//	フォグ
//--------------------------------------------
//color:現在のピクセル色
//fog_color:フォグの色
//fog_range:フォグの範囲情報
//eye_length:視点からの距離
float4 CalcFog(in float4 color, float4 fog_color, float2 fog_range, float eye_length)
{
    float fogAlpha = saturate((eye_length - fog_range.x) / (fog_range.y - fog_range.x));
    return lerp(color, fog_color, pow(fogAlpha, 0.9f));
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float scene_depth = depth_map.Sample(sampler_states[POINT], pin.texcoord).x;
    float4 tex = texture_maps.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float4 ndc_position;
    //texture space to ndc
    ndc_position.x = pin.texcoord.x * +2 - 1;
    ndc_position.y = pin.texcoord.y * -2 + 1;
    ndc_position.z = scene_depth;
    ndc_position.w = 1;
    //ndc to world space
    float4 world_position = mul(ndc_position, inverse_view_projection);
    world_position = world_position / world_position.w;

    // Adapt mist effects.
    tex = CalcFog(tex, fog_color, fog_range.xy, length(world_position.xyz - camera_position.xyz));

    return tex;
}