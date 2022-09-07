#include "husk_particles.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_map : register(t0);

float4 main(GS_OUT pin) : SV_TARGET
{
	return pin.color;
}