#include"ArcherEnemy.h"
#include"EnemyFileSystem.h"
#include"imgui_include.h"
#include "NormalEnemy.h"
#include "StraightBullet.h"
#include "Operators.h"
#include "BulletManager.h"
//****************************************************************
//
// ���Ȃ��ʏ�U���̎G���G�̔h���N���X
//
//****************************************************************
ArcherEnemy::ArcherEnemy(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    EnemyParamPack ParamPack_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_arrow.fbx",
        ParamPack_,
        EmitterPoint_)
{

    // �ʒu��������
    //mPosition = EmitterPoint_;
    mOrientation = { 0.0f,0.0f,0.0f,1.0f };
    mScale = { 0.05f,0.05f,0.05f };
    //�p�����[�^�[�̏�����
    fParamInitialize();
    fRegisterFunctions();

    mfAddFunc = BulletManager::Instance().fGetAddFunction();

}


ArcherEnemy::ArcherEnemy(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_arrow.fbx")
{}


void ArcherEnemy::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    //--------------------<�X�V����>--------------------//
    elapsedTime_ = fBaseUpdate(elapsedTime_, Graphics_);
#ifdef USE_IMGUI
    std::string name = "ArcherEnemy" + std::to_string(object_id);
    imgui_menu_bar("Enemy", name, display_scape_imgui);
    if (display_scape_imgui)
    {
        ImGui::Begin(name.c_str());
        ImGui::RadioButton("Master", master);
        ImGui::End();
    }
#endif
}



void ArcherEnemy::fUpdateAttackCapsule()
{
}

void ArcherEnemy::fSetEnemyState(int state)
{
    //-----����AI�Ɠ����Ȃ珈�������Ȃ�-----//
    if (ai_state == state) return;

    //-----���ꂼ��̃X�e�[�g�ɑJ��-----//
    switch (ai_state)
    {
    case AiState::Start: fChangeState(DivedState::Start); break;
    case AiState::Idle: fChangeState(DivedState::Idle); break;
    case AiState::Move: fChangeState(DivedState::Move); break;
    case AiState::Damaged: fChangeState(DivedState::Damaged); break;
    case AiState::AttackReady: fChangeState(DivedState::AttackReady); break;
    case AiState::AttackIdle: fChangeState(DivedState::AttackIdle); break;
    case AiState::AttackShot: fChangeState(DivedState::AttackShot); break;
    case AiState::Approach: fChangeState(DivedState::Approach); break;
    case AiState::Leave: fChangeState(DivedState::Leave); break;
    case AiState::Die: fChangeState(DivedState::Die); break;
    case AiState::Stun: fChangeState(DivedState::Stun); break;
    default:
        break;
    }

}


void ArcherEnemy::fRegisterFunctions()
{
    //�o���Ԃ̓o�^
    InitFunc Ini = [=]()->void
    {
        fSpawnInit();
    };
    UpdateFunc Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
    {
        fSpawnUpdate(elapsedTime_, Graphics_);
    };
    FunctionTuple tuple = std::make_tuple(Ini, Up);
    mFunctionMap.insert(std::make_pair(DivedState::Start, tuple));

    {
        //�ҋ@��Ԃ̓o�^
        Ini = [=]()->void
        {
            fIdleInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fIdleUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::Idle, tuple));
    }

    //�ړ���Ԃ̓o�^
    {
        Ini = [=]()->void
        {
            fMoveInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fmoveUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::Move, tuple));
    }
    //�ڋ߈ړ���Ԃ̓o�^
    {
        Ini = [=]()->void
        {
            fMoveApproachInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fMoveApproachUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::Approach, tuple));
    }
    //��ވړ���Ԃ̓o�^
    {
        Ini = [=]()->void
        {
            fMoveLeaveInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fMoveLeaveUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::Leave, tuple));
    }
    //�U��������Ԃ̓o�^
    {
        Ini = [=]()->void
        {
            fAttackBeginInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackBeginUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::AttackReady, tuple));
    }
    //�U���ҋ@�̓o�^
    {
        Ini = [=]()->void
        {
            fAttackPreActionInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackPreActionUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::AttackIdle, tuple));
    }
    //�U����Ԃ̓o�^
    {
        Ini = [=]()->void
        {
            fAttackEndInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fAttackEndUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::AttackShot, tuple));
    }
    //��_����Ԃ̓o�^
    {
        Ini = [=]()->void
        {
            fDamagedInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDamagedUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::Damaged, tuple));
    }
    //�X�^����Ԃ̓o�^
    {
        Ini = [=]()->void
        {
            fStunInit();
        };
        Up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStunUpdate(elapsedTime_, Graphics_);
        };
        tuple = std::make_tuple(Ini, Up);
        mFunctionMap.insert(std::make_pair(DivedState::Stun, tuple));
    }
    //������
    fChangeState(DivedState::Start);

}

void ArcherEnemy::fParamInitialize()
{
	mStayTimer = 0.0f;
    mAttack_flg = false;
}

void ArcherEnemy::fSpawnInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    // �ėp�^�C�}�[��������
    mStayTimer = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = MasterAiState::Idle;
}

