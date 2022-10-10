#include "OutLine.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2

SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[6] : register(t0);

Texture2D dissolve_map : register(t8);

float4 main(VS_OUT pin) : SV_TARGET
{

	    // dissolve
    float4 last_color = float4(light_direction);

    float4 dst_color = float4(0, 0, 0, 0);
    float4 mask = dissolve_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float4 outcolor = lerp(last_color, dst_color, smoothstep(0, mask.r, dissolve_threshold.x));
    if (dissolve_threshold.x < 0.95)
    {
        float4 destiny = float4(emissive_color.rgb * emissive_color.w, 1);
        outcolor += lerp(destiny, dst_color, smoothstep(0, smoothstep(0, mask.r, dissolve_threshold.x), smoothstep(0, 0.1, abs(dissolve_threshold.x - mask.r))));
    }

    // マテリアルのアルファ値が0.1未満ならそのピクセルを破棄する
    clip(outcolor.a < 0.1f ? -1 : 1);

    return float4(1.0,1.0,1.0,outcolor.a);
}