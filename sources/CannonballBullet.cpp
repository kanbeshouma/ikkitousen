#include"CannonballBullet.h"
#include"Operators.h"
CannonballBullet::CannonballBullet(GraphicsPipeline& Graphics_,
    DirectX::XMFLOAT3 FirstVec_, float Power_, DirectX::XMFLOAT3 Position_)
        :BaseBullet(Graphics_, "./resources/Effect/fire_ball.efk",Position_)
{
    mVelocity = FirstVec_ * Power_;
    mPosition = Position_;
    mScale = { 18.0f,18.0f,18.0f };

    // エフェクトを初期化
    mpBombEffect = std::make_unique<Effect>(
        Graphics_,
        effect_manager->get_effekseer_manager(),
        "./resources/Effect/bomb_2.efk");

    // 当たり判定簿大きさを決定
    mBulletData.mRadius = 10.0f;
    mBulletData.mDamage = 2;
    mBulletData.mLengthFromPositionA = 1.0f;
    mBulletData.mLengthFromPositionB = 1.0f;
    mBulletData.mInvincible_time = 0.5f;

}

CannonballBullet::~CannonballBullet()
{
    mpEffect->stop(effect_manager->get_effekseer_manager());
    // 着弾爆発エフェクトを再生
    mpBombEffect->play(effect_manager->get_effekseer_manager(), mPosition, 10.0f);
}

void CannonballBullet::fUpdate(float elapsed_time)
{

    mVelocity.y -= (mkGravity*elapsed_time);
    mPosition += (mVelocity * elapsed_time);

    if(mPosition.y<=-5.0f)
    {
        mIsAlive = false;
    }

    BaseBullet::fUpdate(elapsed_time);
}
