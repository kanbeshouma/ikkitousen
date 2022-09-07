#include "sprite_dissolve.hlsli"
#include "constants.hlsli"

Texture2D color_map : register(t0);
Texture2D mask_map : register(t1);
SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 src_color = color_map.Sample(anisotropic_sampler_state, pin.texcoord) * pin.color;
    float4 dst_color = color_map.Sample(anisotropic_sampler_state, pin.texcoord) * float4(0, 0, 0, 0);
    float4 mask      = mask_map.Sample(anisotropic_sampler_state, pin.texcoord)  * float4(1, 1, 1, 1);

    return lerp(src_color, dst_color, step(mask.r, threshold.x));
}