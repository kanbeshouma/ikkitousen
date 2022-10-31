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

static const float PI = 3.1415926f; // ��

float3 cast_shadow(in float3 color, float depth, float3 shadow_texcoord)
{
	// �[�x�l���r���ĉe���ǂ����𔻒肷��
    // if (pin.shadow_texcoord.z - depth > shadow_bias){}
    float light_percentage = 0;

    [unroll(50)]
    for (int Scope = 0; Scope < number_of_trials; ++Scope)
    {
        float search_width = search_width_magnification * Scope;
        float ratio = 1.0f / (number_of_trials * 4.0f);
         //----���̃s�N�Z���̎��肪�e����Ȃ�������e�̐F�𔖂߂Ă���----//
         // ��
        float depth_around_left = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x - search_width, shadow_texcoord.y)).r;
        if (shadow_texcoord.z - depth_around_left < shadow_bias)
        {
            light_percentage += ratio;
        }
        // �E
        float depth_around_right = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x + search_width, shadow_texcoord.y)).r;
        if (shadow_texcoord.z - depth_around_right < shadow_bias)
        {
            light_percentage += ratio;
        }
        // ��
        float depth_around_up = shadow_map.Sample(shadow_sampler_state, float2(shadow_texcoord.x, shadow_texcoord.y - search_width)).r;
        if (shadow_texcoord.z - depth_around_up < shadow_bias)
        {
            light_percentage += ratio;
        }
        // ��
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
// �x�b�N�}�����z���v�Z����
float beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}
// �t���l�����v�Z�BSchlick�ߎ����g�p
float spc_fresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1 - f0) * pow(1 - u, 5);
}
// Cook-Torrance���f���̋��ʔ��˂��v�Z
// <param name="L">�����Ɍ������x�N�g��</param>
// <param name="V">���_�Ɍ������x�N�g��</param>
// <param name="N">�@���x�N�g��</param>
// <param name="metallic">�����x</param>
float cook_torrance_specular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;
    // �����x�𐂒����˂̎��̃t���l�����˗��Ƃ��Ĉ���
    // �����x�������قǃt���l�����˂͑傫���Ȃ�
    float f0 = metallic;
    // ���C�g�Ɍ������x�N�g���Ǝ����Ɍ������x�N�g���̃n�[�t�x�N�g�������߂�
    float3 H = normalize(L + V);
    // �e��x�N�g�����ǂꂭ�炢���Ă��邩����ς𗘗p���ċ��߂�
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    // D�����x�b�N�}�����z��p���Čv�Z����
    float D = beckmann(microfacet, NdotH);
    // F����Schlick�ߎ���p���Čv�Z����
    float F = spc_fresnel(f0, VdotH);
    // G�������߂�
    float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));
    // m�������߂�
    float m = PI * NdotV * NdotH;
    // �����܂ŋ��߂��A�l�𗘗p���āACook-Torrance���f���̋��ʔ��˂����߂�
    return max(F * D * G / m, 0.0);
}
// �t���l�����˂��l�������g�U���˂��v�Z
// </summary>
// <remark>
// ���̊֐��̓t���l�����˂��l�������g�U���˗����v�Z���܂�
// �t���l�����˂́A�������̂̕\�ʂŔ��˂��錻�ۂ̂Ƃ��ŁA���ʔ��˂̋����ɂȂ�܂�
// ����g�U���˂́A�������̂̓����ɓ����āA�����������N�����āA�g�U���Ĕ��˂��Ă������̂��Ƃł�
// �܂�t���l�����˂��ア�Ƃ��ɂ́A�g�U���˂��傫���Ȃ�A�t���l�����˂������Ƃ��́A�g�U���˂��������Ȃ�܂�

