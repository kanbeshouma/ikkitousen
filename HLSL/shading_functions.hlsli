//--------------------------------------------
//	�����o�[�g�g�U���ˌv�Z�֐�
//--------------------------------------------
// N:�@��(���K���ς�)
// L:���˃x�N�g��(���K���ς�)
// C:���ˌ�(�F�E����)
// K:���˗�
float3 calc_lambert(float3 N, float3 L, float3 C)
{
    float power = saturate(dot(N, -L));
    return C * power;
}

//--------------------------------------------
//	�t�H���̋��ʔ��ˌv�Z�֐�
//--------------------------------------------
// N:�@��(���K���ς�)
// L:���˃x�N�g��(���K���ς�)
// E:�����x�N�g��(���K���ς�)
// C:���ˌ�(�F�E����)
// K:���˗�
float3 calc_phong_specular(float3 N, float3 L, float3 E, float3 C)
{
    float3 R = reflect(L, N);
    float power = max(dot(-E, R), 0);
    power = pow(power, 128);
    return C * power;
}

//--------------------------------------------
//	�n�[�t�����o�[�g�g�U���ˌv�Z�֐�
//--------------------------------------------
// N:�@��(���K���ς�)
// L:���˃x�N�g��(���K���ς�)
// C:���ˌ�(�F�E����)
// K:���˗�
float3 clac_half_lambert(float3 N, float3 L, float3 C)
{
    float D = saturate(dot(N, -L) * 0.5f + 0.5f);
    return C * D;
}