void ArcherEnemy::fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mStayTimer += elapsedTime_;
    mDissolve -= elapsedTime_;

    // ��莞�Ԍo�߂ňړ��ɑJ��
    if (mStayTimer >= SPAWN_STAY_TIME)
    {
        //-----����̏o���̎��̂݃G�t�F�N�g�Đ�-----//
        if (is_appears == false)mVernierEffect->play(effect_manager->get_effekseer_manager(), mPosition);

        //-----�o���t���O��true�ɂ���-----//
        is_appears = true;
        fChangeState(DivedState::Idle);
    }
}

void ArcherEnemy::fIdleInit()
{
    //mpSkinnedMesh->play_animation(IDLE, true, 0.1f);
    mStayTimer = 0;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = MasterAiState::Idle;
}

void ArcherEnemy::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mStayTimer += elapsedTime_;
    if (mStayTimer > IDLE_STAY_TIME) return;
    mStayTimer = 0.0f;
    //-----�}�X�^�[�ȊO�̓}�X�^�[����̎w�����܂�-----//
    if (master)fChangeState(DivedState::Move);
}

void ArcherEnemy::fMoveInit()
{

     mpModel->play_animation(mAnimPara, AnimationName::walk, true);
    mAttackingTime = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = MasterAiState::Move;

    //-----��芪���̈ړ��ʒu�����߂�-----//
    SetMasterSurroundingsPos();

}

void ArcherEnemy::fmoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    //�v���C���[�̕����ɉ�]
    fTurnToPlayer(elapsedTime_, ROT_SPEED);
    //�����Ă�������ɑS�g
    fMove(elapsedTime_);

    const float LengthFromPlayer = Math::calc_vector_AtoB_length(mPosition, mPlayerPosition);
    //�v���C���[�Ƃ̋������U���\�����ȉ��Ȃ痣��Ă����s�����Ƃ�
    if (LengthFromPlayer < AT_SHORTEST_DISTANCE)
    {
        fChangeState(DivedState::Leave);
        return;
    }

    //�v���C���[�Ƃ̋������U���\�����ȏ�Ȃ�߂Â��s�����Ƃ�
    if (LengthFromPlayer > AT_LONGEST_DISTANCE )
    {
        fChangeState(DivedState::Approach);
    }

}

void ArcherEnemy::fMoveApproachInit()
{
    //-----��芪���̈ړ��ʒu�����߂�-----//
    SetMasterSurroundingsPos();

    mStayTimer = 0.0f;
}

void ArcherEnemy::fMoveApproachUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fMove(elapsedTime_);
    //--------------------<�v���C���[�̕����ɉ�]>--------------------//
    if (master)fTurnToPlayer(elapsedTime_, 20.0f);
    else
    {
        //-----�^�[�Q�b�g�ʒu�Ƃ̋������m�F-----//
        CheckFollowersTargetPos();
        fTurnToTarget(elapsedTime_, 20.0, followers_target_pos);
    }
    const float LengthFromPlayer = Math::calc_vector_AtoB_length(mPosition, mPlayerPosition);

    if (LengthFromPlayer > AT_SHORTEST_DISTANCE && LengthFromPlayer < AT_LONGEST_DISTANCE)
    {
        //-----�}�X�^�[�ȊO�̓}�X�^�[����̎w�����܂�-----//
        if (master)fChangeState(DivedState::AttackReady);
        return;
    }

    if (LengthFromPlayer < AT_SHORTEST_DISTANCE)
    {
        fChangeState(DivedState::Leave);
        return;
    }

    //��莞�Ԉړ�������ҋ@��ԂɑJ��
    mStayTimer += elapsedTime_;
    if (mStayTimer >= MOVE_TIME)
    {
        fChangeState(DivedState::Idle);
        mStayTimer = 0;
    }
}

void ArcherEnemy::fMoveLeaveInit()
{
    //-----��芪���̈ړ��ʒu�����߂�-----//
    SetMasterSurroundingsPos();
    mStayTimer = 0.0f;
}


