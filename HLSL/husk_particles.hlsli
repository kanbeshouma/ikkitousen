struct VS_OUT
{
	uint vertex_id : VERTEXID;
};
struct GS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};
struct particle
{
	float4 color;
	float3 position;
	float3 normal;
	float3 velocity;
	float age;
	int state;
};