#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"SwordEnemy.h"
#include"Operators.h"
SwordEnemy::SwordEnemy(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_)
        :BaseEnemy(Graphics_,
                  "./resources/Models/Enemy/enemy_sword.fbx",
                  ParamPack_,
                  EmitterPoint_)
{
    fRegisterFunctions();
    // ボーンを初期化
    mSwordBone = mpModel->get_bone_by_name("hand_r_joint");
    mScale = { 0.05f,0.05f,0.05f };
}
SwordEnemy::SwordEnemy(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_sword.fbx")
{}

void SwordEnemy::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_=fBaseUpdate(elapsedTime_,Graphics_);
    fUpdateAttackCapsule(); // 攻撃用のカプセル位置を更新
#ifdef USE_IMGUI
    std::string name = "SwordEnemy" + std::to_string(object_id);
    imgui_menu_bar("Enemy", name, display_scape_imgui);
    static bool test = false;
    if (display_scape_imgui)
    {
        ImGui::Begin(name.c_str());
        ImGui::RadioButton("Master", master);
        if (master)
        {
            ImGui::DragFloat3("master_pos", &mPosition.x, 0.0f);
            ImGui::DragInt("master_ai_state", &ai_state, 0.0f);
            ImGui::DragInt("master_target_id", &target_player_id, 0.0f);
        }
        else
        {
            ImGui::DragFloat3("master_pos", &master_pos.x, 0.0f);
            ImGui::DragInt("master_ai_state", &master_ai_state, 0.0f);
            ImGui::DragInt("master_target_id", &master_target_id, 0.0f);
        }

        if (ImGui::TreeNode("EnemyParam"))
        {
            ImGui::Text("MaxHp :%d", mMaxHp);
            ImGui::Text("AttackPower :%d", mAttackPower);
            ImGui::Text("InvincibleTime :%f", mAttackInvTime);
            ImGui::Text("BodyCapsuleRad :%f", mBodyCapsule.mRadius);
            ImGui::Text("AttackCapsuleRad :%f", mAttackCapsule.mRadius);
            ImGui::Text("StunTime :%f", mStunTime);
            ImGui::TreePop();
        }

        ImGui::End();
    }
#endif
}

void SwordEnemy::fRegisterFunctions()
{
    // 関数をステートマシンに登録
    {
        InitFunc ini = [=]()->void
        {
            fSpawnInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fSpawnUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::Start, tuple));
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
        mFunctionMap.insert(std::make_pair(DivedState::Idle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fWalkInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fWalkUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::Move, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fAttackBeginInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackBeginUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::AttackBegin, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fAttackPreActionInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackPreActionUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::AttackMiddle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fAttackEndInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackEndUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::AttackEnd, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fAttackRunInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackRunUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::AttackRun, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fEscapeInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fEscapeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::Escape, tuple));
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

void SwordEnemy::fUpdateAttackCapsule()
{
    //--------------------<剣のカプセルの位置を決定する>--------------------//
    DirectX::XMFLOAT3 position{};
    DirectX::XMFLOAT3 up{};
    // ボーンの名前から位置と上ベクトルを取得
    mpModel->fech_by_bone(mAnimPara,
        Math::calc_world_matrix(mScale, mOrientation, mPosition),
        mSwordBone, position, up);
    up = Math::Normalize(up);

    mAttackCapsule.mTop = position + up * 5.0f;
    mAttackCapsule.mBottom = position + up * 1.0f;
}

void SwordEnemy::fSetEnemyState(int state)
{
    //-----今のAIと同じなら処理をしない-----//
    if (ai_state == state) return;
    //-----スタンしてるなら処理をしない-----//
    if (mIsStun) return;

    switch (state)
    {
    case AiState::Start: fChangeState(DivedState::Start); break;
    case AiState::Idle: fChangeState(DivedState::Idle); break;
    case AiState::Move: fChangeState(DivedState::Move); break;
    case AiState::Damaged: fChangeState(DivedState::Damaged); break;
    case AiState::AttackBegin: fChangeState(DivedState::AttackBegin); break;
    case AiState::AttackRun: fChangeState(DivedState::AttackRun); break;
    case AiState::AttackMiddle: fChangeState(DivedState::AttackMiddle); break;
    case AiState::AttackEnd: fChangeState(DivedState::AttackEnd); break;
    case AiState::Die: fChangeState(DivedState::Die); break;
    case AiState::Escape: fChangeState(DivedState::Escape); break;
    case AiState::Stun: fChangeState(DivedState::Stun); break;
    default:
        break;
    }

}


void SwordEnemy::fSpawnInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::idle, true);
    // 汎用タイマーを初期化
    mWaitTimer = 0.0f;
    //-----ステート設定-----//
    ai_state = MasterAiState::Idle;
}

void SwordEnemy::fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    mDissolve -= elapsedTime_;
    // 一定時間経過で移動に遷移
    if(mWaitTimer>=mSpawnDelaySec)
    {
        //-----初回の出現の時のみエフェクト再生-----//
        if(is_appears == false)mVernierEffect->play(effect_manager->get_effekseer_manager(), mPosition);

        //-----出現フラグをtrueにする-----//
        is_appears = true;
        fChangeState(DivedState::Move);
    }
}

void SwordEnemy::fIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    // 汎用タイマーを初期化
    mWaitTimer = 0.0f;
    //-----ステート設定-----//
    ai_state = MasterAiState::Idle;

}

