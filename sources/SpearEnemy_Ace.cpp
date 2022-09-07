#include"SpearEnemy_Ace.h"
#include"Operators.h"
SpearEnemy_Ace::SpearEnemy_Ace(GraphicsPipeline& Graphics_,
                               const DirectX::XMFLOAT3& EmitterPoint_,
                               const EnemyParamPack& ParamPack_)
:BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_ace_spear.fbx",
    ParamPack_,EmitterPoint_)
{
    fRegisterFunctions();
    mScale = { 0.05f,0.05f,0.05f };
    mpSpearBoneBottom = mpModel->get_bone_by_name("spear_ctr");
    mpSpearBoneTop = mpModel->get_bone_by_name("spear_top_ctr");
    mVernierEffect->stop(effect_manager->get_effekseer_manager());

}

SpearEnemy_Ace::SpearEnemy_Ace(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_ace_spear.fbx")
{}

void SpearEnemy_Ace::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_ = fBaseUpdate(elapsedTime_, Graphics_);
    fUpdateAttackCapsule();
}

void SpearEnemy_Ace::fUpdateAttackCapsule()
{
    const DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 up{};
    mpModel->fech_by_bone(mAnimPara,
                            world, 
                            mpSpearBoneTop,
                          mAttackCapsule.mTop, up);
    mpModel->fech_by_bone(mAnimPara, 
                            world,
                            mpSpearBoneBottom,
                          mAttackCapsule.mBottom, up);
}


void SpearEnemy_Ace::fRegisterFunctions()
{
    // 関数をステートマシンに登録
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
            fWipeBeginInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fWipeBeginUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::WipeBegin, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fWipeAttackInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fWipeAttackUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::WipeAttack, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fStunInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStunUpdate(elapsedTime_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Stun, tuple));
    }
    fChangeState(DivideState::Start);
}

void SpearEnemy_Ace::fStartInit()
{
    
}
void SpearEnemy_Ace::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if(mDissolve<=0.0f)
    {
        fChangeState(DivideState::Idle);
    }
}

void SpearEnemy_Ace::fIdleInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::ace_attack_ready);
}

void SpearEnemy_Ace::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 20.0f);
   if(mpModel->end_of_animation(mAnimPara))
   {
       fChangeState(DivideState::Move);
   }
}

void SpearEnemy_Ace::fMoveInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ace_attack_idle,true);
}

void SpearEnemy_Ace::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 20.0f);
    // プレイヤー方向に進む
    const DirectX::XMFLOAT3 vec = mPlayerPosition - mPosition;
    const DirectX::XMFLOAT3 norm = Math::Normalize(vec);
    mPosition += (norm * elapsedTime_ * 60.0f);
    if(Math::Length(vec)<=5.0f)
    {
        fChangeState(DivideState::WipeBegin);
    }
}

void SpearEnemy_Ace::fWipeBeginInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ace_attack_start);
}

void SpearEnemy_Ace::fWipeBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::WipeAttack);
    }
}

void SpearEnemy_Ace::fWipeAttackInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ace_attack);
    mIsAttack = true;
    mAnimationSpeed = 1.5f;
}

void SpearEnemy_Ace::fWipeAttackUpdate( float elapsedTime, GraphicsPipeline& Graphics_)
{
    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::Idle);
        mIsAttack = false;
        mAnimationSpeed = 1.0f;
    }
}

void SpearEnemy_Ace::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::stun);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    audio_manager->play_se(SE_INDEX::STAN);

    mWaitTimer = mStunTime;
    mIsAttack = false;
}
void SpearEnemy_Ace::fStunUpdate(float elapsedTime_)
{
    mWaitTimer -= elapsedTime_;
    if(mWaitTimer<=0.0f)
    {
        fChangeState(DivideState::Idle);
        mStunEffect->stop(effect_manager->get_effekseer_manager());

        mIsStun = false;
    }
}

void SpearEnemy_Ace::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivideState::Stun);
    }
}

