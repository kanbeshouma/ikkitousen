#include "FrontShieldEnemy.h"
#include "Operators.h"


ShieldEnemy::ShieldEnemy(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_)
    :BaseEnemy(Graphics_,
        "./resources/Models/Enemy/enemy_shield.fbx",
        ParamPack_,
        EmitterPoint_)
{
    ShieldEnemy::fRegisterFunctions();
    mShieldEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), mkShieldPath);

    // �{�[����������
    mScale = { 0.05f,0.05f,0.05f };
    is_shield = false;
}

ShieldEnemy::ShieldEnemy(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_shield.fbx")
{}

void ShieldEnemy::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_ = fBaseUpdate(elapsedTime_, Graphics_);
    fUpdateAttackCapsule(); // �U���p�̃J�v�Z���ʒu���X�V
}

void ShieldEnemy::fRegisterFunctions()
{
    // �֐����X�e�[�g�}�V���ɓo�^
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


void ShieldEnemy::fUpdateAttackCapsule()
{
}

bool ShieldEnemy::fDamaged(int Damage_, float InvincibleTime_,GraphicsPipeline& Graphics_,float elapsedTime_)
{
    mIsSuccesGuard = false;
    //�V�[���h���ʂ�ON��Ԋ��A���ʂ���U�����ꂽ�ꍇ�͍U�����͂����A�j���[�V�����֑J��
    if( is_shield  &&fJudge_in_view() )
    {
        fChangeState(DivedState::Shield);
        mIsSuccesGuard = true;
        return false;
    }
    bool ret{ false };

    //���G���Ԃ����݂��Ă��Ȃ��Ƃ��Ƀ_���[�W��H�������
    if (mInvincibleTime <= 0.0f)
    {
        mCurrentHitPoint -= Damage_;
        mInvincibleTime = InvincibleTime_;

        ret = true;
     //   fChangeState(DivedeState::Damaged);
    }
    //HP���[���ɂȂ��Ă��܂�����
    if (mCurrentHitPoint <= 0)
    {
        //fChangeState(DivedeState::Die);
        fDie(Graphics_);
    }
    return ret;
}

void ShieldEnemy::fSpawnInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    // �ėp�^�C�}�[��������
    mWaitTimer = 0.0f;
}

void ShieldEnemy::fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    mDissolve -= elapsedTime_;
    // ��莞�Ԍo�߂ňړ��ɑJ��
    if (mWaitTimer >= mSpawnDelaySec)
    {
        fChangeState(DivedState::Move);
    }
}

void ShieldEnemy::fMoveInit()
{
    // �A�j���[�V�������Đ�
    mpModel->play_animation(mAnimPara, AnimationName::move, true);
    mWaitTimer = 0.0f;
}

void ShieldEnemy::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    //--------------------<�v���C���[�̂�������Ɉړ�>--------------------//

    fMoveFront(elapsedTime_, 10.0f);
    //--------------------<�v���C���[�̕����ɉ�]>--------------------//
    fTurnToPlayer(elapsedTime_, 2.0f);

    // �v���C���[�Ƃ̋��������ȉ��Ŏ����̎��E���ɂ�����
    if (mDifenceRange >= Math::Length(mPlayerPosition - mPosition) && fJudge_in_view())
    {
        fChangeState(DivedState::ShieldReady);
    }
}

void ShieldEnemy::fShieldReadyInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::shield_ready);
    mWaitTimer = 0.0f;
    is_shield = true; //�V�[���h����ON
}

void ShieldEnemy::fShieldReadyUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    // �^�C�}�[�����Z
    mWaitTimer += elapsedTime_;
    //�V�[���h�\�����Ԃ���莞�Ԃ�������
    if (mWaitTimer >= mShieldReadySec * mAnimationSpeed)
    {
        is_shield = false; //�V�[���h����OFF
        fChangeState(DivedState::Move);
    }
   
}

void ShieldEnemy::fShieldAttackInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::shield_Attack);
}

void ShieldEnemy::fShieldAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivedState::Move);
    }
}

void ShieldEnemy::fShieldInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::shield);
    mShieldEffect->play(effect_manager->get_effekseer_manager(), mPosition,5);
    mShieldEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);
}

void ShieldEnemy::fShieldUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if (mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivedState::Move);
    }
}


void ShieldEnemy::fStunInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::stun);
    DirectX::XMFLOAT3 effecPos = { mPosition.x,mPosition.y + 2,mPosition.z };
    mStunEffect->play(effect_manager->get_effekseer_manager(), effecPos);
    audio_manager->play_se(SE_INDEX::STAN);

    mWaitTimer = 0.0f;
    is_shield = false;
}

void ShieldEnemy::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mIsStun = true;
    // �^�C�}�[�����Z
    mWaitTimer += elapsedTime_;
    //�V�[���h�\�����Ԃ���莞�Ԃ�������
    if (mWaitTimer >= mStunTime )
    {
        fChangeState(DivedState::Move);
        mStunEffect->stop(effect_manager->get_effekseer_manager());

        mIsStun = false;
    }

}

void ShieldEnemy::fDieInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::die);
}

void ShieldEnemy::fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
}
void ShieldEnemy::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivedState::Stun);
    }
}
//�v���C���[�������̎��E���ɂ��邩����
bool ShieldEnemy::fJudge_in_view() const
{
    //�v���C���[�Ƃ̈ʒu�֌W�𔻒肵�A�U�����ꂽ�Ƃ��Ɏ����̎��E45�x���Ȃ��Ȃ琳�ʂƔ��肷��
    const DirectX::XMVECTOR EtoP_vec = Math::calc_vector_AtoB_normalize(mPosition, mPlayerPosition);
    const DirectX::XMFLOAT3 forward = Math::GetFront(mOrientation);
    const DirectX::XMVECTOR Forward = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&forward));
    const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EtoP_vec, Forward);
    float dot;
    DirectX::XMStoreFloat(&dot, Dot);
    dot = acosf(dot);
    if (DirectX::XMConvertToDegrees(dot) < mViewingAngle)
    {
        //���ʂ���̍U���Ȃ�true��Ԃ�
        return true;
    }

    return false;

}