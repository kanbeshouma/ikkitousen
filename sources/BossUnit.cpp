#include"BossUnit.h"
#include"Operators.h"
#include <random>

BossUnit::BossUnit(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EntryPoint_,
    const EnemyParamPack& ParamPack_,
    AddBulletFunc fAddBullet_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/boss_unit.fbx",
        ParamPack_, EntryPoint_), mStartPosition(EntryPoint_)
{
    mScale = { 0.25f,0.25f,0.25f };
    mDissolve = 1.0f;
    fRegisterFunctions();

    mWarningLine.fInitialize(Graphics_.get_device().Get(),L"");
    mWarningLine.fSetColor({ 1.0f,0.0f,0.0f,1.0f });
    mWarningLine.fSetRadius(0.05f);
    mLifeTimer = mkLifeTime;

    mpBeamEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(),
        "./resources/Effect/unit_beam.efk");
}

BossUnit::BossUnit(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/boss_unit.fbx")
{}

void BossUnit::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_=fBaseUpdate(elapsedTime_, Graphics_);
    mWarningLine.fUpdate();

    if (!mIsPlayerSearch && !mIsStun)
    {
        mLifeTimer -= elapsedTime_;
    }

    if (mLifeTimer < 0.0f)
    {
        mIsAlive = false;
        mVernierEffect->stop(effect_manager->get_effekseer_manager());
    }

    

    fGuiMenu();
}

void BossUnit::fUpdateAttackCapsule()
{
    throw std::logic_error("Not implemented");
}

void BossUnit::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivideState::Stun);
        mpBeamEffect->stop(effect_manager->get_effekseer_manager());
    }
}

void BossUnit::fRegisterFunctions()
{
    {
        InitFunc ini = [=]()->void
        {
            fIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Idle,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Start,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fWonderInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fWonderUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Wonder,
            tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fAttackChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fAttackChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::AttackBegin,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fAttackBeamInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fAttackBeamUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::AttackBeam,
            tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fStunInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fStunUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Stun,
            tuple));
    }


    fChangeState(DivideState::Start);
}

void BossUnit::fSelectWonderOrAttack()
{
    const std::uniform_int_distribution<int> RandTargetAdd(0, 2);
    if (RandTargetAdd(mt) == 0)
    {
        fChangeState(DivideState::AttackBegin);
    }
    else
    {
        fChangeState(DivideState::Wonder);
    }
}

void BossUnit::fResetLaser()
{
    mWarningLine.fSetAlpha(1.0f);


    mWarningLine.fSetLengthThreshold(0.0f);

    mBeamThreshold = 0.0f;
    mWarningThreshold = 0.0f;
    mIsAttack = false;
}

void BossUnit::fGuiMenu()
{
#ifdef USE_IMGUI
    ImGui::Begin("BossUnit");
    ImGui::Text("IsAttack:");
    ImGui::SameLine();
    ImGui::Text(mIsAttack ? "true" : "false");

    ImGui::Text("OnPlayer:");
    ImGui::SameLine();
    ImGui::Text(mOnPlayer ? "true" : "false");

    ImGui::Checkbox("PlayerSearch", &mIsPlayerSearch);
    ImGui::Checkbox("Stun", &mIsStun);
    ImGui::DragFloat("LifeTime", &mLifeTimer);

    ImGui::End();
#endif
}

void BossUnit::fStartInit()
{
    mDissolve = 1.0f;
    mpModel->play_animation(mAnimPara, AnimationName::IDLE, true);
}

void BossUnit::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if(mDissolve<=0.0f)
    {
        fChangeState(DivideState::Idle);
    }
}

void BossUnit::fIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::IDLE);
    mTimer = 0.0f;
}

void BossUnit::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if (mpModel->end_of_animation(mAnimPara))
    {
        fSelectWonderOrAttack();
    }
}

void BossUnit::fWonderInit()
{
    const std::uniform_int_distribution<int> RandTargetAdd(0, 5);
    float rot = RandTargetAdd(mt) * 60.0f;
    rot = DirectX::XMConvertToRadians(rot);
    mMoveVec = { cosf(rot),0.0f,sinf(rot) };
    mTimer = 0.0f;
    mpModel->play_animation(mAnimPara, AnimationName::MOVE);
    mMoveRev = false;
}

