struct VsInput
{
	float3 mWorldPosition : POSITION;
	float2 mTexCoord : TEXCOORD;
	float3 mNormal : NORMAL;
};

struct VsOutput
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
};
#define PsInput VsOutput