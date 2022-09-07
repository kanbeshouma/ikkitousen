#include "constants.hlsli"

Texture2D hdr_color_buffer_texture : register(t0);
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
SamplerState sampler_states[4] : register(s0);
float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
    float4 sampled_color = hdr_color_buffer_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord);

    // ê‘êFÇÕÉuÉãÅ[ÉÄèEÇ¢Ç∏ÇÁÇ¢ÇÃÇ≈ï‚ê≥Ç∑ÇÈ
    float3 corrected_color = float3(sampled_color.r * 3.5f, sampled_color.gb);

    float4 out_color = float4(smoothstep(bloom_extraction_threshold, bloom_extraction_threshold + 0.5,
    dot(corrected_color, float3(0.299, 0.587, 0.114))) * sampled_color.rgb, sampled_color.a);


    return out_color;
}