void BossUnit::fWonderUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    mPosition += (mMoveVec * elapsedTime_ * mMoveSpeed);

    if(Math::Length(mPosition-mStartPosition)>length&&mMoveRev==false)
    {
        mMoveVec = Math::Normalize(mStartPosition - mPosition);
        mMoveRev = true;
    }

    // 進行方向に回転
   // プレイヤーの方向に回転
    constexpr DirectX::XMFLOAT3 up = { 0.001f,1.0f,0.0f };

    const DirectX::XMFLOAT3 nextPos= mPosition + (mMoveVec * elapsedTime_ * mMoveSpeed);
    float rotSpeed = 10.0f;

    // プレイヤーとのベクトル
    const DirectX::XMFLOAT3 vToPlayer = Math::Normalize(nextPos - mPosition);
    // 自分の正面ベクトル
    const auto front = Math::Normalize(Math::GetFront(mOrientation));
    float dot = Math::Dot(vToPlayer, front);

    dot = acosf(dot);

    if (fabs(dot) > DirectX::XMConvertToRadians(2.0f))
    {
        DirectX::XMVECTOR q;
        float cross{ (vToPlayer.x * front.z) - (vToPlayer.z * front.x) };
        if (cross > 0)
        {
            mOrientation = Math::RotQuaternion(mOrientation, up, dot * rotSpeed * elapsedTime_);
        }
        else
        {
            mOrientation = Math::RotQuaternion(mOrientation, up, -dot * rotSpeed * elapsedTime_);
        }
    }



    if(mTimer>=mkWalkTime)
    {
        fSelectWonderOrAttack();
    }

}

void BossUnit::fAttackChargeInit()
{
    fResetLaser();

   // 順番にランダム・プレイヤー方向を切り替える
    mOnPlayer = msOnPlayer;
    msOnPlayer = !msOnPlayer;

   if(mOnPlayer)
   {
       mWarningLine.fSetPosition(mPosition, mPlayerPosition);
   }
   else
   {
       const std::uniform_real_distribution<float> RandTargetAdd(-50.0f, 50.0f);

       const DirectX::XMFLOAT3 TargetAdd = { RandTargetAdd(mt), 0, RandTargetAdd(mt) };

       // この時点のプレイヤーからランダムに足したポイント
       TargetPos = mPlayerPosition + TargetAdd;
   }

   mpModel->play_animation(mAnimPara, AnimationName::BEAM_CHARGE_START);
    mTimer = 0.0f;
}

void BossUnit::fAttackChargeUpdate(float elapsedTime_, 
    GraphicsPipeline& Graphics_)
{
    mWarningThreshold += elapsedTime_ * 10.0f;
    mWarningThreshold = (std::min)(3.0f, mWarningThreshold);

    if(!mOnPlayer)
    {
        fTurnToTarget(elapsedTime_, 1.0f, TargetPos);
        const DirectX::XMFLOAT3 front = Math::GetFront(mOrientation);
        mWarningLine.fSetPosition(mPosition, mPosition + front * 100.0f);
    }
    else
    {
        if (mTimer <= mkChargeTime * 0.3f)
        {

            fTurnToPlayer(elapsedTime_, 20.0f);
            mWarningLine.fSetPosition(mPosition, mPlayerPosition);
         
        }
    }
   
    mTimer += elapsedTime_;
    if(mTimer>mkChargeTime)
    {
        fChangeState(DivideState::AttackBeam);
    }

    mWarningLine.fSetLengthThreshold(mWarningThreshold);

    if(mpModel->end_of_animation(mAnimPara))
    {
        mpModel->play_animation(mAnimPara,
            AnimationName::BEAM_CHARGE_IDLE, true);
    }

}

void BossUnit::fAttackBeamInit()
{
    mTimer = 0.0f;
    mIsAttack = true;
    mpModel->play_animation(mAnimPara, AnimationName::BEAM_SHOOT_START);
    mpBeamEffect->play(effect_manager->get_effekseer_manager(), mPosition);
}

void BossUnit::fAttackBeamUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    mBeamThreshold += elapsedTime_;
    DirectX::XMFLOAT3 pos = mPosition;
    pos.y += 5.0f;
    mAttackCapsule.mTop = pos;
    mAttackCapsule.mBottom = Math::lerp(pos,
        pos + (Math::GetFront(mOrientation) * 200.0f), mBeamThreshold);

    mpBeamEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);
    if(mTimer>mkChargeTime)
    {
        fResetLaser();
        mpBeamEffect->stop(effect_manager->get_effekseer_manager());
        fChangeState(DivideState::Idle);
        mIsAttack = false;
    }
   

    if (mpModel->end_of_animation(mAnimPara))
    {
        mpModel->play_animation(mAnimPara, AnimationName::BEAM_SHOOT_IDLE);
    }
}

void BossUnit::fStunInit()
{
    mTimer = 0.0f;
    mpModel->play_animation(mAnimPara, AnimationName::STUN, true);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);

}

void BossUnit::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    if(mTimer>=mStunTime)
    {
        fChangeState(DivideState::Idle);
        mStunEffect->stop(effect_manager->get_effekseer_manager());

        mIsStun = false;
    }
}

void BossUnit::fRender(GraphicsPipeline& graphics)
{
    BaseEnemy::fRender(graphics);
    mWarningLine.fRender(graphics);
}
