#include"DragonBreath.h"

DragonBreath::DragonBreath(GraphicsPipeline& graphics, const DirectX::XMFLOAT3& effect_position,
float Power,const DirectX::XMFLOAT3& Target_)
: BaseBullet(graphics, "./resources/Effect/dragon_breath.efk", effect_position)
{
    mpBombEffect = std::make_unique<Effect>(graphics,
    effect_manager->get_effekseer_manager(),BombEffectPath);

    mTargetPosition = Target_;

    fTurnToTarget(1.0f, 20.0f, mTargetPosition);
    mVelocity = Math::Normalize(Target_ - effect_position);
    mScale = { 5.0f,5.0f,5.0f };
    mPosition = effect_position;
    mPower = Power;

    mBulletData.mRadius = 20.0f;
    mBulletData.mLengthFromPositionB = 1.0f;
    mBulletData.mLengthFromPositionA = 1.0f;
    mBulletData.mDamage = 10;
    mBulletData.mInvincible_time = 1.0f;
}


DragonBreath::~DragonBreath()
{
    mpBombEffect->play(effect_manager->get_effekseer_manager(), mPosition);
    mpBombEffect->set_scale(effect_manager->get_effekseer_manager(), { 10.0f,10.0f,10.0f });
}

void DragonBreath::fUpdate(float elapsed_time)
{
    BaseBullet::fUpdate(elapsed_time);

    fTurnToTarget(elapsed_time, 10.0f, mTargetPosition);
    mPosition += elapsed_time * mVelocity * mPower;
    if(mPosition.y<=-5.0f)
    {
        mIsAlive = false;
    }
    
}

void DragonBreath::fTurnToTarget(float elapsedTime_,float RotSpeed_,const DirectX::XMFLOAT3& Target_)
{
        // �v���C���[�̕����ɉ�]
        constexpr DirectX::XMFLOAT3 up = { 0.001f,1.0f,0.0f };

        // �v���C���[�Ƃ̃x�N�g��
        const DirectX::XMFLOAT3 vToPlayer = Math::Normalize(Target_ - mPosition);
        // �����̐��ʃx�N�g��
        const auto front = Math::Normalize(Math::GetFront(mOrientation));
        float dot = Math::Dot(vToPlayer, front);

        dot = acosf(dot);

        if (fabs(dot) > DirectX::XMConvertToRadians(2.0f))
        {
            float cross{ (vToPlayer.x * front.z) - (vToPlayer.z * front.x) };
            if (cross > 0)
            {
                mOrientation = Math::RotQuaternion(mOrientation, up, dot * RotSpeed_ * elapsedTime_);
            }
            else
            {
                mOrientation = Math::RotQuaternion(mOrientation, up, -dot * RotSpeed_ * elapsedTime_);
            }
        }
}
