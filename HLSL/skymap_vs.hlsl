#include "skinned_mesh.hlsli"
#include "constants.hlsli"

struct VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
};

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
	//vout.position = mul(vin.position, mul(world, view_projection)).xyww;
    vout.position = mul(vin.position, mul(world, view_projection));

    vout.world_position = mul(vin.position, world);

    vin.normal.w = 0;
    vout.world_normal = normalize(mul(vin.normal, world));

    float sigma = vin.tangent.w;
    vin.tangent.w = 0;
    vout.world_tangent = normalize(mul(vin.tangent, world));
    vout.world_tangent.w = sigma;

    vout.texcoord = vin.texcoord;
    vout.color = 1.0;

    return vout;
}
