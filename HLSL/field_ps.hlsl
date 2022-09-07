#include "skinned_mesh.hlsli"
#include "constants.hlsli"

#define S(r,v,a) step(a/field_resolution.y,abs(v-(r)))

static const float2 s = float2(1, 1.7320508); // 1.7320508 = sqrt(3)

float calcHexDistance(float2 p)
{
    p = abs(p);
    return max(dot(p, s * 0.5), p.x);
}

float2 calcHexOffset(float2 uv)
{
    float4 hexCenter = round(float4(uv, uv - float2(0.5, 1.0)) / s.xyxy);
    float4 offset = float4(uv - hexCenter.xy * s, uv - (hexCenter.zw + 0.5) * s);
    return dot(offset.xy, offset.xy) < dot(offset.zw, offset.zw) ? offset.xy : offset.zw;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float2 uv = (2.0 * pin.texcoord - field_resolution.xy) / field_resolution.y;
    float2 hexInfo = calcHexOffset(uv * 200.0);
    float speed = 0.1;
    float a = cos(speed * (1.5 * length(uv) - field_time));
    float h = calcHexDistance(hexInfo);

    float4 fragColor = float4(0, 0, 0, 1);

    fragColor.b = 1 - (S(abs(sin(h * a * 5.0)), 1.0, 1.0) + 0.3 * S(h, 0.45, 20.0) + 0.15
        + 0.3 * smoothstep(0.5, 0.25 + 12.0 / field_resolution.y, h));

    if (fragColor.b < 0.2f)
    {
        clip(-1);
    }

    return fragColor;
}