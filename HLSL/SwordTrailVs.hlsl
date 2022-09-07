#include"SwordTrail.hlsli"
#include "Constants.hlsli"

VsOutput main( VsInput VsInput_ )
{
	VsOutput vs_output;
	vs_output.Texcoord = VsInput_.mTexCoord;

	float4 worldPos=vs_output.Position = 
	mul(float4(VsInput_.mWorldPosition,1.0f), view_projection);
	float3 Z = normalize(VsInput_.mNormal);
	float3 X = normalize(cross(Z, float3(0, 1, 0)));
	float3 Y = normalize(cross(Z, X));
	row_major float3x3 R = { X, Y, Z };

	vs_output.Position = worldPos;
	//mul(float4(worldPos + mul(worldPos, R), 1), view_projection);


	return vs_output;
}