#include "constants.hlsli"

static const uint downsampled_count = 3;
Texture2D downsampled_textures[downsampled_count] : register(t0);
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
SamplerState sampler_states[4] : register(s0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
    float3 sampled_color = 0;
#if 0
    [unroll]
    for (uint downsampled_index = 0; downsampled_index < downsampled_count; ++downsampled_index)
    {
        sampled_color += downsampled_textures[downsampled_index].Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord).xyz;
    }
#else
    sampled_color += downsampled_textures[downsampled_count - 1].Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord).xyz;
#endif
    return float4(sampled_color * blur_convolution_intensity, 1);
}