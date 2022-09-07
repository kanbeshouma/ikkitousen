#include "husk_particles.hlsli"

VS_OUT main(uint vertex_id : SV_VERTEXID)
{
    VS_OUT vout;
    vout.vertex_id = vertex_id;
    return vout;
}