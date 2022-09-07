#include "skinned_mesh.hlsli"
#include "constants.hlsli"
#include "shading_functions.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2

#define BASE_COLOR 0
#define NORMAL     1
#define METAL      2
#define ROUGHNESS  3
#define AO         4
#define Emissive   5

SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[6] : register(t0);

Texture2D shadow_map : register(t6);
SamplerState shadow_sampler_state : register(s6);

Texture2D dissolve_map : register(t8);
Texture2D glow_map : register(t9);

Texture2D sub_color_map_purple : register(t20);
Texture2D sub_color_map_red : register(t21);

static const float PI = 3.1415926f; // π

float3 cast_shadow(in float3 color, float depth, float3 shadow_texcoord)
{
	// 深度値を比較して影かどうかを判定する
    // if (pin.shadow_texcoord.z - depth > shadow_bias){}
    float light_percentage = 0;

    [unroll(50)]
    for (int Scope = 0; Scope < number_of_trials; ++Scope)
    {
        float search_width = search_width_magnification * Scope;
        float ratio = 1.0f / (number_of_trials * 4.0f);
         //----今のピクセルの周りが影じゃなかったら影の色を薄めていく----//
         // 左
        float depth_around_left = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x - search_width, shadow_texcoord.y)).r;
        if (shadow_texcoord.z - depth_around_left < shadow_bias)
        {
            light_percentage += ratio;
        }
        // 右
        float depth_around_right = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x + search_width, shadow_texcoord.y)).r;
        if (shadow_texcoord.z - depth_around_right < shadow_bias)
        {
            light_percentage += ratio;
        }
        // 上
        float depth_around_up = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x, shadow_texcoord.y - search_width)).r;
        if (shadow_texcoord.z - depth_around_up < shadow_bias)
        {
            light_percentage += ratio;
        }
        // 下
        float depth_around_down = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x, shadow_texcoord.y + search_width)).r;
        if (shadow_texcoord.z - depth_around_down < shadow_bias)
        {
            light_percentage += ratio;
        }
    }
    //color.rgb *= shadow_color.rgb * (1 + light_percentage);
    return color * light_percentage;
}

float3 get_normal(float3 normal, float3 tangent, float3 binormal, float2 uv)
{
    float3 bin_space_normal = texture_maps[NORMAL].SampleLevel(sampler_states[ANISOTROPIC], uv, 0.0f).xyz;
    bin_space_normal        = (bin_space_normal * 2.0f) - 1.0f;

    float3 new_normal       = tangent * bin_space_normal.x + binormal * bin_space_normal.y + normal * bin_space_normal.z;

    return new_normal;
}
// ベックマン分布を計算する
float beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}
// フレネルを計算。Schlick近似を使用
float spc_fresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1 - f0) * pow(1 - u, 5);
}
// Cook-Torranceモデルの鏡面反射を計算
// <param name="L">光源に向かうベクトル</param>
// <param name="V">視点に向かうベクトル</param>
// <param name="N">法線ベクトル</param>
// <param name="metallic">金属度</param>
float cook_torrance_specular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;
    // 金属度を垂直入射の時のフレネル反射率として扱う
    // 金属度が高いほどフレネル反射は大きくなる
    float f0 = metallic;
    // ライトに向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
    float3 H = normalize(L + V);
    // 各種ベクトルがどれくらい似ているかを内積を利用して求める
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    // D項をベックマン分布を用いて計算する
    float D = beckmann(microfacet, NdotH);
    // F項をSchlick近似を用いて計算する
    float F = spc_fresnel(f0, VdotH);
    // G項を求める
    float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));
    // m項を求める
    float m = PI * NdotV * NdotH;
    // ここまで求めた、値を利用して、Cook-Torranceモデルの鏡面反射を求める
    return max(F * D * G / m, 0.0);
}
// フレネル反射を考慮した拡散反射を計算
// </summary>
// <remark>
// この関数はフレネル反射を考慮した拡散反射率を計算します
// フレネル反射は、光が物体の表面で反射する現象のとこで、鏡面反射の強さになります
// 一方拡散反射は、光が物体の内部に入って、内部錯乱を起こして、拡散して反射してきた光のことです
// つまりフレネル反射が弱いときには、拡散反射が大きくなり、フレネル反射が強いときは、拡散反射が小さくなります

