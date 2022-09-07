#include"StraightBullet.h"
#include"Operators.h"
#include"user.h"
StraightBullet::StraightBullet(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 InitPoint_, DirectX::XMFLOAT3 mVelocity_)
    :BaseBullet(Graphics_, "./resources/Effect/arrow.efk",{})
{
    mLifeTime = 10.0f;
    mPosition = InitPoint_;
    mVelocity = mVelocity_;
    mScale = { 0.8f,0.8f,0.8f };
    // �i�s��������e�̉�]�l���v�Z����
    DirectX::XMFLOAT3 up{ 0.001f,1.0f,0.0f };
    up = Math::Normalize(up);
    // �I�_�Ƃ̃x�N�g��
    const auto n_Velocity = Math::Normalize(mVelocity);
    // �O�ςŉ�]�����Z�o
    auto cross = Math::Cross(up, n_Velocity);
    cross = Math::Normalize(cross);
    // �������O�̎��N���b�V������̂��߉��̒l����
    if (Math::Length(cross) <= 0.0f)
    {
        cross = { 0.0f,1.0f,0.0f };
    }
    // ���ςŉ�]�p���Z�o
    auto dot = Math::Dot(up, n_Velocity);
    dot = acosf(dot);
    const DirectX::XMFLOAT4 dummy{ 0.0f,0.0f,0.0f,1.0f };
    mOrientation = Math::RotQuaternion(dummy, cross, dot);
}

void StraightBullet::fUpdate(float elapsed_time)
{

    mPosition += (mVelocity);
    mLifeTime -= elapsed_time;
    mIsAlive = mLifeTime >= 0.0f;
    BaseBullet::fUpdate(elapsed_time);
}