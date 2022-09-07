#include "skinned_mesh.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
texture2D texture_maps[4] : register(t0);
texture2D toon_map : register(t7);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord) * pin.color;
    // ハーフランバート拡散照明によるライティング計算
    float p = dot(pin.world_normal.xyz * -1.0f, light_direction.xyz);
    p = p * 0.5f + 0.5f;
    p = p * p;
    //計算結果よりトゥーンシェーダー用のテクスチャから色をフェッチする
    float4 col = toon_map.Sample(sampler_states[ANISOTROPIC], float2(p, 0.0f));

    return color *= col;
}