#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 tex   = texture_maps[0].Sample(sampler_states[POINT], pin.texcoord);
    float alpha  = tex.a;
    float time = scan_line_time;
    float sin_v  = sin(pin.texcoord.y * 2 + time * -0.1);
    float steped = step(0.99, sin_v * sin_v);
    // ×‚¢‰¡ü
    tex.rgb -= (1 - steped) * abs(sin(pin.texcoord.y * 50.0f + time * 1.0f)) * 0.05;
    //@×‚¢‰¡ü
    tex.rgb -= (1 - steped) * abs(sin(pin.texcoord.y * 100.0f - time * 2.0f)) * 0.08;
    // ‘å‚«‚¢‰¡ü‚ÌF
    tex.rgb += steped * 0.1;

    return float4(tex.rgb, alpha);
}