#include "BossRushUnit.h"
#include"Operators.h"
BossRushUnit::BossRushUnit(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/boss_animation_fifth_dive.fbx")
{
   
}

BossRushUnit::BossRushUnit(GraphicsPipeline& Graphics_, const DirectX::XMFLOAT3& EntryPosition)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/boss_animation_fifth_dive.fbx", {}, {})
{
    fRegisterFunctions();
    mScale = { 0.5f,0.5f,0.5f };
    mCurrentHitPoint = 1000;
    mAttackPower = 7.0f;
    mAttackInvTime = 2.0f;
    mIsBoss = true;

}

void BossRushUnit::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    auto time = fBaseUpdate(elapsedTime_, Graphics_);
    fUpdateAttackCapsule();
}

void BossRushUnit::fUpdateAttackCapsule()
{
    const auto front = Math::GetFront(mOrientation);

    mAttackCapsule.mBottom = mPosition + (front * 10.0);
    mAttackCapsule.mTop = mPosition - (front * 10.0f);
    mAttackCapsule.mRadius = 10.0f;
}

void BossRushUnit::fStartAppear(const DirectX::XMFLOAT3& EntryPosition)
{
    // 位置を設定
    mPosition = EntryPosition;
    fChangeState(DivideState::Start);
    mIsStart = true;
}

bool BossRushUnit::fGetIsStart() const
{
    return mIsStart;
}

void BossRushUnit::fNoneInit()
{
    mDissolve = 1.0f;
    mPosition = { 0.0f,500.0f,0.0f };
    mIsStart = false;
}

void BossRushUnit::fNoneUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // 何もしない
}

void BossRushUnit::fAppearInit()
{
    mDissolve = 1.0f;
    mpModel->play_animation(mAnimPara, AnimationName::Idle, true);
}

void BossRushUnit::fAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    mDissolve -= elapsedTime_;

    if(mDissolve<=0.0f)
    {
        fChangeState(DivideState::Change);
    }
}

void BossRushUnit::fChangeInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::BeginChange);

}

void BossRushUnit::fChangeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::Rush);
    }


    
}

void BossRushUnit::fRushInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::Rush, true);
    mIsAttack = true;
    // 対象とのベクトルを作成
    mVelocity = Math::Normalize(mPlayerPosition - mPosition);

}

void BossRushUnit::fRushUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    mPosition += (mVelocity * elapsedTime_ * 200.0f);

    if (Math::Length(mPosition) > 400.0f)
    {
        fChangeState(DivideState::End);
        mIsAttack = false;
    }
}

void BossRushUnit::fEndInit()
{
    mDissolve = 0.0f;
    mpModel->play_animation(mAnimPara, AnimationName::EndChange);
}

void BossRushUnit::fEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve += elapsedTime_;
    if(mDissolve>=1.0f)
    {
        fChangeState(DivideState::None);
    }
}

void BossRushUnit::fRegisterFunctions()
{
    {
        InitFunc ini = [=]()->void
        {
            fNoneInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fNoneUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::None,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fAppearInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fAppearUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Start,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fChangeInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fChangeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Change,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fRushInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fRushUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Rush,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fEndInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fEndUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::End,
            tuple));
    }
    fChangeState(DivideState::None);
}
