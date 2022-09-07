#include "effect.h"

Effect::Effect(GraphicsPipeline& graphics, Effekseer::Manager* effekseer_manager, const char* filename)
{
    std::lock_guard<std::mutex> lock(graphics.get_mutex());
    //Effekseerのリソースを読み込む
    //EffekseerはUTF_16のファイルパス以外は対応していないので文字コード変換が必要
    char16_t utf16_filename[256];
    Effekseer::ConvertUtf8ToUtf16(utf16_filename, 256, filename);
    //Effekseer::Managerを取得
    //Effekseerエフェクトを読み込み
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
//再生
//----------------------------------------------//
void Effect::play(Effekseer::Manager* effekseer_manager, const DirectX::XMFLOAT3& position, float scale)
{
    effekseer_handle = effekseer_manager->Play(effekseer_effect, position.x, position.y, position.z);
    effekseer_manager->SetScale(effekseer_handle, scale, scale, scale);
    //Effekseer::Color c = { static_cast<uint8_t>(255),static_cast<uint8_t>(255),static_cast<uint8_t>(255),static_cast<uint8_t>(120) };
    //effekseer_manager->SetAllColor(effekseer_handle, c);
}

//-----------------------------------------------//
//停止
//----------------------------------------------//
void Effect::stop(Effekseer::Manager* effekseer_manager)
{
    effekseer_manager->StopEffect(effekseer_handle);
}

//-----------------------------------------------//
//再生位置設定
//----------------------------------------------//
void Effect::set_position(Effekseer::Manager* effekseer_manager, const DirectX::XMFLOAT3& position)
{
    effekseer_manager->SetLocation(effekseer_handle, position.x, position.y, position.z);
}

//-----------------------------------------------//
//スケール設定
//----------------------------------------------//
void Effect::set_scale(Effekseer::Manager* effekseer_manager, const DirectX::XMFLOAT3& scale)
{
    effekseer_manager->SetScale(effekseer_handle, scale.x, scale.y, scale.z);
}

//-----------------------------------------------//
//再生スピード設定
//----------------------------------------------//
void Effect::set_speed(Effekseer::Manager* effekseer_manager, float speed)
{
    effekseer_manager->SetSpeed(effekseer_handle, speed);
}

//-----------------------------------------------//
//Y軸回転
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
    //クォータニオン→回転行列
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
//エフェクトの回転行列を任意の回転行列に合わせる
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
//XMFLOAT4X4からMatrix43への変換関数
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
    

