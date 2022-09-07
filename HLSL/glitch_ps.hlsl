#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

float white_noise(float2 coord)
{
    // フラクトサイン法
    return frac(sin(dot(coord, float2(8.7819, 3.255))) * 437.645);
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float vert_noise = white_noise(float2(floor((pin.texcoord.x) / glitch_pase.x) * glitch_pase.x, glitch_time * 0.1));
    float horz_noise = white_noise(float2(floor((pin.texcoord.y) / glitch_pase.y) * glitch_pase.y, glitch_time * 0.2));
    float vert_glitch_strength = vert_noise / glitch_step_value;
    float horz_glitch_strength = horz_noise / glitch_step_value;
    vert_glitch_strength = vert_glitch_strength * 2.0 - 1.0;
    horz_glitch_strength = horz_glitch_strength * 2.0 - 1.0;
    float V = step(vert_noise, glitch_step_value * 2) * vert_glitch_strength;
    float H = step(horz_noise, glitch_step_value) * horz_glitch_strength;

    float2 sample_point = pin.texcoord;
    float sinv = sin(sample_point.y * 2 - glitch_time * -0.1);
    float steped = 1 - step(0.99, sinv * sinv);
    float timeFrac = steped * step(0.8, frac(glitch_time));
    sample_point.x += timeFrac * (V + H);


    return texture_maps[0].Sample(sampler_states[ANISOTROPIC], sample_point);
}