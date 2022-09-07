#pragma once

#include"effect.h"
#include"graphics_pipeline.h"
#include"practical_entities.h"
#include<memory>
//****************************************************************
// 
// íeÇÃäÓíÍÉNÉâÉX 
// 
//****************************************************************
class BaseBullet : public PracticalEntities
{
public:
    //****************************************************************
    // 
    // ä÷êî
    // 
    //****************************************************************
    BaseBullet(GraphicsPipeline& Graphics_, const char* EffectFileName_,DirectX::XMFLOAT3 EffectPosition_);
    ~BaseBullet() override;

    virtual void fUpdate(float elapsedTime_);

    struct BulletData
    {
        DirectX::XMFLOAT3 mPointA{};
        float mLengthFromPositionA{};
        DirectX::XMFLOAT3 mPointB{};
        float mLengthFromPositionB{};
        float mRadius{};
        int mDamage;
        float mInvincible_time;
    };

    BulletData fGetBulletData() const { return mBulletData; }
    bool fGetIsAlive()const;
    void fSetBulletData(float mLengthA, float mLengthB, float radius, int damage, float invincible_time)
    {
        mBulletData.mLengthFromPositionA = mLengthA;
        mBulletData.mLengthFromPositionB = mLengthB;
        mBulletData.mRadius = radius;
        mBulletData.mDamage = damage;
        mBulletData.mInvincible_time = invincible_time;
    }

    void setCapsuleData();
protected:
    //****************************************************************
    // 
    // ïœêî
    // 
    //****************************************************************
    std::unique_ptr<Effect> mpEffect{ nullptr };
    DirectX::XMFLOAT3 mPosition{};
    DirectX::XMFLOAT3 mScale{};
    DirectX::XMFLOAT4 mOrientation{};
    bool mIsAlive{}; // íeÇÃê∂ë∂îªíË
    DirectX::XMFLOAT3 mVelocity{}; // ë¨ìx
public:
    BulletData mBulletData{};

};
