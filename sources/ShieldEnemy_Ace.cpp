#include "ShieldEnemy_Ace.h"
#include "Operators.h"


ShieldEnemy_Ace::ShieldEnemy_Ace(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_)
    :BaseEnemy(Graphics_,
        "./resources/Models/Enemy/enemy_ace_shield.fbx",
        ParamPack_,
        EmitterPoint_)
{
    ShieldEnemy_Ace::fRegisterFunctions();
    // ボーンを初期化
    mScale = { 0.05f,0.05f,0.05f };
    is_shield = false;
    mVernierEffect->stop(effect_manager->get_effekseer_manager());
    mShieldEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), mkShieldPath);


}

ShieldEnemy_Ace::ShieldEnemy_Ace(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_ace_shield.fbx")
{}

void ShieldEnemy_Ace::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_ = fBaseUpdate(elapsedTime_, Graphics_);
    fUpdateAttackCapsule(); // 攻撃用のカプセル位置を更新
}

void ShieldEnemy_Ace::fRegisterFunctions()
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
            fShieldReadyInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShieldReadyUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::ShieldReady, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShieldAttackInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShieldAttackUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::ShieldAttack, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShieldInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShieldUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::Shield, tuple));
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
    {
        InitFunc ini = [=]()->void
        {
            fDieInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDieUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivedState::Die, tuple));
    }
    fChangeState(DivedState::Start);
}


void ShieldEnemy_Ace::fUpdateAttackCapsule()
{
}

bool ShieldEnemy_Ace::fDamaged(int Damage_, float InvincibleTime_, GraphicsPipeline& Graphics_, float elapsedTime_)
{
    mIsSuccesGuard = false;
    //シールド効果がON状態且つ、正面から攻撃された場合は攻撃をはじくアニメーションへ遷移
    if (is_shield && fJudge_in_view())
    {
        fChangeState(DivedState::Shield);
        mIsSuccesGuard = true;
        return false;
    }
    bool ret{ false };

    //無敵時間が存在していないときにダメージを食らったら
    if (mInvincibleTime <= 0.0f)
    {
        mCurrentHitPoint -= Damage_;
        mInvincibleTime = InvincibleTime_;

        ret = true;
        //   fChangeState(DivedeState::Damaged);
    }
    //HPがゼロになってしまったら
    if (mCurrentHitPoint <= 0)
    {
        //fChangeState(DivedeState::Die);
        fDie(Graphics_);
    }
    return ret;
}

void ShieldEnemy_Ace::fSpawnInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    // 汎用タイマーを初期化
    mWaitTimer = 0.0f;
}

void ShieldEnemy_Ace::fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    mDissolve -= elapsedTime_;
    // 一定時間経過で移動に遷移
    if (mWaitTimer >= mSpawnDelaySec)
    {
        fChangeState(DivedState::Move);
    }
}

void ShieldEnemy_Ace::fMoveInit()
{
    // アニメーションを再生
    mpModel->play_animation(mAnimPara, AnimationName::move, true);
    mWaitTimer = 0.0f;
}

void ShieldEnemy_Ace::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    //--------------------<プレイヤーのいる向きに移動>--------------------//

    fMoveFront(elapsedTime_, 10.0f);
    //--------------------<プレイヤーの方向に回転>--------------------//
    fTurnToPlayer(elapsedTime_, 2.0f);

    // プレイヤーとの距離が一定以下で自分の視界内にいたら
    if (mDifenceRange >= Math::Length(mPlayerPosition - mPosition) && fJudge_in_view())
    {
        fChangeState(DivedState::ShieldReady);
    }
}

void ShieldEnemy_Ace::fShieldReadyInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::shield_ready);
    mWaitTimer = 0.0f;
    is_shield = true; //シールド効果ON
}

void ShieldEnemy_Ace::fShieldReadyUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    // タイマーを加算
    mWaitTimer += elapsedTime_;
    //シールド構え時間が一定時間たったら
    if (mWaitTimer >= mShieldReadySec * mAnimationSpeed)
    {
        is_shield = false; //シールド効果OFF
        fChangeState(DivedState::Move);
    }

}

void ShieldEnemy_Ace::fShieldAttackInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::shield_Attack);
}

void ShieldEnemy_Ace::fShieldAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivedState::Move);
    }
}

void ShieldEnemy_Ace::fShieldInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::shield);
    mShieldEffect->play(effect_manager->get_effekseer_manager(), mPosition, 5);
    mShieldEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);

}

void ShieldEnemy_Ace::fShieldUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivedState::Move);
    }
}


void ShieldEnemy_Ace::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::stun);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    mWaitTimer = 0.0f;
    is_shield = false;
}

void ShieldEnemy_Ace::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mIsStun = true;
    // タイマーを加算
    mWaitTimer += elapsedTime_;
    //シールド構え時間が一定時間たったら
    if (mWaitTimer >= mStunTime)
    {
        mStunEffect->stop(effect_manager->get_effekseer_manager());

        mIsStun = false;
        fChangeState(DivedState::Move);
    }

}

void ShieldEnemy_Ace::fDieInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::die);
}

void ShieldEnemy_Ace::fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
}
void ShieldEnemy_Ace::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivedState::Stun);
    }
}
//プレイヤーが自分の視界内にいるか判別
bool ShieldEnemy_Ace::fJudge_in_view() const
{
    //プレイヤーとの位置関係を判定し、攻撃されたときに自分の視界45度いないなら正面と判定する
    const DirectX::XMVECTOR EtoP_vec = Math::calc_vector_AtoB_normalize(mPosition, mPlayerPosition);
    const DirectX::XMFLOAT3 forward = Math::GetFront(mOrientation);
    const DirectX::XMVECTOR Forward = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&forward));
    const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EtoP_vec, Forward);
    float dot;
    DirectX::XMStoreFloat(&dot, Dot);
    dot = acosf(dot);
    if (DirectX::XMConvertToDegrees(dot) < mViewingAngle)
    {
        //正面からの攻撃ならtrueを返す
        return true;
    }

    return false;

}