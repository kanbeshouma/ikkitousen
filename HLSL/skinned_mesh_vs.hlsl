#include "skinned_mesh.hlsli"
#include "constants.hlsli"

struct VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 bone_weights : WEIGHTS;
    uint4 bone_indices : BONES;
};

VS_OUT main(VS_IN vin)
{
    vin.normal.w = 0;
    float sigma = vin.tangent.w;
    vin.tangent.w = 0;

    float4 blended_position = { 0, 0, 0, 1 };
    float4 blended_normal = { 0, 0, 0, 0 };
    float4 blended_tangent = { 0, 0, 0, 0 };
    for (int bone_index = 0; bone_index < 4; ++bone_index)
    {
        blended_position += vin.bone_weights[bone_index]
         * mul(vin.position, bone_transforms[vin.bone_indices[bone_index]]);
        blended_normal += vin.bone_weights[bone_index]
         * mul(vin.normal, bone_transforms[vin.bone_indices[bone_index]]);
        blended_tangent += vin.bone_weights[bone_index]
         * mul(vin.tangent, bone_transforms[vin.bone_indices[bone_index]]);
    }
    vin.position = float4(blended_position.xyz, 1.0f);
    vin.normal = float4(blended_normal.xyz, 0.0f);
    vin.tangent = float4(blended_tangent.xyz, 0.0f);

    VS_OUT vout;
    vout.position = mul(vin.position, mul(world, view_projection));
    // カメラシェイク
    vout.position = mul(vout.position, shake_matrix);

    vout.world_position = mul(vin.position, world);
    vin.normal.w = 0;
    vout.world_normal = normalize(mul(vin.normal, world));
    vout.world_tangent = normalize(mul(vin.tangent, world));
    vout.world_tangent.w = sigma;

    vout.texcoord = vin.texcoord;
    // シャドウマップ
	// ライトから見たNDC座標を算出
    float4 wvpPos = mul(vin.position, mul(world, light_view_projection));
	// NDC座標からUV座標を算出する
    wvpPos /= wvpPos.w;
    wvpPos.y = -wvpPos.y;
    wvpPos.xy = 0.5f * wvpPos.xy + 0.5f;
    vout.shadow_texcoord = wvpPos.xyz;

    vout.color = material_color;

    return vout;
}