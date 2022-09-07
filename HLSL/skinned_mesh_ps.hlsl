#include "skinned_mesh.hlsli"
#include "constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
texture2D texture_maps[4] : register(t0);

Texture2D shadow_map : register(t6);
SamplerState shadow_sampler_state : register(s6);


float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);

	// シャドウマップから深度値取得
    {
        float depth = shadow_map.Sample(shadow_sampler_state, pin.shadow_texcoord.xy).r;
		// 深度値を比較して影かどうかを判定する
        //if (pin.shadow_texcoord.z - depth > shadow_bias)
        {
            float light_percentage = 0;

            [unroll(50)]
            for (int Scope = 0; Scope < number_of_trials; ++Scope)
            {
                float search_width = search_width_magnification * Scope;
                float ratio = 1.0f / (number_of_trials * 4.0f);
                //----今のピクセルの周りが影じゃなかったら影の色を薄めていく----//
                // 左
                float depth_around_left = shadow_map.Sample(shadow_sampler_state, float2(pin.shadow_texcoord.x - search_width, pin.shadow_texcoord.y)).r;
                if (pin.shadow_texcoord.z - depth_around_left < shadow_bias)
                {
                    light_percentage += ratio;
                }
                // 右
                float depth_around_right = shadow_map.Sample(shadow_sampler_state, float2(pin.shadow_texcoord.x + search_width, pin.shadow_texcoord.y)).r;
                if (pin.shadow_texcoord.z - depth_around_right < shadow_bias)
                {
                    light_percentage += ratio;
                }
                // 上
                float depth_around_up = shadow_map.Sample(shadow_sampler_state, float2(pin.shadow_texcoord.x, pin.shadow_texcoord.y - search_width)).r;
                if (pin.shadow_texcoord.z - depth_around_up < shadow_bias)
                {
                    light_percentage += ratio;
                }
                // 下
                float depth_around_down = shadow_map.Sample(shadow_sampler_state, float2(pin.shadow_texcoord.x, pin.shadow_texcoord.y + search_width)).r;
                if (pin.shadow_texcoord.z - depth_around_down < shadow_bias)
                {
                    light_percentage += ratio;
                }
            }

            //color.rgb *= shadow_color.rgb * (1 + light_percentage);
            color.rgb *= light_percentage;
        }
    }

    float alpha = color.a;

#if 0
    // Inverse gamma process
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, GAMMA);
#endif

    float3 N = normalize(pin.world_normal.xyz);
    float3 T = normalize(pin.world_tangent.xyz);
    float sigma = pin.world_tangent.w;
    T = normalize(T - dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);

    float4 normal = texture_maps[1].Sample(sampler_states[LINEAR], pin.texcoord);
    normal = (normal * 2.0f) - 1.0f;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

    float3 L = normalize(-light_direction.xyz);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    float V = normalize(camera_position.xyz - pin.world_position.xyz);

    float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);

    return float4(diffuse + specular, alpha) * pin.color;
}