#include "skinned_mesh.hlsli"
#include "constants.hlsli"
#include "shading_functions.hlsli"

cbuffer Data :register(b10)
{
	float4 mBeamColor;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    return mBeamColor;
}