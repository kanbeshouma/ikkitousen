#include "skinned_mesh.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

struct PARTICLE
{
	float4 color;
	float3 position;
	float3 normal;
	float3 velocity;
	float age;
	int state;
};

AppendStructuredBuffer<PARTICLE> particle_buffer : register(u1);

void main(VS_OUT pin)
{
	float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
	float alpha = color.a;

#if 0
	// UNIT.32
	// Inverse gamma process
	const float GAMMA = 2.2;
	color.rgb = pow(color.rgb, GAMMA);
#endif

	float3 N = normalize(pin.world_normal.xyz);
#if 1
	// UNIT.29
	float3 T = normalize(pin.world_tangent.xyz);
	float sigma = pin.world_tangent.w;
	T = normalize(T - dot(N, T));
	float3 B = normalize(cross(N, T) * sigma);

	float4 normal = texture_maps[1].Sample(sampler_states[LINEAR], pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
#endif
	float3 L = normalize(-light_direction.xyz);
	float3 diffuse = color.rgb * max(0, dot(N, L));
	float3 V = normalize(camera_position.xyz - pin.world_position.xyz);
	float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);
	float3 ambient = color.rgb * 0.2;

	PARTICLE p;
	p.color = float4(max(0, ambient + diffuse + specular), alpha) * pin.color;
	p.position = pin.world_position.xyz;
	p.normal = N.xyz;
	p.velocity = 0;
	p.age = 0;
	p.state = 0;
	particle_buffer.Append(p);
}