// </remark>
// <param name="N">�@��</param>
// <param name="L">�����Ɍ������x�N�g���B���̕����Ƌt�����̃x�N�g���B</param>
// <param name="V">�����Ɍ������x�N�g���B</param>
float calc_diffuse_from_fresnel(float3 N, float3 L, float3 V)
{
    // step-4 �t���l�����˂��l�������g�U���ˌ������߂�
    // �@���ƌ����Ɍ������x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
    float dotNL = saturate(dot(N, L));

    // �@���Ǝ����Ɍ������x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
    float dotNV = saturate(dot(N, V));

    // �@���ƌ����ւ̕����Ɉˑ�����g�U���˗��ƁA�@���Ǝ��_�x�N�g���Ɉˑ�����g�U���˗���
    // ��Z���čŏI�I�Ȋg�U���˗������߂Ă���BPI�ŏ��Z���Ă���̂͐��K�����s������
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

    if (color_map.r > 0.7f && color_map.g > 0.7f && color_map.b > 0.7f)
    {
        color_map.rgb= pin.color.rgb;
    }

    // sub color �ɂ����`�⊮
        float3 lerp_color_map = color_map.xyz;

    if (sub_color_purple.r > 0.9f && sub_color_purple.g > 0.9f && sub_color_purple.b > 0.9f
     && sub_color_red.r > 0.9f    && sub_color_red.g > 0.9f    && sub_color_red.b > 0.9f)
    {
    }
    else
    {
        if (sub_color_threshold_purple > 0.01f)
        {
            lerp_color_map = lerp(color_map, sub_color_purple, sub_color_threshold_purple).xyz;
        }
        if (sub_color_threshold_red > 0.01f)
        {
            lerp_color_map = lerp(sub_color_purple, sub_color_red, sub_color_threshold_red).xyz;
        }
    }

    color_map.rgb = lerp_color_map;

    // shadow map
    //float depth = shadow_map.Sample(shadow_sampler_state, pin.shadow_texcoord.xy).r;
    //color_map.rgb = cast_shadow(color_map.rgb, depth, pin.shadow_texcoord);

    // �@�����v�Z
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = normalize(pin.world_tangent.xyz);
    float sigma = pin.world_tangent.w;
    T = normalize(T - dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    float3 normal = get_normal(N, T, B, pin.texcoord);
    // �����x
    float metallic = metal_map.r;
    // ���炩��
    float smooth = roughness_map.r;
    // �����Ɍ������ĐL�т�x�N�g�����v�Z����
    float3 toEye = normalize(camera_position - pin.world_position).xyz;
    // �V���v���ȃf�B�Y�j�[�x�[�X�̊g�U���˂���������
    float diffuseFromFresnel = calc_diffuse_from_fresnel(normal, -light_direction.xyz * 2.0f, toEye);
    float NdotL = saturate(dot(normal, -light_direction.xyz * 2.0f));
    float3 lambertDiffuse = float3(1, 1, 1) * NdotL / PI;
    float3 diffuse = color_map.rgb * diffuseFromFresnel * lambertDiffuse;
    // Cook-Torrance���f���𗘗p�������ʔ��˗����v�Z����
    float3 spec = cook_torrance_specular(-light_direction.xyz * 2.0f, toEye, normal, smooth) * float3(1, 1, 1);
    spec *= lerp(float3(1, 1, 1), color_map.rgb, metallic);
    // ���炩�����g���āA�g�U���ˌ��Ƌ��ʔ��ˌ�����������
    float3 lig = diffuse * (1.0f - smooth) + spec * 2;

#if 0
    // Inverse gamma process
    const float GAMMA = 2.2;
    color_map.rgb = pow(color_map.rgb, GAMMA);
#endif

    // �_�����̏���
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

    // �����ɂ���グ
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

    // �u���[���Ŗ\�����Ȃ��悤�ɋ���
    finalColor.xyz = min(finalColor.xyz, 6.0);

    // dissolve
    float4 last_color = float4(finalColor.rgb * ao_map.r * light_direction.w, finalColor.a * pin.color.a * color_map.a);

    float4 dst_color  = float4(0, 0, 0, 0);
    float4 mask       = dissolve_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float4 outcolor = lerp(last_color, dst_color, smoothstep(0, mask.r, dissolve_threshold.x));
    if (dissolve_threshold.x < 0.95)
    {
        float4 destiny = float4(emissive_color.rgb * emissive_color.w, 1);
        outcolor += lerp(destiny, dst_color, smoothstep(0, smoothstep(0, mask.r, dissolve_threshold.x), smoothstep(0, 0.1, abs(dissolve_threshold.x - mask.r))));
    }

    // �}�e���A���̃A���t�@�l��0.1�����Ȃ炻�̃s�N�Z����j������
    clip(outcolor.a < 0.1f ? -1 : 1);
    return outcolor;
}