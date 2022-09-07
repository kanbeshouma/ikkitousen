#include "skinned_mesh.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);

TextureCube skymap : register(t7);

[earlydepthstencil]
float4 main(VS_OUT pin) : SV_TARGET
{
    float4 diffuse_color = skymap.Sample(sampler_states[ANISOTROPIC], pin.world_position.xyz);

    const float GAMMA = 2.2;
    float3 diffuse_reflection = pow(diffuse_color.rgb, GAMMA);
    float alpha = diffuse_color.a;

    const float intensity = 0.8;

    return float4(diffuse_reflection.rgb * intensity, alpha);
}