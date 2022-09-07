#include "BaseBullet.h"
#include "effect_manager.h"
#include "Operators.h"
BaseBullet::BaseBullet(GraphicsPipeline& Graphics_, const char* FileName_,
    DirectX::XMFLOAT3 EffectPosition_)
{
    mpEffect = std::make_unique<Effect>(
        Graphics_,
        effect_manager->get_effekseer_manager(),
        FileName_);
    mpEffect->play(effect_manager->get_effekseer_manager(), EffectPosition_,13.0f);
    mIsAlive = true;
}

BaseBullet::~BaseBullet()
{
    mpEffect->stop(effect_manager->get_effekseer_manager());
}

void BaseBullet::fUpdate(float elapsedTime_)
{
    mpEffect->set_position(effect_manager->get_effekseer_manager(), mPosition);
    mpEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);
    mpEffect->set_scale(effect_manager->get_effekseer_manager(), mScale);
    setCapsuleData();
}

bool BaseBullet::fGetIsAlive() const
{
    return mIsAlive;
}

void BaseBullet::setCapsuleData()
{
    DirectX::XMFLOAT3 up = Math::GetUp(mOrientation);
    mBulletData.mPointA = mPosition + (up * mBulletData.mLengthFromPositionA);
    mBulletData.mPointB = mPosition - (up * mBulletData.mLengthFromPositionB);
}
