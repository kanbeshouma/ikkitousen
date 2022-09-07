#include "husk_particles.hlsli"
#include "constants.hlsli"

RWStructuredBuffer<particle> particle_buffer : register(u0);
RWStructuredBuffer<particle> updated_particle_buffer : register(u1);

[numthreads(1024, 1, 1)]
void main(uint3 dtid : SV_DISPATCHTHREADID)
{
    uint id = dtid.x;
    if (id <= particle_count)
    {
        particle p = updated_particle_buffer[id];

        if (p.state == 0)
        {
            p = particle_buffer[id];
            p.state = 1;
        }
        else if (p.state == 1)
        {
            float3 v = normalize(position_on_near_plane.xyz - eye_position.xyz);
            float3 z = p.position.xyz - eye_position.xyz;
            float3 x = dot(v, z) * v - z;
            z = normalize(z);
            if (length(x) < 0.2)
            {
                p.color *= 2.0;
                p.state = 2;
            }
        }
        else if (p.state == 2)
        {
            float3 v = normalize(position_on_near_plane.xyz - eye_position.xyz);
            float3 z = p.position.xyz - eye_position.xyz;
            float3 x = dot(v, z) * v - z;
            z = normalize(z);
            float3 y = normalize(cross(z, v));
            p.velocity = z * 2.0 + y * 5.0 - x * 0.5;
            p.position += p.velocity * delta_time;
            if (p.age > 5.0)
            {
                p.state = 3;
            }
        }
        else if (p.state == 3)
        {
            float3 d = particle_buffer[id].position - p.position;
            p.velocity = normalize(d) * lerp(0, 5.0, length(d));
            p.position += p.velocity * delta_time;
            if (length(particle_buffer[id].position - p.position) < 0.01)
            {
                p.state = 0;
            }
        }
        if (p.state > 0)
        {
            p.age += delta_time;
        }
        updated_particle_buffer[id] = p;
    }
}