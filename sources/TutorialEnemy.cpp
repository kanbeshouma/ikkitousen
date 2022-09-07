#include"TutorialEnemy.h"
#include"Operators.h"
TutorialEnemy_NoAttack::TutorialEnemy_NoAttack(GraphicsPipeline& graphics,
                                               const DirectX::XMFLOAT3& entry_position, const EnemyParamPack& param ): BaseEnemy(graphics,
                                                                                                                                 "./resources/Models/Enemy/enemy_tutorial.fbx", param, entry_position)
{
    mPosition = entry_position;
    mScale = { 0.06f,0.06f,0.06f };
    fRegisterFunctions();
}

TutorialEnemy_NoAttack::TutorialEnemy_NoAttack(GraphicsPipeline& graphics)
: BaseEnemy(graphics, "./resources/Models/Enemy/enemy_tutorial.fbx")
{}

void TutorialEnemy_NoAttack::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_=fBaseUpdate(elapsedTime_, Graphics_);
}

void TutorialEnemy_NoAttack::fUpdateAttackCapsule()
{

}

void TutorialEnemy_NoAttack::fRegisterFunctions()
{
    {
        InitFunc Ini = [=]()->void
        {
            fStartInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStartUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Start, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fIdleInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fIdleUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Idle, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fStunInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStunUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Stun, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fMoveInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fMoveUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Move, tuple));
    }
    fChangeState(DivideState::Start);
}

void TutorialEnemy_NoAttack::fStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    mDissolve = 1.0f;
}

void TutorialEnemy_NoAttack::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if(mDissolve<0.0f)
    {
        fChangeState(DivideState::Idle);
    }
}

void TutorialEnemy_NoAttack::fIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle);
}

void TutorialEnemy_NoAttack::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::Move);
    }
}

void TutorialEnemy_NoAttack::fMoveInit()
{
   // 一定範囲内をランダムで動く
    const std::uniform_int_distribution<int> RandTargetAdd(0, 5);
    const int randNumber = RandTargetAdd(mt);

   const float radian{ DirectX::XMConvertToRadians(static_cast<float>(randNumber) * (360.0f / 6)) };

    // 回転量と距離から移動先を割り出す
    DirectX::XMFLOAT3 randPosition = { cosf(radian),0.0f,sinf(radian) };

    mWonderTarget = { randPosition * 40.0f };
    mWonderBegin = mPosition;
    mTimer = 0.0f;
}

void TutorialEnemy_NoAttack::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_ * 0.5f;
    fTurnToTarget(elapsedTime_, 10.0f, mWonderTarget);
    mPosition = Math::lerp(mWonderBegin, mWonderTarget, mTimer);
    if(mTimer>=1.0f)
    {
        fChangeState(DivideState::Idle);
    }

}

void TutorialEnemy_NoAttack::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::stun);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);

    mTimer = mStunTime;
}

void TutorialEnemy_NoAttack::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer -= elapsedTime_;

    if(mTimer<=0.0f)
    {
        mStunEffect->stop(effect_manager->get_effekseer_manager());
        mIsStun = false;
        fChangeState(DivideState::Idle);
    }
}

TutorialEnemy::TutorialEnemy(GraphicsPipeline& graphics, const DirectX::XMFLOAT3& entry_position,
    const EnemyParamPack& param)
        :BaseEnemy(graphics, "./resources/Models/Enemy/enemy_ace_tutorial.fbx", param, entry_position)
{
    mPosition = entry_position;
    mScale = { 0.06f,0.06f,0.06f };
    fRegisterFunctions();

    // ボーンを初期化
    bone = mpModel->get_bone_by_name("hand_r_joint");
}

TutorialEnemy::TutorialEnemy(GraphicsPipeline& graphics)
    : BaseEnemy(graphics, "./resources/Models/Enemy/enemy_ace_tutorial.fbx")
{
    throw std::logic_error("Not implemented");
}

void TutorialEnemy::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
   elapsedTime_= fBaseUpdate(elapsedTime_, Graphics_);
   fUpdateAttackCapsule();
}

void TutorialEnemy::fUpdateAttackCapsule()
{
    DirectX::XMFLOAT3 position{};
    DirectX::XMFLOAT3 up{};

    const auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    mpModel->fech_by_bone(mAnimPara, world, bone, position, up);

    mAttackCapsule.mBottom = position;
    mAttackCapsule.mTop = position + (10.0f * up);
}

void TutorialEnemy::fRegisterFunctions()
{
    {
        InitFunc Ini = [=]()->void
        {
            fStartInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStartUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Start, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fIdleInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fIdleUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Idle, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fStunInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStunUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Stun, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fMoveInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fMoveUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Move, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fAttackStartInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackStartUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::AttackStart, tuple));
    }
    {
        InitFunc Ini = [=]()->void
        {
            fAttackInit();
        };
        UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackUpdate(elapsedTime_, Graphics_);
        };
        FunctionTuple tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivideState::Attack, tuple));
    }
    fChangeState(DivideState::Start);
}

void TutorialEnemy::fStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    mDissolve = 1.0f;
}

void TutorialEnemy::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if (mDissolve < 0.0f)
    {
        fChangeState(DivideState::Idle);
    }
}

void TutorialEnemy::fIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle);
}

void TutorialEnemy::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::Move);
    }
}

void TutorialEnemy::fMoveInit()
{
    // 一定範囲内をランダムで動く
    const std::uniform_int_distribution<int> RandTargetAdd(0, 5);
    const int randNumber = RandTargetAdd(mt);

    const float radian{ DirectX::XMConvertToRadians(static_cast<float>(randNumber) * (360.0f / 6)) };

    // 回転量と距離から移動先を割り出す
    DirectX::XMFLOAT3 randPosition = { cosf(radian),0.0f,sinf(radian) };

    mWonderTarget = { randPosition * 40.0f };
    mWonderBegin = mPosition;
    mTimer = 0.0f;

    mpModel->play_animation(mAnimPara, AnimationName::walk, true);
}

void TutorialEnemy::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_ * 0.5f;
    fTurnToTarget(elapsedTime_, 10.0f, mWonderTarget);
    mPosition = Math::lerp(mWonderBegin, mWonderTarget, mTimer);
    if (mTimer >= 1.0f)
    {
        fChangeState(DivideState::AttackStart);
    }
}

void TutorialEnemy::fAttackStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_ready);
}

void TutorialEnemy::fAttackStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    float length = Math::Length(mPlayerPosition - mPosition);
    DirectX::XMFLOAT3 velPower = Math::Normalize(mPlayerPosition - mPosition);
    mPosition += velPower * elapsedTime_ * 40.0f;

    if(length<=10.0f)
    {
        fChangeState(DivideState::Attack);
    }
}

void TutorialEnemy::fAttackInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack);
    mIsAttack = true;
}
void TutorialEnemy::fAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    if(mpModel->end_of_animation(mAnimPara))
    {
        mIsAttack = false;
        fChangeState(DivideState::Idle);
    }
}

void TutorialEnemy::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::stun);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    audio_manager->play_se(SE_INDEX::STAN);

    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    mIsAttack = false;
    mTimer = 0;
}

void TutorialEnemy::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    if (mTimer >= mStunTime)
    {
        fChangeState(DivideState::Idle);
        mStunEffect->stop(effect_manager->get_effekseer_manager());
        mIsStun = false;
    }
}
