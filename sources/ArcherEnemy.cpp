#include"ArcherEnemy.h"
#include"EnemyFileSystem.h"
#include"imgui_include.h"
#include "NormalEnemy.h"
#include "StraightBullet.h"
#include "Operators.h"
#include "BulletManager.h"
//****************************************************************
//
// 盾なし通常攻撃の雑魚敵の派生クラス
//
//****************************************************************
ArcherEnemy::ArcherEnemy(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    EnemyParamPack ParamPack_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_arrow.fbx",
        ParamPack_,
        EmitterPoint_)
{

    // 位置を初期化
    mPosition = EmitterPoint_;
    mOrientation = { 0.0f,0.0f,0.0f,1.0f };
    mScale = { 0.05f,0.05f,0.05f };
    //パラメーターの初期化
    fParamInitialize();
    fRegisterFunctions();

    mfAddFunc = BulletManager::Instance().fGetAddFunction();
}


ArcherEnemy::ArcherEnemy(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_arrow.fbx")
{}


void ArcherEnemy::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    //--------------------<更新処理>--------------------//
    elapsedTime_ = fBaseUpdate(elapsedTime_, Graphics_);
   
}



void ArcherEnemy::fUpdateAttackCapsule()
{
}


void ArcherEnemy::fRegisterFunctions()
{
    //登場状態の登録
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
        //待機状態の登録
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

    //移動状態の登録
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
    //接近移動状態の登録
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
    //後退移動状態の登録
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
    //攻撃準備状態の登録
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
    //攻撃待機の登録
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
    //攻撃状態の登録
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
    //被ダメ状態の登録
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
    //スタン状態の登録
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
    //初期化
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
    // 汎用タイマーを初期化
    mStayTimer = 0.0f;
}

void ArcherEnemy::fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mStayTimer += elapsedTime_;
    mDissolve -= elapsedTime_;

    // 一定時間経過で移動に遷移
    if (mStayTimer >= SPAWN_STAY_TIME)
    {
        fChangeState(DivedState::Idle);
    }
}

void ArcherEnemy::fIdleInit()
{
    //mpSkinnedMesh->play_animation(IDLE, true, 0.1f);
    mStayTimer = 0;
}

void ArcherEnemy::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mStayTimer += elapsedTime_;
    if (mStayTimer > IDLE_STAY_TIME) return;
    mStayTimer = 0.0f;
    fChangeState(DivedState::Move);
}

void ArcherEnemy::fMoveInit()
{
  
     mpModel->play_animation(mAnimPara, AnimationName::walk, true);
    mAttackingTime = 0.0f;
}

void ArcherEnemy::fmoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    //プレイヤーの方向に回転
    fTurnToPlayer(elapsedTime_, ROT_SPEED);
    //向いている方向に全身
    fMove(elapsedTime_);

    const float LengthFromPlayer = Math::calc_vector_AtoB_length(mPosition, mPlayerPosition);
    //プレイヤーとの距離が攻撃可能距離以下なら離れていく行動をとる
    if (LengthFromPlayer < AT_SHORTEST_DISTANCE)
    {
        fChangeState(DivedState::Leave);
        return;
    }

    //プレイヤーとの距離が攻撃可能距離以上なら近づく行動をとる
    if (LengthFromPlayer > AT_LONGEST_DISTANCE )
    {
        fChangeState(DivedState::Approach);
    }

}

void ArcherEnemy::fMoveApproachInit()
{
    mStayTimer = 0.0f;
}

void ArcherEnemy::fMoveApproachUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fMove(elapsedTime_);
    fTurnToPlayer(elapsedTime_, ROT_SPEED);
    const float LengthFromPlayer = Math::calc_vector_AtoB_length(mPosition, mPlayerPosition);

    if (LengthFromPlayer > AT_SHORTEST_DISTANCE && LengthFromPlayer < AT_LONGEST_DISTANCE)
    {
        fChangeState(DivedState::AttackReady);
        return;
    }

    if (LengthFromPlayer < AT_SHORTEST_DISTANCE)
    {
        fChangeState(DivedState::Leave);
        return;
    }

    //一定時間移動したら待機状態に遷移
    mStayTimer += elapsedTime_;
    if (mStayTimer >= MOVE_TIME)
    {
        fChangeState(DivedState::Idle);
        mStayTimer = 0;
    }
}

void ArcherEnemy::fMoveLeaveInit()
{
    mStayTimer = 0.0f;
}


void ArcherEnemy::fMoveLeaveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
  //プレイヤーから遠ざかるためにプレイヤーから反対方向へ向く処理
    auto turn_opposite_player = [&](float elapsed_time, float rot_speed)
    {
        // プレイヤーの反対方向に回転
        constexpr DirectX::XMFLOAT3 up = { 0.001f,1.0f,0.0f };

        // プレイヤーとの逆ベクトル
        const DirectX::XMFLOAT3 vToPlayer = Math::Normalize(mPosition - mPlayerPosition);
        // 自分の正面ベクトル
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
        fChangeState(DivedState::AttackReady);
        return;
    }

    if (LengthFromPlayer > AT_LONGEST_DISTANCE)
    {
        fChangeState(DivedState::Approach);
        return;
    }

    //一定時間移動したら待機状態に遷移
    mStayTimer += elapsedTime_;
    if (mStayTimer >= MOVE_TIME)
    {
        fChangeState(DivedState::Idle);
        mStayTimer = 0;
    }
}

void ArcherEnemy::fAttackBeginInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_ready);
    audio_manager->play_se(SE_INDEX::ENEMY_EMERGENCE);

    mStayTimer = 0.0f;
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
    //弾装填
        float bullet_speed = 1.0f * 0.2f;
        auto straightBullet = new StraightBullet(Graphics_,
            tar_pos, Math::GetFront(mOrientation) * bullet_speed);
        //パラメータ設定
        straightBullet->fSetBulletData(0.5f, 0.5f, 0.5f, 1, 1.0f);
        //登録
        mfAddFunc(straightBullet);

        //待機時間設定
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
    //ターゲットに向かって回転
    DirectX::XMVECTOR orientation_vec = DirectX::XMLoadFloat4(&mOrientation);
    DirectX::XMVECTOR forward;
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientation_vec);
    DirectX::XMFLOAT4X4 m4x4 = {};
    DirectX::XMStoreFloat4x4(&m4x4, m);
    forward = { m4x4._31, m4x4._32, m4x4._33 };
    forward = DirectX::XMVector3Normalize(forward);
    DirectX::XMFLOAT3 f;
    DirectX::XMStoreFloat3(&f, forward);


    mPosition += f * MAX_MOVE_SPEED * elapsed_time;

}
