#include "fullscreen_quad.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D tunnel_map : register(t0);

#define PI 3.141592653589793
#define TAU 6.283185307179586

float4 main(VS_OUT pin) : SV_TARGET
{
    float time = threshold.z * 0.1;

    float2 p = (-(threshold.xy * 2.0f) + 2.0 * pin.texcoord) / threshold.y; // normalized coordinates (-1 to 1 vertically)
    float2 uv_orig = p;
    // added twist by me ------------
    float rot_z = 1.0 - 0.07 * sin(1.0 * sin(length(p * 1.5)));
    p = mul(p, float2x2(cos(rot_z), -sin(rot_z), sin(rot_z), cos(rot_z)));
	//-------------------------------
    float a = atan2(p.y, p.x); // angle of each pixel to the center of the screen
    float r_round = length(p);
    //float r_square = pow(pow(p.x * p.x, 4.0) + pow(p.y * p.y, 4.0), 1.0 / 8.0); // modified distance metric (http://en.wikipedia.org/wiki/Minkowski_distance)
    //float r = lerp(r_square, r_round, 0.5 + 0.5 * sin(time * 2.)); // interp between round & rect tunnels
    float r = r_round; // interp between round & rect tunnels
    float speed = 0.3;
    float2 uv = float2(speed / r + time, a / 3.1415927); // index texture by (animated inverse) radious and angle
    //////////////////////////////////////////////////////

    // subdivide to grid
    float wave = 2.0;
    uv += float2(0.0, 0.25 * sin(time + uv.x * wave)); // pre-warp
    float count = 1.0;
    float2 uv_draw = frac(uv * (count / 2.0f) + 4.0 * time) - 0.5; // create grid

    float rot = PI / 2.;
    uv_draw = mul(uv_draw, float2x2(cos(rot), -sin(rot), sin(rot), cos(rot)));

    float4 tex = tunnel_map.Sample(sampler_states[LINEAR], uv_draw);

    float3 last_color = tex.rgb;
    // darker towards center
    float leng = length(pin.texcoord - float2(threshold.xy));
    float brightness = 4.0;
    last_color *= min(1.0, leng * brightness);

    return float4(last_color, threshold.w);
}