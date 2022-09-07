// https://docs.microsoft.com/ja-jp/cpp/build/reference/hlsl-property-pages?view=msvc-160
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};