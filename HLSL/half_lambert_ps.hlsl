#include "skinned_mesh.hlsli"
#include "constants.hlsli"
#include "shading_functions.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
texture2D texture_maps[4] : register(t0);

Texture2D shadow_map : register(t6);
SamplerState shadow_sampler_state : register(s6);

float3 cast_shadow(in float3 color, float depth, float3 shadow_texcoord)
{
	// [“x’l‚ð”äŠr‚µ‚Ä‰e‚©‚Ç‚¤‚©‚ð”»’è‚·‚é
    // if (pin.shadow_texcoord.z - depth > shadow_bias){}
    float light_percentage = 0;

    [unroll(50)]
    for (int Scope = 0; Scope < number_of_trials; ++Scope)
    {
        float search_width = search_width_magnification * Scope;
        float ratio = 1.0f / (number_of_trials * 4.0f);
         //----¡‚ÌƒsƒNƒZƒ‹‚ÌŽü‚è‚ª‰e‚¶‚á‚È‚©‚Á‚½‚ç‰e‚ÌF‚ð”–‚ß‚Ä‚¢‚­----//
         // ¶
        float depth_around_left = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x - search_width, shadow_texcoord.y)).r;
        if (shadow_texcoord.z - depth_around_left < shadow_bias)
        {
            light_percentage += ratio;
        }
        // ‰E
        float depth_around_right = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x + search_width, shadow_texcoord.y)).r;
        if (shadow_texcoord.z - depth_around_right < shadow_bias)
        {
            light_percentage += ratio;
        }
        // ã
        float depth_around_up = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x, shadow_texcoord.y - search_width)).r;
        if (shadow_texcoord.z - depth_around_up < shadow_bias)
        {
            light_percentage += ratio;
        }
        // ‰º
        float depth_around_down = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x, shadow_texcoord.y + search_width)).r;
        if (shadow_texcoord.z - depth_around_down < shadow_bias)
        {
            light_percentage += ratio;
        }
    }
    //color.rgb *= shadow_color.rgb * (1 + light_percentage);
    return color * light_percentage;
}


float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float depth = shadow_map.Sample(shadow_sampler_state, pin.shadow_texcoord.xy).r;
    color.rgb = cast_shadow(color.rgb, depth, pin.shadow_texcoord);

    float p = dot(pin.world_normal.xyz, -light_direction.xyz);
    p = p * 0.5f + 0.5f;
    float ambient = 0.0f; // ŠÂ‹«Œõ(ƒ‚ƒfƒ‹‘S‘Ì‚ð–¾‚é‚­‚·‚é)
    p = p * p + ambient;

    // “_ŒõŒ¹‚Ìˆ—
    float3 E = normalize(pin.world_position.xyz - camera_position.xyz);
    float3 point_diffuse = 0;
    float3 point_specular = 0;
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        float3 LP = pin.world_position.xyz - point_lights[i].position.xyz;
        float len = length(LP);
        if (len >= point_lights[i].range)
            continue;
        float attenuate_length = saturate(1.0f - len / point_lights[i].range);
        float attenuation = attenuate_length * attenuate_length;
        LP /= len;
        point_diffuse += calc_lambert(pin.world_normal.xyz, LP, point_lights[i].color.rgb) * attenuation;
        point_specular += calc_phong_specular(pin.world_normal.xyz, LP, E, point_lights[i].color.rgb) * attenuation;
    }

    color.rgb += point_diffuse;
    //color.rgb += point_specular;

    float4 Out = float4(color.rgb * p, color.a);

    return Out * pin.color * light_color;
}