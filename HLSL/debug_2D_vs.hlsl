#include "debug_2D.hlsli"
VS_OUT main(float4 position : POSITION, float4 color : COLOR)
{
    VS_OUT vout;
    vout.position = position;
    vout.color = color;
    return vout;
}