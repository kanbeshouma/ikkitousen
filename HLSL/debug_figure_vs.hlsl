#include "debug_figure.hlsli"
#include "constants.hlsli"

VS_OUT main(float4 position : POSITION)
{
    VS_OUT vout;
    vout.position = mul(position, mul(world, view_projection));
    vout.color = material_color;

    return vout;
}