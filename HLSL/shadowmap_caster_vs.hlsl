#include "skinned_mesh.hlsli"
#include "constants.hlsli"

float4 main(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD) : SV_POSITION
{
    float4 pos = mul(position, mul(world, view_projection));
    return mul(pos, shake_matrix);
}