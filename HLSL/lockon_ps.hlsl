#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float2 sample_point = pin.texcoord;
    float4 tex          = texture_maps[0].Sample(sampler_states[ANISOTROPIC], sample_point);

    float vignette_x = length(0.5 - pin.texcoord.x);
    float vignette_y = length(0.5 - pin.texcoord.y);
    float fill_x = 0;
    fill_x = step(lockon_scope, vignette_x);
    fill_x -= step(lockon_scope + lockon_thickness, vignette_x);
    float fill_y = 0;
    fill_y = step(lockon_scope, vignette_y);
    fill_y -= step(lockon_scope + lockon_thickness, vignette_y);

    float fill = fill_x + fill_y;

    tex.r += lockon_color.r * fill * lockon_alpha;
    tex.g += lockon_color.g * fill * lockon_alpha;
    tex.b += lockon_color.b * fill * lockon_alpha;
    //tex.rgb += float3(lockon_color.x * fill, lockon_color.y * fill, lockon_color.z * fill) * lockon_alpha;
    return tex;
}