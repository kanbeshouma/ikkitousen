#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"SpearEnemy.h"
#include"Operators.h"
SpearEnemy::SpearEnemy(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_spear.fbx", ParamPack_,
        EmitterPoint_)
{
    SpearEnemy::fRegisterFunctions();
    mScale = { 0.05f,0.05f,0.05f };
}

SpearEnemy::SpearEnemy(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_spear.fbx")
{}

void SpearEnemy::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_=fBaseUpdate(elapsedTime_, Graphics_);
#ifdef USE_IMGUI
    std::string name = "SpearEnemy" + std::to_string(object_id);
    imgui_menu_bar("Enemy", name, display_scape_imgui);
    if (display_scape_imgui)
    {
        ImGui::Begin(name.c_str());
        ImGui::DragFloat3("position", &mPosition.x, 0.1f);
        ImGui::RadioButton("Master", master);
        ImGui::End();
    }
#endif
}

void SpearEnemy::fSetEnemyState(int state)
{
    //-----今のAIと同じなら処理をしない-----//
    if (ai_state == state) return;

    switch (ai_state)
    {
    case AiState::Start: fChangeState(DivedState::Start); break;
    case AiState::Idle: fChangeState(DivedState::Idle); break;
    case AiState::Move: fChangeState(DivedState::Move); break;
    case AiState::Damaged: fChangeState(DivedState::Damaged); break;
    case AiState::ThrustBegin: fChangeState(DivedState::ThrustBegin); break;
    case AiState::ThrustMiddle: fChangeState(DivedState::ThrustMiddle); break;
    case AiState::ThrustEnd: fChangeState(DivedState::ThrustEnd); break;
    case AiState::Die: fChangeState(DivedState::Die); break;
    case AiState::Stun: fChangeState(DivedState::Stun); break;
    default:
        break;
    }

}

void SpearEnemy::fUpdateAttackCapsule()
{

}


void SpearEnemy::fRegisterFunctions()
{
    // 関数をステートマシンに登録
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
        mFunctionMap.insert(std::make_pair(DivedState::Idle, tuple));
    }
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
        mFunctionMap.insert(std::make_pair(DivedState::Start, tuple));
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
        mFunctionMap.insert(std::make_pair(DivedState::Move, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fThrustBeginInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fThrustBeginUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::ThrustBegin, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fThrustMiddleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fThrustMiddleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::ThrustMiddle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fThrustEndInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fThrustEndUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::ThrustEnd, tuple));
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
        mFunctionMap.insert(std::make_pair(DivedState::Stun, tuple));
    }
    fChangeState(DivedState::Start);
}


//--------------------<開始時>--------------------//
void SpearEnemy::fStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    // 汎用タイマーを初期化
    mWaitTimer = 0.0f;
    mIsAttack = false;
}

void SpearEnemy::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if (mDissolve <= 0.0f)
    {
        //-----初回の出現の時のみエフェクト再生-----//
        if (is_appears == false)mVernierEffect->play(effect_manager->get_effekseer_manager(), mPosition);

        //-----出現フラグをtrueにする-----//
        is_appears = true;
        fChangeState(DivedState::Idle);
    }
}