// </remark>
// <param name="N">法線</param>
// <param name="L">光源に向かうベクトル。光の方向と逆向きのベクトル。</param>
// <param name="V">視線に向かうベクトル。</param>
float calc_diffuse_from_fresnel(float3 N, float3 L, float3 V)
{
    // step-4 フレネル反射を考慮した拡散反射光を求める
    // 法線と光源に向かうベクトルがどれだけ似ているかを内積で求める
    float dotNL = saturate(dot(N, L));

    // 法線と視線に向かうベクトルがどれだけ似ているかを内積で求める
    float dotNV = saturate(dot(N, V));

    // 法線と光源への方向に依存する拡散反射率と、法線と視点ベクトルに依存する拡散反射率を
    // 乗算して最終的な拡散反射率を求めている。PIで除算しているのは正規化を行うため
    return (dotNL * dotNV);
}

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color_map = texture_maps[BASE_COLOR].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float4 normal_map = texture_maps[NORMAL].Sample(sampler_states[LINEAR], pin.texcoord);
    float4 metal_map = texture_maps[METAL].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float4 roughness_map = texture_maps[ROUGHNESS].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float4 ao_map = texture_maps[AO].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float4 emissive_map = texture_maps[Emissive].Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float4 sub_color_purple = sub_color_map_purple.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
    float4 sub_color_red    = sub_color_map_red.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    // sub color による線形補完
    float3 lerp_color_map = color_map;

    if (sub_color_purple.r > 0.9f && sub_color_purple.g > 0.9f && sub_color_purple.b > 0.9f
     && sub_color_red.r > 0.9f    && sub_color_red.g > 0.9f    && sub_color_red.b > 0.9f)
    {
    }
    else
    {
        if (sub_color_threshold_purple > 0.01f)
        {
            lerp_color_map = lerp(color_map, sub_color_purple, sub_color_threshold_purple);
        }
        if (sub_color_threshold_red > 0.01f)
        {
            lerp_color_map = lerp(sub_color_purple, sub_color_red, sub_color_threshold_red);
        }
    }

    color_map.rgb = lerp_color_map;

    // shadow map
    //float depth = shadow_map.Sample(shadow_sampler_state, pin.shadow_texcoord.xy).r;
    //color_map.rgb = cast_shadow(color_map.rgb, depth, pin.shadow_texcoord);

    // 法線を計算
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = normalize(pin.world_tangent.xyz);
    float sigma = pin.world_tangent.w;
    T = normalize(T - dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    float3 normal = get_normal(N, T, B, pin.texcoord);
    // 金属度
    float metallic = metal_map.r;
    // 滑らかさ
    float smooth = roughness_map.r;
    // 視線に向かって伸びるベクトルを計算する
    float3 toEye = normalize(camera_position - pin.world_position);
    // シンプルなディズニーベースの拡散反射を実装する
    float diffuseFromFresnel = calc_diffuse_from_fresnel(normal, -light_direction.xyz * 2.0f, toEye);
    float NdotL = saturate(dot(normal, -light_direction.xyz * 2.0f));
    float3 lambertDiffuse = float4(1, 1, 1, 1) * NdotL / PI;
    float3 diffuse = color_map.rgb * diffuseFromFresnel * lambertDiffuse;
    // Cook-Torranceモデルを利用した鏡面反射率を計算する
    float3 spec = cook_torrance_specular(-light_direction.xyz * 2.0f, toEye, normal, smooth) * float4(1, 1, 1, 1);
    spec *= lerp(float3(1, 1, 1), color_map.rgb, metallic);
    // 滑らかさを使って、拡散反射光と鏡面反射光を合成する
    float3 lig = diffuse * (1.0f - smooth) + spec * 2;

#if 0
    // Inverse gamma process
    const float GAMMA = 2.2;
    color_map.rgb = pow(color_map.rgb, GAMMA);
#endif

    // 点光源の処理
    float3 E = normalize(pin.world_position.xyz - camera_position.xyz);
    float3 point_diffuse = 0;
    float3 point_specular = 0;
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        float3 LP = pin.world_position.xyz - point_lights[i].position.xyz;
        float len = length(LP);
        if (len >= point_lights[i].range)
            continue;
        float attenuate_length = saturate(1.0f - len / point_lights[i].range);
        float attenuation = attenuate_length * attenuate_length;
        LP /= len;
        point_diffuse += calc_lambert(pin.world_normal.xyz, LP, point_lights[i].color.rgb) * attenuation;
        point_specular += calc_phong_specular(pin.world_normal.xyz, LP, E, point_lights[i].color.rgb) * attenuation;
    }

    lig += point_diffuse * 2.0f;
    //lig += point_specular;

    // 環境光による底上げ
    lig += float3(0.4, 0.4, 0.4) * color_map.rgb;

    float4 finalColor = 1.0f;
    finalColor.xyz = lig;

    // emissive
    float3 emissive = emissive_map.r * emissive_color.w * emissive_color.rgb;
    finalColor.xyz += emissive;

    // glow
    float3 glow = glow_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord).rgb;
    float glow_interval  = -1.0f;
    float sin_v = sin(pin.texcoord.y * glow_thickness + dissolve_threshold.y * glow_interval);
    float steped = smoothstep(1, sin_v * sin_v, 0.99);
    finalColor.rgb += glow * steped * emissive_color.w;

    // ブルームで暴走しないように強制
    finalColor.xyz = min(finalColor.xyz, 6.0);

    // dissolve
    float4 last_color = float4(finalColor.rgb * ao_map.r * light_direction.w * pin.color.rgb, finalColor.a * pin.color.a * color_map.a);

    float4 dst_color  = float4(0, 0, 0, 0);
    float4 mask       = dissolve_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float4 outcolor = lerp(last_color, dst_color, smoothstep(0, mask.r, dissolve_threshold.x));
    if (dissolve_threshold.x < 0.95)
    {
        float4 destiny = float4(emissive_color.rgb * emissive_color.w, 1);
        outcolor += lerp(destiny, dst_color, smoothstep(0, smoothstep(0, mask.r, dissolve_threshold.x), smoothstep(0, 0.1, abs(dissolve_threshold.x - mask.r))));
    }

    // マテリアルのアルファ値が0.1未満ならそのピクセルを破棄する
    clip(outcolor.a < 0.1f ? -1 : 1);

    return outcolor;
}