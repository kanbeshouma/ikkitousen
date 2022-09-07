Texture2D hdr_color_buffer_texture : register(t0);
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
SamplerState sampler_states[4] : register(s0);
float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
    return hdr_color_buffer_texture.Sample(sampler_states[LINEAR_BORDER_BLACK], texcoord, 0.0);
}