void SwordEnemy::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    // 一定時間経過で移動に遷移
    if (mWaitTimer >= mSpawnDelaySec)
    {
        //-----マスター以外はマスターからの指示をまつ-----//
        if (master)fChangeState(DivedState::Move);
    }
}

void SwordEnemy::fWalkInit()
{
    SetMasterSurroundingsPos();

    // アニメーションを再生
    mpModel->play_animation(mAnimPara,AnimationName::walk,true);
    mWaitTimer = 0.0f;
    //-----ステート設定-----//
    ai_state = MasterAiState::Move;
}

void SwordEnemy::fWalkUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    //--------------------<プレイヤーのいる向きに移動>--------------------//
    fMoveFront(elapsedTime_, mMoveSpeed);

    //--------------------<プレイヤーの方向に回転>--------------------//
    if (master)fTurnToPlayer(elapsedTime_, 20.0f);
    else
    {
        //-----ターゲット位置との距離を確認-----//
        CheckFollowersTargetPos();
        fTurnToTarget(elapsedTime_, 20.0, followers_target_pos);
    }

    // プレイヤーとの距離が一定以下になったら
    if(mAttackRange >= Math::Length(mPlayerPosition-mPosition))
    {
        //-----マスター以外はマスターからの指示をまつ-----//
        if (master)fChangeState(DivedState::AttackBegin);
        //-----あまりにも距離が近い場合は攻撃に遷移する-----//
        else if (mAttackRange / 2 >= Math::Length(mPlayerPosition - mPosition))fChangeState(DivedState::AttackBegin);
    }
}

//--------------------<剣を振り上げる>--------------------//
void SwordEnemy::fAttackBeginInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::attack_idle);
    mWaitTimer = 0.0f;
    //-----ステート設定-----//
    ai_state = MasterAiState::Attack;
}
void SwordEnemy::fAttackBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_,2.0f);

    // タイマーを加算
    mWaitTimer += elapsedTime_;
    if(mWaitTimer>=mAttackBeginTimeSec*mAnimationSpeed)
    {
        fChangeState(DivedState::AttackRun);
    }
}

void SwordEnemy::fAttackRunInit()
{
    //-----攻撃動作に入ったことを知らせる-----//
    fSetAttackOperation(true);
    mMoveTimer = 0;
    audio_manager->play_se(SE_INDEX::ENEMY_EMERGENCE);

}

void SwordEnemy::fAttackRunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mMoveTimer += elapsedTime_;
    // プレイヤーとの距離が近くなるまで走って近づく
    fTurnToPlayer(elapsedTime_, 3.0f);
    fMoveFront(elapsedTime_, 60.0f);
    // プレイヤーとの距離が近くなるまたは制限時間になったら
    if ((mAttackRange * 0.1f > Math::Length(mPlayerPosition - mPosition)) || mMoveTimer > mMoveTimeLimit)
    {
        fChangeState(DivedState::AttackMiddle);
    }
}

//--------------------<剣を振り下ろす予備動作>--------------------//
void SwordEnemy::fAttackPreActionInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_up, false, false);
    mWaitTimer = 0.0f;

}
void SwordEnemy::fAttackPreActionUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    if (mWaitTimer >= mAttackPreActionTimeSec * mAnimationSpeed)
    {
        fChangeState(DivedState::AttackEnd);
    }
}
//--------------------<攻撃中（当たり判定ON）>--------------------//
void SwordEnemy::fAttackEndInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_down, false, false);
    mWaitTimer = 0.0f;
    fSetAttack(true);
    fSetAttackOperation(false);

}
void SwordEnemy::fAttackEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    if(mWaitTimer>=mAttackDownSec)
    {
        fChangeState(DivedState::Escape);
        fSetAttack(false);
        fSetAttackOperation(false);
    }
}

void SwordEnemy::fEscapeInit()
{
    mMoveTimer = 0;
}

void SwordEnemy::fEscapeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mMoveTimer += elapsedTime_;
    // プレイヤーと逆に進
    DirectX::XMFLOAT3 vec = { mPosition - mPlayerPosition };
    vec.y = 0.0f;
    mPosition += Math::Normalize(vec) * elapsedTime_ * 30.0f;
    // プレイヤーと距離を取るまたは制限時間になったら
    if (Math::Length(vec) >= 60.0f || mMoveTimer > mMoveTimeLimit)
    {
        fChangeState(DivedState::Idle);
    }
}

void SwordEnemy::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::damage, false, false);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    audio_manager->play_se(SE_INDEX::STAN);

    mWaitTimer = mStunTime ;
    mIsAttack = false;
    //-----攻撃動作に入ったことを知らせる-----//
    fSetAttackOperation(false);
}

void SwordEnemy::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer -= elapsedTime_;
    if (mWaitTimer <=0.0f)
    {
        mStunEffect->stop(effect_manager->get_effekseer_manager());
        mIsStun = false;
        fChangeState(DivedState::Idle);
    }

}


void SwordEnemy::fDieInit()
{
    throw std::logic_error("Not implemented");
}

void SwordEnemy::fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    throw std::logic_error("Not implemented");
}

void SwordEnemy::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivedState::Stun);
        //-----スタンになったことを送信する-----//
        SendEnemyConditionData(EnemySendData::EnemyConditionEnum::Stun);
    }
}

void SwordEnemy::AiTransitionIdle()
{
    fChangeState(DivedState::Idle);
}

void SwordEnemy::AiTransitionMove()
{
    fChangeState(DivedState::Move);
}

void SwordEnemy::AiTransformAttack()
{
    fChangeState(DivedState::AttackBegin);
}

