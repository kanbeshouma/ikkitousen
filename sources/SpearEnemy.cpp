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
    mVernierEffect->play(effect_manager->get_effekseer_manager(), mPosition);
}

SpearEnemy::SpearEnemy(GraphicsPipeline& Graphics_)
    :BaseEnemy(Graphics_, "./resources/Models/Enemy/enemy_spear.fbx")
{}

void SpearEnemy::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    elapsedTime_=fBaseUpdate(elapsedTime_, Graphics_);

}

void SpearEnemy::fUpdateAttackCapsule()
{

}


void SpearEnemy::fRegisterFunctions()
{
    // �֐����X�e�[�g�}�V���ɓo�^
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


//--------------------<�J�n��>--------------------//
void SpearEnemy::fStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::idle, true);
    // �ėp�^�C�}�[��������
    mWaitTimer = 0.0f;
    mIsAttack = false;
}

void SpearEnemy::fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    if (mDissolve <= 0.0f)
    {
        //-----�o���t���O��true�ɂ���-----//
        is_appears = true;
        fChangeState(DivedState::Idle);
    }
}

//--------------------<�ҋ@>--------------------//
void SpearEnemy::fIdleInit()
{
    // �A�j���[�V�������Đ�
    mpModel->play_animation(mAnimPara,AnimationName::idle, true);

}
void SpearEnemy::fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mWaitTimer += elapsedTime_;
    if(mWaitTimer>=mIdleSec)
    {
        fChangeState(DivedState::Move);
    }
}
//--------------------<�ړ�>--------------------//
void SpearEnemy::fMoveInit()
{
    mpModel->play_animation(mAnimPara,AnimationName::walk, true);
    // �ːi��̍��W��������
    mThrustTarget = mPlayerPosition;

}
void SpearEnemy::fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    //--------------------<�v���C���[�̕����ɉ�]>--------------------//
    fTurnToPlayer(elapsedTime_, 2.0f);
    // �v���C���[�Ɠːi�����̂Ȃ��p�����ȉ��ɂȂ�����ːi����
    auto v1 = mThrustTarget- mPosition;
    auto v2 = Math::GetFront(mOrientation);
    v1 = Math::Normalize(v1);
    v2 = Math::Normalize(v2);
    float dot = Math::Dot(v1, v2);
    dot = acosf(dot);
    dot = DirectX::XMConvertToDegrees(dot);

    // ��苗���ɂȂ�����ːi���[�V�����ɓ���
    if(Math::Length(mPosition-mPlayerPosition)<=mAttackLength&&
        fabs(dot)<=mThrustDegree)
    {
        fChangeState(DivedState::ThrustBegin);
    }
    else
    {
        //--------------------<�v���C���[�̂�������Ɉړ�>--------------------//
        fMoveFront(elapsedTime_, mMoveSpeed);
    }
}
//--------------------<�ːi����>--------------------//
void SpearEnemy::fThrustBeginInit()
{
    audio_manager->play_se(SE_INDEX::ENEMY_EMERGENCE);

    mpModel->play_animation(mAnimPara, AnimationName::attack_idle);
    mWaitTimer = 0.0f;
    mIsAttack = true;
    //-----�U������ɓ��������Ƃ�m�点��-----//
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
//--------------------<����˂��o���A�j���[�V����>--------------------//
void SpearEnemy::fThrustMiddleInit()
{
    audio_manager->play_se(SE_INDEX::SPEAR_RUSH);

    // �˂����̃A�j���[�V�������Đ�
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
//--------------------<�ːi��>--------------------//
void SpearEnemy::fThrustEndInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::attack_down, true);
    mWaitTimer = 0.0f;
}

void SpearEnemy::fThrustEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // ���ʕ����ɓˌ�����
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
    }
}
