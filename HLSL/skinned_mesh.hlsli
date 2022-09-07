struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float4 world_normal : NORMAL;
    float4 world_tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
    float3 shadow_texcoord : TEXCOORD1;   // シャドウマップ用のパラメーター変数
};