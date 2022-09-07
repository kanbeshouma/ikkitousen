#include "skinned_mesh.hlsli"
#include "constants.hlsli"
#include "shading_functions.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
texture2D texture_maps[4] : register(t0);
SamplerState sampler_states[3] : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float3 fragment_color = color.rgb;
    float alpha = color.a;

    //fragment_color *= point_lights[point_light_unique_id].luminous_intensity;
    fragment_color *= pow(point_lights[point_light_unique_id].luminous_intensity, 4);
    //fragment_color += point_lights[point_light_unique_id].luminous_intensity;

    return float4(fragment_color, alpha) * pin.color;
}