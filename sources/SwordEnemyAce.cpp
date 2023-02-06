#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"SwordEnemyAce.h"
#include"Operators.h"
SwordEnemy_Ace::SwordEnemy_Ace(GraphicsPipeline& Graphics_, const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_ace_sword.fbx",ParamPack_,EmitterPoint_)
{
    SwordEnemy_Ace::fRegisterFunctions();
    // �{�[����������
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
#ifdef USE_IMGUI
    std::string name = "SwordEnemy_Ace" + std::to_string(object_id);
    imgui_menu_bar("Enemy", name, display_scape_imgui);
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

void SwordEnemy_Ace::fUpdateAttackCapsule()
{
    throw std::logic_error("Not implemented");
}

void SwordEnemy_Ace::fSetEnemyState(int state)
{
    //-----����AI�Ɠ����Ȃ珈�������Ȃ�-----//
    if (ai_state == state) return;
    //-----�X�^�����Ă�Ȃ珈�������Ȃ�-----//
    if (mIsStun) return;

    switch (state)
    {
    case AiState::Start: fChangeState(DivideState::Start); break;
    case AiState::Idle: fChangeState(DivideState::Idle); break;
    case AiState::CounterStart: fChangeState(DivideState::CounterStart); break;
    case AiState::CounterMiddle: fChangeState(DivideState::CounterMiddle); break;
    case AiState::CounterEnd: fChangeState(DivideState::CounterEnd); break;
    case AiState::Move: fChangeState(DivideState::Move); break;
    case AiState::Stun: fChangeState(DivideState::Stun); break;
    default:
        break;
    }

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
    ai_state = MasterAiState::Idle;
}

void SwordEnemy_Ace::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if(mDissolve<=0.0f)
    {
        //-----�o���t���O��true�ɂ���-----//
        is_appears = true;
        fChangeState(DivideState::Idle);
    }
}

void SwordEnemy_Ace::fIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    mWaitTimer = 0.0f;
    ai_state = MasterAiState::Idle;
}

void SwordEnemy_Ace::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    if(mWaitTimer>=3.0f)
    {
        //-----�}�X�^�[�ȊO�̓}�X�^�[����̎w�����܂�-----//
        if (master)fChangeState(DivideState::Move);
    }
}

void SwordEnemy_Ace::fCounterStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ace_attack_ready);
    mWaitTimer = 0.0f;
    ai_state = MasterAiState::Attack;

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

    // �_���[�W���󂯂���
    if(mIsHit)
    {
        fChangeState(DivideState::CounterAttack);
        mIsWaitCounter = false;
    }

}

void SwordEnemy_Ace::fCounterAttackInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::ace_attack, false, true, 0.3f, 2.0f);
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
    ai_state = MasterAiState::Move;
    //-----��芪���̈ړ��ʒu�����߂�-----//
    SetMasterSurroundingsPos();

}

void SwordEnemy_Ace::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    //--------------------<�v���C���[�̂�������Ɉړ�>--------------------//
    fMoveFront(elapsedTime_, mMoveSpeed);

    //--------------------<�v���C���[�̕����ɉ�]>--------------------//
    if (master)fTurnToPlayer(elapsedTime_, 20.0f);
    else
    {
        //-----�^�[�Q�b�g�ʒu�Ƃ̋������m�F-----//
        CheckFollowersTargetPos();
        fTurnToTarget(elapsedTime_, 20.0, followers_target_pos);
    }

    if(fGetLengthFromPlayer()<=10.0f)
    {
        fChangeState(DivideState::CounterStart);
    }
}

void SwordEnemy_Ace::fStunInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::stun, true);
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
        //-----�X�^���ɂȂ������Ƃ𑗐M����-----//
        SendEnemyConditionData(EnemySendData::EnemyConditionEnum::Stun);

    }
}

void SwordEnemy_Ace::AiTransitionIdle()
{
    fChangeState(DivideState::Idle);
}

void SwordEnemy_Ace::AiTransitionMove()
{
    fChangeState(DivideState::Move);
}

void SwordEnemy_Ace::AiTransformAttack()
{
    fChangeState(DivideState::CounterStart);
}

