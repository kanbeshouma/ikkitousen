#include "husk_particles.hlsli"
#include "constants.hlsli"

StructuredBuffer<particle> particle_buffer : register(t9);

[maxvertexcount(4)]
void main(point VS_OUT input[1] : SV_POSITION, inout TriangleStream<GS_OUT> output)
{
	const float3 billboard[] =
	{
		float3(-1.0f, -1.0f, 0.0f), // Bottom-left corner
		float3(-1.0f, +1.0f, 0.0f),	// Top-left corner
		float3(+1.0f, -1.0f, 0.0f),	// Bottom-right corner
		float3(+1.0f, +1.0f, 0.0f),	// Top-right corner
	};
	const float2 texcoords[] =
	{
		float2(0.0f, 1.0f),			// Bottom-left
		float2(0.0f, 0.0f),			// Top-left
		float2(1.0f, 1.0f),			// Bottom-right
		float2(1.0f, 0.0f),			// Top-right
	};

	particle p = particle_buffer[input[0].vertex_id];

	float3 Z = normalize(p.normal);
	float3 X = normalize(cross(Z, float3(0, 1, 0)));
	float3 Y = normalize(cross(Z, X));
	row_major float3x3 R = { X, Y, Z };

	[unroll]
	for (uint vertex_index = 0; vertex_index < 4; ++vertex_index)
	{
		GS_OUT element;
		float3 corner_pos = billboard[vertex_index] * particle_size;

		element.position = mul(float4(p.position + mul(corner_pos, R), 1), view_projection);
		element.color = p.color;
		element.texcoord = texcoords[vertex_index];
		output.Append(element);
	}

	output.RestartStrip();
}