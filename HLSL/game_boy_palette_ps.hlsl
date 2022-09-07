#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float gamma = 1.5;
    color.r = pow(color.r, gamma);
    color.g = pow(color.g, gamma);
    color.b = pow(color.b, gamma);

    float3 col1 = border_color1.rgb;
    float3 col2 = border_color2.rgb;
    float3 col3 = border_color3.rgb;
    float3 col4 = border_color4.rgb;

    //float3 col1 = float3(0.612, 0.725, 0.086);
    //float3 col2 = float3(0.549, 0.667, 0.078);
    //float3 col3 = float3(0.188, 0.392, 0.188);
    //float3 col4 = float3(0.063, 0.247, 0.063);

    float dist1 = length(color.rgb - col1);
    float dist2 = length(color.rgb - col2);
    float dist3 = length(color.rgb - col3);
    float dist4 = length(color.rgb - col4);

    float d = min(dist1, dist2);
    d = min(d, dist3);
    d = min(d, dist4);

    if (d == dist1)
    {
        color.rgb = col1;
    }
    else if (d == dist2)
    {
        color.rgb = col2;
    }
    else if (d == dist3)
    {
        color.rgb = col3;
    }
    else
    {
        color.rgb = col4;
    }

    return color;
}