void ArcherEnemy::fMoveLeaveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
  //�v���C���[���牓�����邽�߂Ƀv���C���[���甽�Ε����֌�������
    auto turn_opposite_player = [&](float elapsed_time, float rot_speed)
    {
        // �v���C���[�̔��Ε����ɉ�]
        constexpr DirectX::XMFLOAT3 up = { 0.001f,1.0f,0.0f };

        // �v���C���[�Ƃ̋t�x�N�g��
        const DirectX::XMFLOAT3 vToPlayer = Math::Normalize(mPosition - mPlayerPosition);
        // �����̐��ʃx�N�g��
        const auto front = Math::Normalize(Math::GetFront(mOrientation));
        float dot = Math::Dot(vToPlayer, front);

        dot = acosf(dot);

        if (fabs(dot) > DirectX::XMConvertToRadians(10.0f))
        {
            float cross{ (vToPlayer.x * front.z) - (vToPlayer.z * front.x) };
            if (cross > 0)
            {
                mOrientation = Math::RotQuaternion(mOrientation, up, dot * rot_speed * elapsed_time);
            }
            else
            {
                mOrientation = Math::RotQuaternion(mOrientation, up, -dot * rot_speed * elapsed_time);
            }
        }
    };
    turn_opposite_player(elapsedTime_, ROT_SPEED);
    fMove(elapsedTime_);

    const float LengthFromPlayer = Math::calc_vector_AtoB_length(mPosition, mPlayerPosition);
    if (LengthFromPlayer > AT_SHORTEST_DISTANCE && LengthFromPlayer < AT_LONGEST_DISTANCE)
    {
        //-----�}�X�^�[�ȊO�̓}�X�^�[����̎w�����܂�-----//
        if (master)fChangeState(DivedState::AttackReady);
        return;
    }

    if (LengthFromPlayer > AT_LONGEST_DISTANCE)
    {
        fChangeState(DivedState::Approach);
        return;
    }

    //��莞�Ԉړ�������ҋ@��ԂɑJ��
    mStayTimer += elapsedTime_;
    if (mStayTimer >= MOVE_TIME)
    {
        //-----�}�X�^�[�ȊO�̓}�X�^�[����̎w�����܂�-----//
        if (master)fChangeState(DivedState::Idle);
        mStayTimer = 0;
    }
}

void ArcherEnemy::fAttackBeginInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_ready);
    audio_manager->play_se(SE_INDEX::ENEMY_EMERGENCE);

    mStayTimer = 0.0f;

    //-----�X�e�[�g�ݒ�-----//
    ai_state = MasterAiState::Attack;
}

void ArcherEnemy::fAttackBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, ROT_SPEED);
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivedState::AttackIdle);
    }
}

void ArcherEnemy::fAttackPreActionInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_idle);
    mStayTimer = 0.0f;

}

void ArcherEnemy::fAttackPreActionUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, ROT_SPEED);
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivedState::AttackShot);
    }
}

void ArcherEnemy::fAttackEndInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::attack_shot);
    mAttackingTime = 0.0f;
}

void ArcherEnemy::fAttackEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    DirectX::XMFLOAT3 tar_pos = { mPosition.x, mPosition.y + 1.5f, mPosition.z };
    fTurnToPlayer(elapsedTime_, ROT_SPEED);
    //�e���U
        float bullet_speed = 1.0f * 0.2f;
        auto straightBullet = new StraightBullet(Graphics_,
            tar_pos, Math::GetFront(mOrientation) * bullet_speed);
        //�p�����[�^�ݒ�
        straightBullet->fSetBulletData(0.5f, 0.5f, 0.5f, 1, 1.0f);
        //�o�^
        mfAddFunc(straightBullet);

        //�ҋ@���Ԑݒ�
        mStayTimer = 3.0f;
        fChangeState(DivedState::Idle);

}

void ArcherEnemy::fDamagedInit()
{
}


void ArcherEnemy::fDamagedUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    using namespace DirectX;
    XMVECTOR P_Pos = XMLoadFloat3(&mPlayerPosition);
    XMVECTOR E_Pos = XMLoadFloat3(&mPosition);
    XMVECTOR Vec = E_Pos - P_Pos;
    Vec = XMVector3Normalize(Vec);
    XMFLOAT3 v;
    XMStoreFloat3(&v, Vec);


    fChangeState(DivedState::Idle);
}

void ArcherEnemy::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::damage, false);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    audio_manager->play_se(SE_INDEX::STAN);

    mStayTimer = 0;
}

void ArcherEnemy::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mStayTimer += elapsedTime_;
    if (mStayTimer >= mStunTime)
    {
        fChangeState(DivedState::Idle);
        mStunEffect->stop(effect_manager->get_effekseer_manager());
        mIsStun = false;
    }
}

void ArcherEnemy::fSetStun(bool Arg_,bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivedState::Stun);
    }
}

void ArcherEnemy::AiTransitionIdle()
{
    fChangeState(DivedState::Idle);
}

void ArcherEnemy::AiTransitionMove()
{
    fChangeState(DivedState::Move);
}

void ArcherEnemy::AiTransformAttack()
{
    fChangeState(DivedState::AttackReady);
}

void ArcherEnemy::fGuiMenu()
{
#ifdef USE_IMGUI
    ImGui::Text("Name : arch");
    ImGui::DragFloat3("position", &mPosition.x);
    ImGui::DragFloat3("angle", &mOrientation.x);
    const char* state_list[] = { "IDLE","MOVE","ATTACK","DAUNTED" };
    /* std::string state =  state_list[static_cast<int>(mNowState)];
     ImGui::Text("State"); ImGui::SameLine();
     ImGui::Text(state.c_str());*/
    ImGui::Checkbox("Attack", &mAttack_flg);
    if (ImGui::Button("dameged", { 70.0f,30.0f }))
    {
       // fDamaged(1, 0.6f);
    }
#endif
}

void ArcherEnemy::fMove(float elapsed_time)
{

    //--------------------<�v���C���[�̂�������Ɉړ�>--------------------//
    fMoveFront(elapsed_time, MAX_MOVE_SPEED);
}
