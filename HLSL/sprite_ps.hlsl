//==============================================================================================================================
//   https://yttm-work.jp/directx/directx_0014.html
//   https://araramistudio.jimdo.com/2017/10/18/%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0-directx-11%E3%81%A7%E3%83%86%E3%82%AF%E3%82%B9%E3%83%81%E3%83%A3%E3%83%9E%E3%83%83%E3%83%94%E3%83%B3%E3%82%B0/
//         �s�N�Z���V�F�[�_
// �� �v���p�e�B�Őݒ肷�邱�ƂŃr���h����VS���R���p�C�����Ă����
//    .cso �̓R���p�C�����ꂽ�V�F�[�_�t�@�C��
//
// sprite�N���X��render�����o�֐��̐F�w��̈������@�\����悤�ɁA�s�N�Z���V�F�[�_�[�ɕύX��������
// �F���̕ύX��A �A���t�@�l��ω������t�F�[�h�A�E�g���̌��ʂ�\���ł���悤�ɂ���
//
//==============================================================================================================================
#include "sprite.hlsli"
#include "constants.hlsli"

Texture2D color_map : register(t0);
SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = color_map.Sample(anisotropic_sampler_state, pin.texcoord);
    float alpha = color.a;

    // ���ɐ��𑖂点��
    float3 glow_horizon_color = { 0.2, 0.2, 0.2 };
    float glow_horizon_interval = 0.5f;
    float glow_horizon_thickness = 1.5; // ������قǍׂ�
    float horizon_sin_v = sin(pin.texcoord.x * glow_horizon_thickness + (threshold.x - 0.5f) * glow_horizon_interval);
    float horizon_steped = smoothstep(1, horizon_sin_v * horizon_sin_v, 0.99);
    color.rgb += glow_horizon_color * horizon_steped;

    // �c�ɐ��𑖂点��
    if (threshold.y < 0)
    {
        // ������
        float3 glow_vertical_color = { 0.2, 0.2, 0.2 };
        float glow_vertical_interval = 5.0f;
        float glow_vertical_thickness = 3.0;
        float vertical_sin_v = sin(pin.texcoord.y * glow_vertical_thickness + (threshold.y - 0.5f) * glow_vertical_interval);
        float vertical_steped = smoothstep(1, vertical_sin_v * vertical_sin_v, 0.99);
        color.rgb -= glow_vertical_color * vertical_steped;

        // �ׂ���
        glow_vertical_thickness = 4.5;
        vertical_sin_v = sin(pin.texcoord.y * glow_vertical_thickness + (threshold.y - 2.0f) * glow_vertical_interval);
        vertical_steped = smoothstep(1, vertical_sin_v * vertical_sin_v, 0.99);
        color.rgb -= glow_vertical_color * vertical_steped;
    }


#if 0
    // Inverse gamma process
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, GAMMA);
#endif
    return float4(color.rgb, alpha) * pin.color;
}