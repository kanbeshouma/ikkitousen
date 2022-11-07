#include "effect.h"

Effect::Effect(GraphicsPipeline& graphics, Effekseer::Manager* effekseer_manager, const char* filename)
{
    std::lock_guard<std::mutex> lock(graphics.get_mutex());
    //Effekseer�̃��\�[�X��ǂݍ���
    //Effekseer��UTF_16�̃t�@�C���p�X�ȊO�͑Ή����Ă��Ȃ��̂ŕ����R�[�h�ϊ����K�v
    char16_t utf16_filename[256];
    Effekseer::ConvertUtf8ToUtf16(utf16_filename, 256, filename);
    //Effekseer::Manager���擾
    //Effekseer�G�t�F�N�g��ǂݍ���
    effekseer_effect = Effekseer::Effect::Create(effekseer_manager, (EFK_CHAR*)utf16_filename);
}

Effect::~Effect()
{
    if (effekseer_effect != nullptr)
    {
        effekseer_effect->Release();
        effekseer_effect = nullptr;
    }
}

//-----------------------------------------------//
//�Đ�
//----------------------------------------------//
void Effect::play(Effekseer::Manager* effekseer_manager, const DirectX::XMFLOAT3& position, float scale)
{
        effekseer_handle = effekseer_manager->Play(effekseer_effect, position.x, position.y, position.z);
        effekseer_manager->SetScale(effekseer_handle, scale, scale, scale);
    //Effekseer::Color c = { static_cast<uint8_t>(255),static_cast<uint8_t>(255),static_cast<uint8_t>(255),static_cast<uint8_t>(120) };
    //effekseer_manager->SetAllColor(effekseer_handle, c);
}

//-----------------------------------------------//
//��~
//----------------------------------------------//
void Effect::stop(Effekseer::Manager* effekseer_manager)
{
    effekseer_manager->StopEffect(effekseer_handle);
}

//-----------------------------------------------//
//�Đ��ʒu�ݒ�
//----------------------------------------------//
void Effect::set_position(Effekseer::Manager* effekseer_manager, const DirectX::XMFLOAT3& position)
{
    effekseer_manager->SetLocation(effekseer_handle, position.x, position.y, position.z);
}

//-----------------------------------------------//
//�X�P�[���ݒ�
//----------------------------------------------//
void Effect::set_scale(Effekseer::Manager* effekseer_manager, const DirectX::XMFLOAT3& scale)
{
   effekseer_manager->SetScale(effekseer_handle, scale.x, scale.y, scale.z);
}

//-----------------------------------------------//
//�Đ��X�s�[�h�ݒ�
//----------------------------------------------//
void Effect::set_speed(Effekseer::Manager* effekseer_manager, float speed)
{
    effekseer_manager->SetSpeed(effekseer_handle, speed);
}

//-----------------------------------------------//
//Y����]
//----------------------------------------------//
void Effect::set_rotationY(Effekseer::Manager* effekseer_manager, const float angle)
{
    Effekseer::Vector3D up{ 0,1,0 };
    effekseer_manager->SetRotation(effekseer_handle, up, angle);
}

void Effect::set_rotation_axis(Effekseer::Manager* effekseer_manager, const DirectX::XMFLOAT3& axis, const float angle)
{
    Effekseer::Vector3D Axis = { axis.x,axis.y,axis.z };
    effekseer_manager->SetRotation(effekseer_handle, Axis, angle);

}


void Effect::set_quaternion(Effekseer::Manager* effekseer_manager, DirectX::XMFLOAT4 orientation)
{
    //�N�H�[�^�j�I������]�s��
    auto transformQuaternionToRotMat = [&](DirectX::XMFLOAT4X4& q,
        DirectX::XMFLOAT4 o)
    {
        q._11 = 1.0f - 2.0f * o.y * o.y - 2.0f * o.z * o.z;
        q._12 = 2.0f * o.x * o.y + 2.0f * o.w * o.z;
        q._13 = 2.0f * o.x * o.z - 2.0f * o.w * o.y;

        q._21 = 2.0f * o.x * o.y - 2.0f * o.w * o.z;
        q._22 = 1.0f - 2.0f * o.x * o.x - 2.0f * o.z * o.z;
        q._23 = 2.0f * o.y * o.z + 2.0f * o.w * o.x;

        q._31 = 2.0f * o.x * o.z + 2.0f * o.w * o.y;
        q._32 = 2.0f * o.y * o.z - 2.0f * o.w * o.x;
        q._33 = 1.0f - 2.0f * o.x * o.x - 2.0f * o.y * o.y;
    };
    DirectX::XMFLOAT4X4 r_mat{};

    transformQuaternionToRotMat(r_mat, orientation);
    static float ang = 0;
   set_posture(effekseer_manager, r_mat, ang);
}

void Effect::set_orient(Effekseer::Manager* effekseer_manager, DirectX::XMFLOAT3& orient)
{

}



//-----------------------------------------------//
//�G�t�F�N�g�̉�]�s���C�ӂ̉�]�s��ɍ��킹��
//----------------------------------------------//
void Effect::set_posture(Effekseer::Manager* effekseer_manager, DirectX::XMFLOAT4X4& rotate_mat,float ang)
{

    Effekseer::Matrix43 rotate_mat_efec = transform_XMFLOAT4X4toMatrix43(rotate_mat);

    Effekseer::Matrix43 base_mat = effekseer_manager->GetMatrix(effekseer_handle);

    Effekseer::Vector3D scale{};
    base_mat.GetScale(scale);

    Effekseer::Vector3D pos{};
    base_mat.GetTranslation(pos);
   // rotate_mat_efec.RotationZ(DirectX::XMConvertToRadians(ang));
    Effekseer::Matrix43 srt{};
    srt.SetSRT(scale, rotate_mat_efec, pos);
    effekseer_manager->SetMatrix(effekseer_handle, srt);
}


//-----------------------------------------------//
//XMFLOAT4X4����Matrix43�ւ̕ϊ��֐�
//----------------------------------------------//
Effekseer::Matrix43 Effect::transform_XMFLOAT4X4toMatrix43(DirectX::XMFLOAT4X4& mat_4x4)
{

    Effekseer::Matrix43 matrix_4x3{};

    matrix_4x3.Value[0][0] = mat_4x4._11;
    matrix_4x3.Value[0][1] = mat_4x4._12;
    matrix_4x3.Value[0][2] = mat_4x4._13;

    matrix_4x3.Value[1][0] = mat_4x4._21;
    matrix_4x3.Value[1][1] = mat_4x4._22;
    matrix_4x3.Value[1][2] = mat_4x4._23;

    matrix_4x3.Value[2][0] = mat_4x4._31;
    matrix_4x3.Value[2][1] = mat_4x4._32;
    matrix_4x3.Value[2][2] = mat_4x4._33;

    matrix_4x3.Value[3][0] = mat_4x4._41;
    matrix_4x3.Value[3][1] = mat_4x4._42;
    matrix_4x3.Value[3][2] = mat_4x4._43;

    return matrix_4x3;
}