//--------------------<待機>--------------------//
void SpearEnemy::fIdleInit()
{
    // アニメーションを再生
    mpModel->play_animation(mAnimPara,AnimationName::idle, true);
    ai_state = MasterAiState::Idle;
}
void SpearEnemy::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    if(mWaitTimer>=mIdleSec)
    {
        //-----マスター以外はマスターからの指示をまつ-----//
        if (master)fChangeState(DivedState::Move);
    }
}
//--------------------<移動>--------------------//
void SpearEnemy::fMoveInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::walk, true);
    // 突進先の座標を初期化
    mThrustTarget = mPlayerPosition;
    ai_state = MasterAiState::Move;
    //-----取り巻きの移動位置を決める-----//
    SetMasterSurroundingsPos();


}
void SpearEnemy::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    // プレイヤーと突進方向のなす角が一定以下になったら突進する
    auto v1 = mThrustTarget- mPosition;
    auto v2 = Math::GetFront(mOrientation);
    v1 = Math::Normalize(v1);
    v2 = Math::Normalize(v2);
    float dot = Math::Dot(v1, v2);
    dot = acosf(dot);
    dot = DirectX::XMConvertToDegrees(dot);

    // 一定距離になったら突進モーションに入る
    if(Math::Length(mPosition-mPlayerPosition)<=mAttackLength&&
        fabs(dot)<=mThrustDegree)
    {
        //-----マスター以外はマスターからの指示をまつ-----//
        if (master)fChangeState(DivedState::ThrustBegin);
        //-----あまりにも距離が近い場合は攻撃に遷移する-----//
        else if(Math::Length(mPosition - mPlayerPosition) <= mAttackLength / 2&&
            fabs(dot) <= mThrustDegree)fChangeState(DivedState::ThrustBegin);
    }
    else
    {
        //--------------------<プレイヤーのいる向きに移動>--------------------//
        fMoveFront(elapsedTime_, mMoveSpeed);
    }

    //--------------------<プレイヤーの方向に回転>--------------------//
    if (master)fTurnToPlayer(elapsedTime_, 20.0f);
    else
    {
        //-----ターゲット位置との距離を確認-----//
        CheckFollowersTargetPos();
        fTurnToTarget(elapsedTime_, 20.0, followers_target_pos);
    }
}
//--------------------<突進準備>--------------------//
void SpearEnemy::fThrustBeginInit()
{
    audio_manager->play_se(SE_INDEX::ENEMY_EMERGENCE);

    mpModel->play_animation(mAnimPara, AnimationName::attack_idle);
    mWaitTimer = 0.0f;
    mIsAttack = true;
    ai_state = MasterAiState::Attack;
    //-----攻撃動作に入ったことを知らせる-----//
    fSetAttackOperation(true);
}
void SpearEnemy::fThrustBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);
    mWaitTimer += elapsedTime_;
   if(mWaitTimer>=mThrustBeginSec)
   {
       fChangeState(DivedState::ThrustMiddle);
   }
}
//--------------------<槍を突き出すアニメーション>--------------------//
void SpearEnemy::fThrustMiddleInit()
{
    audio_manager->play_se(SE_INDEX::SPEAR_RUSH);

    // 突き中のアニメーションを再生
    mpModel->play_animation(mAnimPara, AnimationName::attack_up);
    mWaitTimer = 0.0f;
}

void SpearEnemy::fThrustMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);
    mWaitTimer += elapsedTime_;
    if(mWaitTimer >= mThrustMiddleSec)
    {
        fChangeState(DivedState::ThrustEnd);
    }
}
//--------------------<突進中>--------------------//
void SpearEnemy::fThrustEndInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_down, true);
    mWaitTimer = 0.0f;
}

void SpearEnemy::fThrustEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // 正面方向に突撃する
    const auto V = Math::GetFront(mOrientation) * mThrustSpeed * elapsedTime_;
    mPosition += V;
    mAttackCapsule.mBottom = {
        mPosition.x,
        mPosition.y + 8.0f,
        mPosition.z,
    };
    mAttackCapsule.mTop = mAttackCapsule.mBottom + (Math::GetFront(mOrientation) * 15.0f);
    mAttackCapsule.mRadius = 5.0f;


    mWaitTimer += elapsedTime_;
    if (mWaitTimer >= mThrustEndSec)
    {
        fChangeState(DivedState::Idle);
        mIsAttack = false;
        fSetAttackOperation(false);
    }}

void SpearEnemy::fDamageInit()
{
    throw std::logic_error("Not implemented");
}

void SpearEnemy::fDamageUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    throw std::logic_error("Not implemented");
}

void SpearEnemy::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::damage, false, false);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    audio_manager->play_se(SE_INDEX::STAN);

    mWaitTimer = mStunTime;
}

void SpearEnemy::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer -= elapsedTime_;
    if (mWaitTimer <= 0.0f)
    {
        fChangeState(DivedState::Idle);
        mIsStun = false;
        mStunEffect->stop(effect_manager->get_effekseer_manager());
    }
}

void SpearEnemy::fDieInit()
{
    throw std::logic_error("Not implemented");
}

void SpearEnemy::fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    throw std::logic_error("Not implemented");
}

void SpearEnemy::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsAttack = false;
        fSetAttackOperation(false);
        mIsStun = Arg_;
        fChangeState(DivedState::Stun);
        //-----スタンになったことを送信する-----//
        SendEnemyConditionData(EnemySendData::EnemyConditionEnum::Stun);

    }
}

void SpearEnemy::AiTransitionIdle()
{
    fChangeState(DivedState::Idle);
}

void SpearEnemy::AiTransitionMove()
{
    fChangeState(DivedState::Move);
}

void SpearEnemy::AiTransformAttack()
{
    fChangeState(DivedState::ThrustBegin);
}
