#include"SwordEnemyAce.h"
#include"Operators.h"
SwordEnemy_Ace::SwordEnemy_Ace(GraphicsPipeline& Graphics_, const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_ace_sword.fbx",ParamPack_,EmitterPoint_)
{
    SwordEnemy_Ace::fRegisterFunctions();
    // ボーンを初期化
    mScale = { 0.05f,0.05f,0.05f };
    mVernierEffect->stop(effect_manager->get_effekseer_manager());
    mBone = mpModel->get_bone_by_name("sword_handle_ctr");
}

SwordEnemy_Ace::SwordEnemy_Ace(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_ace_sword.fbx")
{}

SwordEnemy_Ace::~SwordEnemy_Ace()
{}

void SwordEnemy_Ace::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_=fBaseUpdate(elapsedTime_,Graphics_);
    mIsHit = false;
}

void SwordEnemy_Ace::fUpdateAttackCapsule()
{
    throw std::logic_error("Not implemented");
}


void SwordEnemy_Ace::fRegisterFunctions()
{
    {
        InitFunc ini = [=]()->void
        {
            fStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Start, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Idle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fMoveInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fMoveUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Move, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fCounterStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fCounterStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::CounterStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fCounterMiddleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fCounterMiddleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::CounterMiddle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fCounterEndInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fCounterEndUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::CounterAttack, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fStunInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStunUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Stun, tuple));
    }
    fChangeState(DivideState::Start);
}

void SwordEnemy_Ace::fStartInit()
{
    mpModel->play_animation(mAnimPara, 0, false, false);
}

void SwordEnemy_Ace::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if(mDissolve<=0.0f)
    {
        fChangeState(DivideState::Idle);
    }
}

void SwordEnemy_Ace::fIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    mWaitTimer = 0.0f;
}

void SwordEnemy_Ace::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    if(mWaitTimer>=3.0f)
    {
        fChangeState(DivideState::Move);
    }
}

void SwordEnemy_Ace::fCounterStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ace_attack_ready);
    mWaitTimer = 0.0f;
}

void SwordEnemy_Ace::fCounterStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
  if(mWaitTimer>3.0f)
  {
      fChangeState(DivideState::CounterMiddle);
  }
}

void SwordEnemy_Ace::fCounterMiddleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ace_attack_idle, true);
    mWaitTimer = 0.0f;
    mIsWaitCounter = true;
}

void SwordEnemy_Ace::fCounterMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    mWaitTimer += elapsedTime_;
    if(mWaitTimer>=3.0f)
    {
        fChangeState(DivideState::Idle);
        mIsWaitCounter = false;
    }

    // ダメージを受けたら
    if(mIsHit)
    {
        fChangeState(DivideState::CounterAttack);
        mIsWaitCounter = false;
    }
    
}

void SwordEnemy_Ace::fCounterAttackInit()
{
    mpModel->play_animation(AnimationName::ace_attack, false, true, 0.3f, 2.0f);
    mWaitTimer = 0.0f;
    mAnimationSpeed = 2.0f;
    mIsAttack = true;
}

void SwordEnemy_Ace::fCounterAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 pos{};
    DirectX::XMFLOAT3 up{};
    mpModel->fech_by_bone(mAnimPara, world, mBone, pos, up);
    mAttackCapsule.mBottom = pos;
    mAttackCapsule.mTop = pos + up * 10.0f;
    mAttackCapsule.mRadius = 5.0f;
    

    mWaitTimer += elapsedTime_;
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::CounterEnd);
        mAnimationSpeed = 1.0f;
        mIsAttack = false;
    }
}

void SwordEnemy_Ace::fCounterEndInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ace_attack_end);
}

void SwordEnemy_Ace::fCounterEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
   if(mpModel->end_of_animation(mAnimPara))
   {
       fChangeState(DivideState::Idle);
   }
}

void SwordEnemy_Ace::fMoveInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::walk);
}

void SwordEnemy_Ace::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    DirectX::XMFLOAT3 vec = mPlayerPosition - mPosition;
    vec = Math::Normalize(vec);
    mPosition += vec * elapsedTime_ * 30.0f;

    if(fGetLengthFromPlayer()<=10.0f)
    {
        fChangeState(DivideState::CounterStart);
    }
}

void SwordEnemy_Ace::fStunInit()
{
    mpModel->play_animation(AnimationName::stun, true);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    audio_manager->play_se(SE_INDEX::STAN);

    mWaitTimer = mStunTime;
    mIsAttack = false;
}

void SwordEnemy_Ace::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer -= elapsedTime_;
    if(mWaitTimer<=0.0f)
    {
        mStunEffect->stop(effect_manager->get_effekseer_manager());
        mIsStun = false;
        fChangeState(DivideState::Idle);
    }
}

bool SwordEnemy_Ace::fDamaged(int damage, float invincible_time, GraphicsPipeline& Graphics_, float elapsedTime_)
{
    bool ret = false;
    if (mInvincibleTime <= 0.0f)
    {
        mIsHit = true;
        if (!mIsWaitCounter||mIsStun)
        {
            mCurrentHitPoint -= damage;
            mInvincibleTime = invincible_time;
            ret = true;
        }
    }
   
    return ret;
}

void SwordEnemy_Ace::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivideState::Stun);
    }
}

