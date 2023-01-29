#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"LastBoss.h"
#include"Operators.h"
#include"post_effect.h"
#include"BulletManager.h"
#include "CannonballBullet.h"
#include "DragonBreath.h"
#include "EnemyManager.h"
#include"Operators.h"
#include"audio_manager.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"

LastBoss::BossParamJson LastBoss::fLoadParam()
{
    // Json�t�@�C������l���擾
    std::filesystem::path path = "./resources/Data/BossParam.json";
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);
            o_archive(mBossParam);
        }
    }

    return mBossParam;
}

void LastBoss::fResetLoadRaram()
{
    mBossParam.BossStateNumber = 0;
}

void LastBoss::fSaveParam()
{
    // Json�t�@�C������l���擾
    std::filesystem::path path = "./resources/Data/BossParam.json";
    path.replace_extension(".json");
    //if (std::filesystem::exists(path.c_str()))
    {
        std::ofstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONOutputArchive o_archive(ifs);
            o_archive(mBossParam);
        }
    }
}

//****************************************************************
//
// ��̓��[�h
//
//****************************************************************
void LastBoss::fShipStartInit()
{
    // �A�j���[�V�������Đ�
    mpModel->play_animation(mAnimPara, AnimationName::ship_appearing_scene);
    mDissolve = 0.0f;
    mPosition = { 0.0f,40.0f,250.0f };
    mCurrentMode = Mode::ShipAppear;
    mTimer = 0.0f;
    mShipRoar = false;
    mSkipTimer = 0.0f;
    count_grope_id = 1;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::ShipStart;

}

void LastBoss::fShipStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    if(mTimer>14.5f&& mTimer < 18.0f)
    {
        const int t = static_cast<int>(mTimer * 30);

        if (CorrespondenceManager::Instance().GetMultiPlay() == false)
        {
            if (!mIsSpawnEnemy && t % 10 == 0)
            {
                // �����_���ȓG���o��������
                auto vec = Math::GetFront(mOrientation);
                DirectX::XMFLOAT3 pos = mPosition;
                pos.y = 0.0f;
                mpEnemyManager->fCreateRandomMasterEnemy(Graphics_, pos + (vec * 40.0f), count_grope_id);
                mpEnemyManager->fCreateRandomEnemy(Graphics_, pos + (vec * 40.0f), count_grope_id, 1);
                mpEnemyManager->fCreateRandomEnemy(Graphics_, pos + (vec * 40.0f), count_grope_id, 2);
                mIsSpawnEnemy = true;
                count_grope_id++;
            }
            if (mIsSpawnEnemy && t % 10 == 9)
            {
                mIsSpawnEnemy = false;
            }
        }
    }

    if(mTimer>15.0f&&mShipRoar==false)
    {
        audio_manager->play_se(SE_INDEX::ROAR_3);
        mShipRoar = true;
    }


    //<�}���`�v���C�̎��A�C�x���g���I�����Ă��Ȃ��Ƃ���������Ȃ�>//
    if (CorrespondenceManager::Instance().GetMultiPlay() && end_event == false)
    {
        if (mpModel->end_of_animation(mAnimPara))
        {
            //-----�J�E���g�𑝂₷-----//
            mpEnemyManager->EndEnventCount(1);
            end_event = true;
            SendWatchEndEvent();
        }
    }
    else if (CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)ship_event = true;
    }

    if (ship_event)
    {
        mCurrentMode = Mode::Ship;
        fChangeState(DivideState::ShipIdle);
    }
}

void LastBoss::fShipIdleInit()
{
    mTimer = 0.0f;
    mpModel->play_animation(mAnimPara, AnimationName::ship_idle, true);
    mIsSpawnEnemy = false;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::ShipIdle;
}

void LastBoss::fShipIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    fTurnToPlayer(elapsedTime_, 5.0f);
   if(mTimer>3.0f)
   {
       //<�}���`�v���C���̓z�X�g�����J�ڂ��Ȃ��悤�ɂ���>//
       if (CorrespondenceManager::Instance().GetMultiPlay())
       {
           if (CorrespondenceManager::Instance().GetHost()) fChangeState(DivideState::ShipBeamStart);;
       }
       //<�V���O���v���C�̎��͂��̂܂ܑJ��>//
       else fChangeState(DivideState::ShipBeamStart);

   }

    const int t = static_cast<int>(mTimer * 10);
    if (!mIsSpawnEnemy && t % 10 == 0)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                // �����_���ȓG���o��������
                mpEnemyManager->fCreateRandomMasterEnemy(Graphics_, mPlayerPosition, count_grope_id);
                count_grope_id++;
                mIsSpawnEnemy = true;
            }
        }
        else
        {
            // �����_���ȓG���o��������
            mpEnemyManager->fCreateRandomMasterEnemy(Graphics_, mPlayerPosition, count_grope_id);
            count_grope_id++;
            mIsSpawnEnemy = true;
        }
   }
   if(mIsSpawnEnemy && t % 10 == 9)
   {
       mIsSpawnEnemy = false;
   }

}

void LastBoss::fShipAttackInit()
{

}

void LastBoss::fShipAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

}

void LastBoss::fShipBeamStartInit()
{
    mpEnemyManager->fReserveDeleteEnemies();
    mMoveBegin = mPosition;
    mMoveEnd = { 0.0f,0.0f,600.0f };
    mMoveThreshold = 0.0f;
    mOrientation = { 0.0f,1.0f,0.0f,0.0f };
    mpModel->play_animation(mAnimPara, AnimationName::ship_beam_shot_start);
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::ShipBeamStart;
}

void LastBoss::fShipBeamStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mMoveThreshold += elapsedTime_ * 5.0f;
    mMoveThreshold = (std::min)(1.1f, mMoveThreshold);
    mPosition = Math::lerp(mMoveBegin, mMoveEnd, mMoveThreshold);

    // �A�j���[�V�����I���Ɠ����ɑJ��
    if (mpModel->end_of_animation(mAnimPara) && mMoveThreshold >= 1.0f)
    {
        effect_manager->finalize();
        effect_manager->initialize(Graphics_);
        fChangeState(DivideState::ShipBeamCharge);
    }
}

void LastBoss::fShipBeamChargeInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ship_beam_charge_idle, true);
    mTimer = mkShipBeamChargeSec;

    const auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 up{};
    mpModel->fech_by_bone(mAnimPara, world, mShipFace, mShipFacePosition, up);

    mShipPointer.fSetRadius(0.1f);
    mShipPointer.fSetColor({ 1.0f,0.0f,0.0f,1.0f });
    mShipPointer.fSetAlpha(1.0f);

    // �|�C���^�[�̒�����������
    mPointerLength = 0.0f;

    mpBeamBaseEffect->play(effect_manager->get_effekseer_manager(), { 0.0f,0.0f,520.0f });
    mpBeamBaseEffect->set_scale(effect_manager->get_effekseer_manager(), { 15.0f,15.0f,15.0f });

    // SE���Đ�
    audio_manager->play_se(SE_INDEX::BOSS_CHARGE);
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::ShipBeamCharge;

}

void LastBoss::fShipBeamChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // ���[�U�[�|�C���^�[��L�΂�
    mPointerLength += elapsedTime_ * 1000.0f;


    mShipPointer.fSetLengthThreshold(1.0f);
    const DirectX::XMFLOAT3 front{ Math::GetFront(mOrientation) };
    mShipPointerEnd = mShipFacePosition + (front * mPointerLength );

    // �|�C���^�[�Ɍv�Z���ꂽ�l��ݒ�
    mShipPointer.fSetPosition(mShipFacePosition, mShipPointerEnd);

    // TODO : �����ŏ����߂̃J�����V�F�C�N
    mTimer -= elapsedTime_;
    if(mTimer<=0.0f)
    {
        audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
        // ����
        fChangeState(DivideState::ShipBeamShoot);
    }
}

void LastBoss::fShipBeamShootInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ship_beam_shot_start);
    mTimer = mkShipBeamShootSec;

    //--------------------<���[�U�[�|�C���^�[������>--------------------//
    mShipPointer.fSetAlpha(0.0f);

    //--------------------<���[�U�[�r�[���𔭎˂���>--------------------//
    const auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 up{};
    // �{�[���̈ʒu���擾
    mpModel->fech_by_bone(mAnimPara, world, mShipFace, mShipFacePosition, up);

    // �r�[���̒�����������
    mBeamLength = 0.0f;
    // �r�[���̍U����L��
    mIsAttack = true;

    // �r�[���̔��ˈʒu���v�Z
    const DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 520.0f };
    const auto normVec = Math::Normalize(mPlayerPosition - mPosition);
    float d = Math::Dot(normVec, pos - mPosition);

    mBeamEffectPosition = mPosition + (normVec * d);
    mpBeamEffect->play(effect_manager->get_effekseer_manager(), mBeamEffectPosition, 8);
    mpBeamBaseEffect->play(effect_manager->get_effekseer_manager(), mBeamEffectPosition, 8);

    // �r�d�Đ�
    audio_manager->play_se(SE_INDEX::BOSS_BEAM);

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::ShipBeamShoot;

}

void LastBoss::fShipBeamShootUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    const DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 520.0f };
    const auto normVec = Math::Normalize(mPlayerPosition - mPosition);
    float d = Math::Dot(normVec, pos - mPosition);
    mBeamEffectPosition = mPosition + (normVec * d);
    // �G�t�F�N�g�̈ʒu���X�V
    mpBeamEffect->set_position(effect_manager->get_effekseer_manager(), mBeamEffectPosition);
    mpBeamEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);

    mpBeamBaseEffect->set_position(effect_manager->get_effekseer_manager(), mBeamEffectPosition);
    mpBeamBaseEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);

    const auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 up{};
    // �{�[���̈ʒu���擾
    mpModel->fech_by_bone(mAnimPara, world, mShipFace, mShipFacePosition, up);

    // �r�[���̌����ݒ肷��
    const DirectX::XMFLOAT3 front = Math::GetFront(mOrientation);
    const DirectX::XMFLOAT3 beamEnd{ mShipFacePosition + (front * mBeamLength) };

    // �r�[���̒������X�V����
    mBeamLength += elapsedTime_ * 1000.0f;


    // �U��capsule��ݒ�
    mAttackCapsule.mBottom = mShipFacePosition;
    mAttackCapsule.mTop = beamEnd;
    mAttackCapsule.mRadius = 13.0f;


    fTurnToPlayer(elapsedTime_,6.0f);
    // TODO : �����Ō������J�����V�F�C�N
    mTimer -= elapsedTime_;
    if(mTimer<=0.0f)
    {
        audio_manager->stop_se(SE_INDEX::BOSS_BEAM);
        fChangeState(DivideState::ShipBeamEnd);
        mIsAttack = false;
        mBeamLength = 0.0f;
    }
}

void LastBoss::fShipBeamEndInit()
{
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::ShipBeamEnd;

}

void LastBoss::fShipBeamEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    effect_manager->finalize();
    effect_manager->initialize(Graphics_);
    fChangeState(DivideState::ShipToHuman);
}

void LastBoss::fChangeShipToHumanInit()
{
    audio_manager->stop_all_se();
    mpBeamEffect->stop(effect_manager->get_effekseer_manager());
    mpModel->play_animation(mAnimPara, AnimationName::ship_to_human);
    mDissolve = 0.0f;
    mCurrentMode = Mode::ShipToHuman;
    mPosition = { 0.0f,20.0f,0.0f };
    PostEffect::boss_awakening_effect({ 0.0f,0.0f }, 0.0f, 0.25f);

    mpEnemyManager->fReserveDeleteEnemies();

    // �t���O�̔z���������
    for (int i = 0; i < ARRAYSIZE(mSeArrayShipToHuman); ++i)
    {
        mSeArrayShipToHuman[i] = false;
    }
    mTimer = 0.0f;
    mSkipTimer = 0.0f;
    if (mBossParam.mShowMovie_ShipToHuman)
    {
        mDrawSkip = true;
    }

    //-----�}���`�v���C�Ŏg���Ă���l��������-----//
    end_event = false;

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::ShipToHuman;

}

void LastBoss::fChangeShipToHumanUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{

    if (game_pad->get_button() & GamePad::BTN_B && CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        mSkipTimer += elapsedTime_ * 0.3f;
    }

    mTimer += elapsedTime_;
    // SE��炷
#pragma region SoundSe
    if (mTimer >= 0.9f && mSeArrayShipToHuman[0] == false) // 57
    {
        audio_manager->play_se(SE_INDEX::BOSS_SMALL_ROAR);
        mSeArrayShipToHuman[0] = true;
    }
    if (mTimer >= 2.9f && mSeArrayShipToHuman[1] == false) //
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayShipToHuman[1] = true;
    }
    if (mTimer >= 3.5f && mSeArrayShipToHuman[2] == false) //
    {
        audio_manager->play_se(SE_INDEX::FOOT_TRANSFORM);
        mSeArrayShipToHuman[2] = true;
    }
    if (mTimer >= 5.08f && mSeArrayShipToHuman[3] == false) //
    {
        audio_manager->play_se(SE_INDEX::DOCKING_2);
        mSeArrayShipToHuman[3] = true;
    }
    if (mTimer >= 5.2f && mSeArrayShipToHuman[4] == false) //
    {
        audio_manager->play_se(SE_INDEX::REVERBERATION);
        mSeArrayShipToHuman[4] = true;
    }

#pragma endregion

        //<�}���`�v���C�̎��A�C�x���g���I�����Ă��Ȃ��Ƃ���������Ȃ�>//
    if (CorrespondenceManager::Instance().GetMultiPlay() && end_event == false)
    {
        if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)
        {
            //-----�J�E���g�𑝂₷-----//
            mpEnemyManager->EndEnventCount(1);
            end_event = true;
            SendWatchEndEvent();
        }
    }
    else if(CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)ship_to_human_event = true;
    }

    if (ship_to_human_event)
    {
        PostEffect::clear_post_effect();
        fChangeState(DivideState::HumanIdle);

        // ���݂̃��[�h��l�^�ɕύX����
        mCurrentMode = Mode::Human;
        mPosition = {};

        // �A�j���[�V�����������t���O���I���ɂ���
        mBossParam.mShowMovie_ShipToHuman = true;
        mBossParam.BossStateNumber = 1;
    }
}

void LastBoss::fHumanIdleInit()
{
    mDrawSkip = false;
    mpModel->play_animation(mAnimPara, AnimationName::human_idle, true, true, 0.3f, 1.5f);
    mAnimationSpeed = 1.5f;
    mTimer = 2.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanIdle;

}

void LastBoss::fHumanIdleUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);

   // �����ɉ����čU����i�𕪊򂳂���

    mTimer -= elapsedTime_;
    if (mTimer >= 0.0f)
    {
        return;
    }

    //--------------------<�A�j���[�V�������I��������>--------------------//

     // �̗͂��X�������������ȉ����߂̕K�E�Z
    if (!mFirstSp && fComputePercentHp() < 0.7f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                mAnimationSpeed = 1.0f;
                mFirstSp = true;
                fChangeState(DivideState::HumanSpAway);
            }
        }
        else
        {
            mAnimationSpeed = 1.0f;
            mFirstSp = true;
            fChangeState(DivideState::HumanSpAway);
        }
        return;
    }

    // �X�e�[�W�̌��E�l�𔽉f
    if(fLimitStageHuman(elapsedTime_))
    {
        return;
    }

    // �v���C���[�Ƃ̋������߂�������͈͔����U���𑽂�����
    std::uniform_int_distribution<int> RandTargetAdd(0, 9);
    const int randNumber = RandTargetAdd(mt);
    // ����
    if (Math::Length(mPlayerPosition - mPosition) > mkDistanceToPlayer*10.0f)
    {
        // ����ɋ���������������
        if (randNumber <= 3)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanRush);
                    mAnimationSpeed = 1.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanRush);
                mAnimationSpeed = 1.0f;
            }
            return;
        }
    }

    if(Math::Length(mPlayerPosition-mPosition)>mkDistanceToPlayer)
    {
        if(randNumber>=8)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanBlowAttack);
                    mAnimationSpeed = 1.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanBlowAttack);
                mAnimationSpeed = 1.0f;
            }

            return;
        }
        else  if(randNumber>=6)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanAllShot);
                    mAnimationSpeed = 1.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanAllShot);
                mAnimationSpeed = 1.0f;
            }

            return;
        }
        else if(randNumber>=1)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanRush);
                    mAnimationSpeed = 1.0f;

                }
            }
            else
            {
                fChangeState(DivideState::HumanRush);
                mAnimationSpeed = 1.0f;

            }

            return;
        }
        else
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanRush);
                    mAnimationSpeed = 1.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanRush);
                mAnimationSpeed = 1.0f;
            }

            return;
        }
    }
    else
    {
        if (randNumber > 6)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanBlowAttack);
                    mAnimationSpeed = 1.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanBlowAttack);
                mAnimationSpeed = 1.0f;
            }

            return;
        }
        else if(randNumber>2)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanAllShot);
                    mAnimationSpeed = 1.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanAllShot);
                mAnimationSpeed = 1.0f;
            }

            return;
        }
        else
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanMove);
                    mAnimationSpeed = 1.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanMove);
                mAnimationSpeed = 1.0f;
            }

            return;
        }
    }
}

void LastBoss::fHumanMoveInit()
{

}

void LastBoss::fHumanMoveUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{

}

void LastBoss::fHumanAllShotInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_bullet);
    mTimer = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanAllShot;
}

void LastBoss::fHumanAllShotUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{

    fTurnToPlayer(elapsedTime_, 10.0f);

    // ���e��_��ݒ�
    DirectX::XMFLOAT3 position{ mPlayerPosition };

    //--------------------<�e���̌����ɔ���>--------------------//
    mTimer += elapsedTime_;
    if (mTimer < mkHumanAllShotEnd&& mTimer > mkHumanAllShotBegin)
    {
        // �{�[���������Ă���������v�Z
        DirectX::XMFLOAT3 up{};
        DirectX::XMFLOAT3 leftPosition{};
        DirectX::XMFLOAT4X4 leftRotMat{};

        mpModel->fech_by_bone(mAnimPara, Math::calc_world_matrix(mScale,
            mOrientation, mPosition), mTurretBoneLeft, leftPosition,
            up, leftRotMat);
        const DirectX::XMFLOAT3 leftFront  =
              { leftRotMat._31,leftRotMat._32, leftRotMat._33 };

        // �{�[�����璅�e�_�܂ł̃x�N�g��

        // �{�[���Ƃ̂Ȃ��p���Z�o
        DirectX::XMFLOAT3 v = leftPosition - position;
        v.y = 0.0f;
        v = Math::Normalize(v);
        const DirectX::XMFLOAT3 front =
            Math::Normalize(mPlayerPosition - mPosition);
        float dot = Math::Dot(v, front);

        // �^�[�Q�b�g�̈ʒu����Ƃ��������Œ��e�_�����߂�
        std::uniform_real_distribution<float> RandTargetAdd(-70.0f, -30.0f);
        std::uniform_int_distribution<int> switcher(0, 1);

        DirectX::XMFLOAT3 bulletPosition{};

        bulletPosition.x = RandTargetAdd(mt) * switcher(mt) == 0 ? 1.0f : -1.0f;
        bulletPosition.z = RandTargetAdd(mt) * switcher(mt) == 0 ? 1.0f : -1.0f;
        bulletPosition += position;

        const DirectX::XMFLOAT3 bulletVec{
          Math::Normalize(bulletPosition - leftPosition) };

        const float bulletSpeed = Math::Length(mPlayerPosition - mPosition);


        // ���Ԋu�Ŕ���
        if (mShotTimer <= 0.0f)
        {
            audio_manager->play_se(SE_INDEX::CANNON);
            mfAddBullet(new CannonballBullet(Graphics_,bulletVec ,
               180.0f, leftPosition));
            mShotTimer = mkHumanAllShotDelay;
        }
        else
        {
            mShotTimer -= elapsedTime_;
        }
    }

    if(mpModel->end_of_animation(mAnimPara))
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanIdle);
            }
        }
        else
        {
            fChangeState(DivideState::HumanIdle);
        }

    }
}


void LastBoss::fHumanBlowAttackInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_shockwave);
    mpAllAttackEffect->play(effect_manager->get_effekseer_manager(),
        mPosition, 10.0f);
    mIsAttack = true;
    mAttackCapsule.mRadius = 0.0f;
    mTimer = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanBlowAttack;
}

void LastBoss::fHumanBlowAttackUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);

    mTimer += elapsedTime_;

    if (mTimer > mkTimerBlow)
    {
        constexpr DirectX::XMFLOAT3 capsuleLength{ 0.0f,20.0f,0.0f };
        mAttackCapsule.mRadius += elapsedTime_ * 80.0f;
        mAttackCapsule.mTop = mPosition + capsuleLength;
        mAttackCapsule.mBottom = mPosition - capsuleLength;
    }


    if(mpModel->end_of_animation(mAnimPara))
    {
        std::uniform_int_distribution<int> RandTargetAdd(0, 9);
        if (const auto num = RandTargetAdd(mt); num < 4)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanMove);
                    mIsAttack = false;
                    mAttackCapsule.mRadius = 0.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanMove);
                mIsAttack = false;
                mAttackCapsule.mRadius = 0.0f;
            }

        }
        else
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::HumanIdle);
                    mIsAttack = false;
                    mAttackCapsule.mRadius = 0.0f;
                }
            }
            else
            {
                fChangeState(DivideState::HumanIdle);
                mIsAttack = false;
                mAttackCapsule.mRadius = 0.0f;
            }

        }
    }
}

void LastBoss::fMoveAwayInit()
{
    mMoveBegin = mPosition;
    const auto normalV = Math::Normalize(mPosition - mPlayerPosition);
    mMoveEnd = mPosition + normalV * 100.0f;
    mMoveThreshold = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanMove;
}

void LastBoss::fMoveAwayUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mMoveThreshold += elapsedTime_ * 3.0f;
    mPosition = Math::lerp(mMoveBegin, mMoveEnd, mMoveThreshold);

    if(mMoveThreshold>=1.0f|| Math::Length(mPosition) > mkLimitStage)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanIdle);
            }
        }
        else
        {
            fChangeState(DivideState::HumanIdle);
        }

    }
}

void LastBoss::fHumanRushInit()
{
    // �ړ��ڕW��ݒ�
    mMoveEnd = mPlayerPosition;
    mMoveBegin = mPosition;
    mMoveThreshold = 0.0f;
    mpModel->play_animation(mAnimPara, AnimationName::human_rush_idle);
    audio_manager->play_se(SE_INDEX::BOSS_RUSH);
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanRush;

}

void LastBoss::fHumanRushUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    const float limitLength = Math::Length(mMoveBegin - mMoveEnd);
    const float currentLength = Math::Length(mPosition - mMoveBegin);

    // �I������
    if (currentLength > limitLength * 1.2f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanIdle);
            }
        }
        else
        {
            fChangeState(DivideState::HumanIdle);
        }

        return;
    }

    // �v���C���[�̕����Ɉړ�����x�N�g�����쐬����

    const DirectX::XMFLOAT3 iniVec{
        Math::Normalize(mMoveEnd - mMoveBegin) };
    const DirectX::XMFLOAT3 currentVec{
        Math::Normalize(mPlayerPosition - mMoveBegin) };

    DirectX::XMFLOAT3 endPoint{};
    {
        const float d = Math::Dot(currentVec, mMoveEnd - mMoveBegin);
        const DirectX::XMFLOAT3 end = mMoveBegin + (currentVec * d);
        endPoint=Math::lerp(mMoveBegin, end, mMoveThreshold);
    }


    mMoveThreshold += elapsedTime_ * 0.5f;


    const DirectX::XMFLOAT3 prePosition = mPosition;
    // �X�v���C���Ȑ��Ńx�N�g�����擾
    mPosition=Math::fBezierCurve(mMoveBegin, mMoveEnd, endPoint,mMoveThreshold);

    const DirectX::XMFLOAT3 subVec = Math::Normalize(mPosition - prePosition);

    fTurnToTarget(elapsedTime_, 10.0f, mPosition + subVec);

    if(mMoveThreshold>=1.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                audio_manager->stop_se(SE_INDEX::BOSS_RUSH);
                fChangeState(DivideState::HumanBlowAttack);
            }
        }
        else
        {
            audio_manager->stop_se(SE_INDEX::BOSS_RUSH);
            fChangeState(DivideState::HumanBlowAttack);
        }

    }
}

void LastBoss::fHumanSpAttackAwayInit()
{
    mPosition.y = 6.0f;

   // ��т̂��n�_���Z�b�g
    mAwayBegin = mPosition;
    mAwayLerp = 0.0f;

    mBeginOrientation = mOrientation;

    mpModel->play_animation(mAnimPara, AnimationName::human_to_ship_quick);

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanSpAway;

}

void LastBoss::fHumanSpAttackAwayUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // ���ʂ������悤�ɉ�]
    if(mpModel->end_of_animation(mAnimPara))
    {
        mAwayLerp += elapsedTime_ * 2.0f;
        mAwayLerp = (std::min)(1.0f, mAwayLerp);

        // �ړ�
        mPosition = Math::lerp(mAwayBegin, mAwayEnd, mAwayLerp);
        mOrientation = Math::lerp(mBeginOrientation, mEndOrientation, mAwayLerp);
    }

    if(mAwayLerp>=1.0f)
    {
        fChangeState(DivideState::HumanSpWait);
        //------�}���`�v���C�̏ꍇ�̓z�X�g�����o�����������Ȃ�-----//
        if (CorrespondenceManager::Instance().GetMultiPlay() &&
            CorrespondenceManager::Instance().GetHost())
        {
            fSpawnChildUnit(Graphics_, 6);
        }
        else if (CorrespondenceManager::Instance().GetMultiPlay() == false) fSpawnChildUnit(Graphics_, 6);

    }
}

void LastBoss::fHumanSpAttackSummonInit()
{
    throw std::logic_error("Not implemented");
}

void LastBoss::fHumanSpAttackSummonUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    throw std::logic_error("Not implemented");
}


void LastBoss::fHumanSpAttackWaitInit()
{
    mTimer = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanSpWait;
}

void LastBoss::fHumanSpAttackWaitUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);
    mTimer += elapsedTime_;
    if(mTimer>=mkHumanSpWaitTime)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanSpOver);
            }
        }
        else
        {
            fChangeState(DivideState::HumanSpOver);
        }

    }

    if(mpEnemyManager->fGetEnemyCounts()<=4)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanSpDamage);
            }
        }
        else
        {
            fChangeState(DivideState::HumanSpDamage);
        }

    }
}

void LastBoss::fHumanSpAttackCancelInit()
{

}

void LastBoss::fHumanSpAttackCancelUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

}

void LastBoss::fHumanSpAttackTimeOverInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ship_to_human_quick);
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanSpOver;
}

void LastBoss::fHumanSpAttackTimeOverUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    if(mpModel->end_of_animation(mAnimPara))
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanSpCharge);
            }
        }
        else
        {
            fChangeState(DivideState::HumanSpCharge);
        }
    }
}

void LastBoss::fHumanSpAttackChargeInit()
{
    mAnimationSpeed = 0.7f;
    mpModel->play_animation(mAnimPara, AnimationName::human_beam_charge, true);
    mTimer = mkSpChargeTime;

    // laser��������
    mRightPointer.fSetRadius(0.1f);  // ���a
    mLeftPointer.fSetRadius(0.1f);

    mRightPointer.fSetColor({1.0f,0.0f,0.0f,1.0f}); // �F
    mLeftPointer. fSetColor({1.0f,0.0f,0.0f,1.0f});

    mRightPointer.fSetLengthThreshold(1.0f); // 臒l
    mLeftPointer. fSetLengthThreshold(1.0f);

    mRightPointer.fSetAlpha(1.0f); // �s�����x
    mLeftPointer. fSetAlpha(1.0f);

    mHumanBeamTarget = mPosition;

    audio_manager->play_se(SE_INDEX::BOSS_CHARGE);

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanSpCharge;

}

void LastBoss::fHumanSpAttackChargeUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    mTimer -= elapsedTime_;


    // �|�C���^�[���v���C���[�̈ʒu�ɉ�]������
    mHumanBeamTarget = Math::lerp(mHumanBeamTarget, mPlayerPosition,
        30.0f * elapsedTime_);

    mRightPointer.fSetPosition(mpTurretRight->fGetPosition(), mHumanBeamTarget);
    mLeftPointer.fSetPosition(mpTurretLeft->fGetPosition(), mHumanBeamTarget);
    fTurnToPlayer(elapsedTime_, 5.0f);



    if (mTimer <= 0.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
                fChangeState(DivideState::HumanSpShoot);
                mAnimationSpeed = 1.0f;
            }
        }
        else
        {
            audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
            fChangeState(DivideState::HumanSpShoot);
            mAnimationSpeed = 1.0f;
        }

    }
}

void LastBoss::fHumanSpBeamShootInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_beam_shoot);
    mTimer = 0.0f;

    mRightPointer.fSetAlpha(0.0f);
    mLeftPointer.fSetAlpha(0.0f);
    mAwayBegin = mPosition;
    mAwayLerp = 0.0f;
    mpBeamRightEffect->play(effect_manager->get_effekseer_manager(), mpTurretRight->fGetPosition());
    mpBeamRightEffect->set_scale(effect_manager->get_effekseer_manager(), { 5.0f,5.0f,5.0f });
    mpBeamLeftEffect->play(effect_manager->get_effekseer_manager(), mpTurretLeft->fGetPosition());
    mpBeamLeftEffect->set_scale(effect_manager->get_effekseer_manager(), { 5.0f,5.0f,5.0f });

    mIsAttack = true;

    audio_manager->play_se(SE_INDEX::BOSS_BEAM);

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanSpShoot;

}

void LastBoss::fHumanSpBeamShootUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);

    auto fRotBeamEffect = [=](const DirectX::XMFLOAT4 Orientation_,
        DirectX::XMFLOAT3 Begin_)->DirectX::XMFLOAT4
    {

        // ��]���Z�o
        // ���݂̏�x�N�g��
        DirectX::XMFLOAT3 up{ 0.001f,0.0f,1.0f };
        up = Math::Normalize(up);
        // �I�_�Ƃ̃x�N�g��
        DirectX::XMFLOAT3 cylinderUp = { mHumanBeamTarget - Begin_ };
        cylinderUp = Math::Normalize(cylinderUp);

        // �O�ςŉ�]�����Z�o
        auto cross = Math::Cross(up, cylinderUp);
        cross = Math::Normalize(cross);
        // �������O�̎��N���b�V������̂��߉��̒l����
        if (Math::Length(cross) <= 0.0f)
        {
            cross = { 0.0f,1.0f,0.0f };
        }
        // ���ςŉ�]�p���Z�o
        auto dot = Math::Dot(up, cylinderUp);
        dot = acosf(dot);
        DirectX::XMFLOAT4 dummy{ 0.0f,0.0f,0.0f,1.0f };
        auto rotQua = Math::RotQuaternion(dummy, cross, dot);

        // �⊮
        auto res = DirectX::XMQuaternionSlerp(
            DirectX::XMLoadFloat4(&Orientation_),
            DirectX::XMLoadFloat4(&rotQua),
            1.0f);

        DirectX::XMFLOAT4 ret{};
        DirectX::XMStoreFloat4(&ret, res);
        return ret;
    };

    mBeamRightOrientation = fRotBeamEffect(mBeamRightOrientation, mpTurretRight->fGetPosition());
    mBeamLeftOrientation = fRotBeamEffect(mBeamLeftOrientation, mpTurretLeft->fGetPosition());

    mpBeamLeftEffect->set_position(effect_manager->get_effekseer_manager(), mpTurretLeft->fGetPosition());
    mpBeamLeftEffect->set_quaternion(effect_manager->get_effekseer_manager(), mBeamLeftOrientation);

    mpBeamRightEffect->set_position(effect_manager->get_effekseer_manager(), mpTurretRight->fGetPosition());
    mpBeamRightEffect->set_quaternion(effect_manager->get_effekseer_manager(), mBeamRightOrientation);


    mHumanBeamTarget = Math::lerp(mHumanBeamTarget, mPlayerPosition,
        3.0f * elapsedTime_);
    mAttackCapsule.mBottom = mPosition;
    mAttackCapsule.mTop = mHumanBeamTarget;
    mAttackCapsule.mRadius = 7.0f;

    mTimer += elapsedTime_;
    if(mTimer>mkHumanSpBeamTime)
    {
        if (mIsAttack)
        {
            audio_manager->stop_se(SE_INDEX::BOSS_BEAM);
            mpBeamRightEffect->stop(effect_manager->get_effekseer_manager());
            mpBeamLeftEffect->stop(effect_manager->get_effekseer_manager());
            mIsAttack = false;
        }
        mPosition = Math::lerp(mAwayBegin, { 0.0f,0.0f,0.0f }, mAwayLerp);
        mAwayLerp += elapsedTime_ * 2.0f;
    }
    else
    {

    }
    if (mAwayLerp > 1.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanIdle);
            }
        }
        else
        {
            fChangeState(DivideState::HumanIdle);
        }

    }
}

void LastBoss::fHumanSpDamageInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_damage);
    mAwayBegin = mPosition;
    mAwayLerp = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanSpDamage;

}

void LastBoss::fHumanSpDamageUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    if(mpModel->end_of_animation(mAnimPara))
    {
        mPosition = Math::lerp(mAwayBegin, { 0.0f,0.0f,0.0f }, mAwayLerp);
        mAwayLerp += elapsedTime_ * 2.0f;

    }
    if (mAwayLerp > 1.0f)
    {
        // �_���[�W��^����
        mCurrentHitPoint -= static_cast<int>(static_cast<float>(mMaxHp) * 0.1f);
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::HumanIdle);

            }
        }
        else
        {
            fChangeState(DivideState::HumanIdle);
        }

    }
}


void LastBoss::fHumanDieStartInit()
{
    mPosition.z = 0.0f;
    // �ʒu�����Z�b�g
    mpModel->play_animation(mAnimPara, AnimationName::human_die);
    // TODO : �G�t�F�N�g�̗ނ����ׂă��Z�b�g����

    // TODO : �J�������{�X�̕��Ɍ�����
    if (mBossParam.mShowMovie_HumanToDragon)
    {
        mDrawSkip = true;
    }
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanDieStart;
}

void LastBoss::fHumanDieStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    bool end{ false };
    if (game_pad->get_button() & GamePad::BTN_B && CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        end = true;
    }

    mPosition = Math::lerp(mPosition, { 0.0f,80.0f,0.0f }, 10.0f * elapsedTime_);
   if(mpModel->end_of_animation(mAnimPara))
   {
       end = true;
   }
   if (end)
   {
       effect_manager->finalize();
       effect_manager->initialize(Graphics_);
       fChangeState(DivideState::HumanDieMiddle);
   }
}

void LastBoss::fHumanDieMiddleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_die_idle, true);
    mTimer = mkHumanDieIdleSec;
    mRgbColorPower = 0.0f;
    mHeartTimer = 0.0f;
    mPerformThresold = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanDieMiddle;
}

void LastBoss::fHumanDieMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    bool end{ false };
    if (game_pad->get_button() & GamePad::BTN_B)
    {
        end = true;
    }


    // RGB���炵�ŐS���̂ǂ��ǂ������o����
    mTimer -= elapsedTime_;
    mPerformThresold += elapsedTime_ * 0.5f;
    mPerformThresold = (std::min)(0.15f, mPerformThresold);
    PostEffect::boss_awakening_effect({ 0.5f,0.5f }, mRgbColorPower, mPerformThresold);
    if (mHeartTimer <= 0.0f)
    {
        mRgbColorPower += mRgbColorSpeed * elapsedTime_;
        if (mRgbColorPower > 1.0f)
        {
            mRgbColorSpeed *= -1.0f;
            audio_manager->play_se(SE_INDEX::HEART_BEAT);
        }
        if (mRgbColorPower < 0.0f)
        {
            //  ���������ҋ@����
            mHeartTimer = mkWaitHeartEffect;
            mRgbColorSpeed *= -1.0f;
        }
    }
    else
    {
        mHeartTimer -= elapsedTime_;
    }

    if (mTimer <= 0.0f)
    {
        end = true;
    }
    if (end)
    {

        effect_manager->finalize();
        effect_manager->initialize(Graphics_);
        fChangeState(DivideState::HumanToDragon);
        PostEffect::clear_post_effect();
    }
}

void LastBoss::fHumanToDragonInit()
{
    audio_manager->stop_all_se();
    mpModel->play_animation(mAnimPara,AnimationName::human_to_dragon);
    mDissolve = 0.0f;
    mCurrentMode = Mode::HumanToDragon;
    mPosition = { 0.0f,40.0f,0.0f };
    for (int i = 0; i < ARRAYSIZE(mSeArrayHumanToDragon); ++i)
    {
        mSeArrayHumanToDragon[i] = false;
    }
    mAnimationSpeed = 1.0f;
    mTimer = 0.0f;
    mIsStun = false;
    mStunEffect->stop(effect_manager->get_effekseer_manager());
    mSkipTimer = 0.0f;

    end_event = false;

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::HumanToDragon;
}

void LastBoss::fHumanToDragonUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    if (game_pad->get_button() & GamePad::BTN_B && CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        mSkipTimer += elapsedTime_ * 0.3f;
    }



    mTimer += elapsedTime_;
#pragma region SoundSe
    int i = 0;
    if (mTimer >= 9.76f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::TEARING);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 11.0f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::THROW);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 14.66f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::ROAR_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 21.63f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 21.89f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 22.10f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::BOSS_READY_FAST);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 22.63f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_2);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 23.18f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 23.38f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DISC);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 24.00f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::SPLINTERS);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 26.23f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::BOSS_READY_FAST);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 27.28f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::ROAR_2);
        mSeArrayHumanToDragon[i] = true;
    }

#pragma endregion
    //<�}���`�v���C�̎��A�C�x���g���I�����Ă��Ȃ��Ƃ���������Ȃ�>//
    if (CorrespondenceManager::Instance().GetMultiPlay() && end_event == false)
    {
        if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)
        {
            //-----�J�E���g�𑝂₷-----//
            mpEnemyManager->EndEnventCount(1);
            end_event = true;
            SendWatchEndEvent();
        }
    }
    else if (CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)human_to_dragon_event = true;
    }

    if (human_to_dragon_event)
    {
        DebugConsole::Instance().WriteDebugConsole("�J��", TextColor::Pink);
        mDrawSkip = false;
        fChangeState(DivideState::DragonHideStart);
        mCurrentMode = Mode::Dragon;
        // �̗͂̒l���h���S�����[�h�̏����l�ɂ��Ă���
        mCurrentHitPoint = static_cast<int>(static_cast<float>(mMaxHp) * mkPercentToDragon);
        mPosition = {};

        // �A�j���[�V�����������t���O���I���ɂ���
        mBossParam.mShowMovie_HumanToDragon = true;
        mBossParam.BossStateNumber = 2;
    }
}

void LastBoss::fDragonIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_idle, true);
    mTimer = 2.0f;
    mCurrentMode = Mode::Dragon;
    mDragonBreathCount = 0;
    mAnimationSpeed = 5.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonIdle;

}

void LastBoss::fDragonIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer -= elapsedTime_;
   if(mTimer>=0.0f)
   {
       return;
   }
   //-----�{�X���b�V���U���̃t���Ooff-----//
   mpEnemyManager->SetStartBossRush(false);

    std::uniform_int_distribution<int> RandTargetAdd(0, 9);
    const int randNumber = RandTargetAdd(mt);

    if(randNumber>8)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonIdle);
            }
        }
        else
        {
            fChangeState(DivideState::DragonIdle);
        }
    }
    else if (randNumber > 6)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonRushHide);
            }
        }
        else
        {
            fChangeState(DivideState::DragonRushHide);
        }

    }
    else if(randNumber>2)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonBeamStart);
            }
        }
        else
        {
            fChangeState(DivideState::DragonBeamStart);
        }
    }
    else
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonHideStart);
            }
        }
        else
        {
            fChangeState(DivideState::DragonHideStart);
        }

    }
    mAnimationSpeed = 1.0f;
}

void LastBoss::fDragonFastBreathStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_hide);
    mTimer = mkDragonHideTime;
    mDissolve = 0.0f;
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpSecondGunLeft->fSetDissolve (mDissolve);
    mpTurretLeft->fSetDissolve(mDissolve);
    mpTurretRight->fSetDissolve(mDissolve);
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonHideStart;
}

void LastBoss::fDragonFastBreathStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    mTimer -= elapsedTime_;
    mDissolve += elapsedTime_ * 1.2f;
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpSecondGunLeft->fSetDissolve(mDissolve);
    if(mTimer<0.0f)
    {
        // �f�B�]���u�I��
        fChangeState(DivideState::DragonAppear);
        mDissolve = 1.0f;
        mpSecondGunRight->fSetDissolve(mDissolve);
        mpSecondGunLeft->fSetDissolve(mDissolve);
        mpTurretLeft->fSetDissolve(mDissolve);
        mpTurretRight->fSetDissolve(mDissolve);
    }
}

void LastBoss::fDragonBreathAppearInit()
{
    // �ʒu���ړ�������

    int areaSeed{};
    // �X�e�[�W���l�������č��ǂ��ɂ��邩����i���}�̓G���A�ԍ��j

    /////////////////////////
    //         |           //
    //     0   |     1     //
    //         |           //
    //---------|-----------//
    //         |           //
    //     2   |     3     //
    //         |           //
    /////////////////////////

    if(mPosition.x>0.0f)
    {
        areaSeed++;
    }
    if(mPosition.z<0.0f)
    {
        areaSeed += 2;
    }

    std::vector<int> vec{};
    vec.reserve(3);
    for(int i=0;i<4;++i)
    {
        if (i == areaSeed) continue;;
        vec.emplace_back(i);
    }
    std::uniform_int_distribution<int> RandTargetAdd(0, 2);

    // �ڕW�n�_�̃C���f�b�N�X���擾
    const int areaIndex = vec.at(RandTargetAdd(mt));

    float rad = DirectX::XMConvertToRadians(45.0f);
    switch (areaIndex)
    {
    case 0:
        mPosition = { -cosf(rad),0.0f,sinf(rad) };
        break;
    case 1 :
        mPosition = { cosf(rad),0.0f,sinf(rad) };
        break;
    case 2:
        mPosition = { -cosf(rad),0.0f,-sinf(rad) };
        break;
    case 3:
        mPosition = { cosf(rad),0.0f,-sinf(rad) };
        break;
    default: ;
    }

    mPosition *= (mkLimitStage * 0.5f * 0.7f);

    mDissolve = 1.0f;

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonAppear;
}

void LastBoss::fDragonBreathAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    mDissolve -= elapsedTime_ * 1.3f;
    if(mDissolve<0.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonBreathCharge);
            }
        }
        else
        {
            fChangeState(DivideState::DragonBreathCharge);
        }
    }
}

void LastBoss::fDragonBreathChargeInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_breath_ready, false, true, 0.3f, 0.5f);
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonBreathCharge;

}

void LastBoss::fDragonBreathChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);

    if(mpModel->end_of_animation(mAnimPara))
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonBreathShot);
            }
        }
        else
        {
            fChangeState(DivideState::DragonBreathShot);
        }

    }
}

void LastBoss::fDragonBreathShotInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_breath_start);
    mDragonBreathCount++;
    mTimer = 0.0f;
    mIsShotBreath = false;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonBreathShot;

}

void LastBoss::fDragonBreathShotUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if(mIsShotBreath==false)
    {
        auto p = mPosition;
        p.y += 30.0f;
        audio_manager->play_se(SE_INDEX::FIRE_BOLL);
        mfAddBullet(new DragonBreath(Graphics_, p, 200.0f, mPlayerPosition));
        mIsShotBreath = true;
    }

    if(mpModel->end_of_animation(mAnimPara))
    {
        // �m���ł�����x���i�ő�O��j
        std::uniform_int_distribution<int> RandTargetAdd(0, 9);
        const int randNumber = RandTargetAdd(mt);
        if (std::pow(mDragonBreathCount, 2) < randNumber)
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::DragonHideStart);
                }
            }
            else
            {
                fChangeState(DivideState::DragonHideStart);
            }

        }
        else
        {
            if (CorrespondenceManager::Instance().GetMultiPlay())
            {
                if (CorrespondenceManager::Instance().GetHost())
                {
                    fChangeState(DivideState::DragonIdle);
                }
            }
            else
            {
                fChangeState(DivideState::DragonIdle);
            }

        }
    }
}

void LastBoss::fDragonRushHideInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_hide);
    mDissolve = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonRushHide;

}

void LastBoss::fDragonRushHideUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve += elapsedTime_;
    mpSecondGunLeft->fSetDissolve(mDissolve);
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpTurretLeft->fSetDissolve(mDissolve);
    mpTurretRight->fSetDissolve(mDissolve);

    if(mpModel->end_of_animation(mAnimPara))
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonRushWait);
            }
        }
        else
        {
            fChangeState(DivideState::DragonRushWait);
        }

        // ���b�N�I������Ȃ��悤�ɉ����Ɉړ�������
        mPosition = { 0.0f,500.0f,0.0f };
    }

}

void LastBoss::fDragonRushWaitInit()
{
    mTimer = mkDragonRushWaitTime;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonRushWait;

    //-----�{�X���b�V���U���̃t���Oon-----//
    mpEnemyManager->SetStartBossRush(true);

}

void LastBoss::fDragonRushWaitUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    mTimer -= elapsedTime_;
    // ��b���Ƃɏo��������
    if(mTimer<=3.5f&&mRushVec[0]->fGetIsStart()==false)
    {
        constexpr float rad = { DirectX::XMConvertToRadians(0.0f) };
        constexpr float length = 200.0f * 0.7f;
        mRushVec[0]->fStartAppear(
            {
                cosf(rad) * length,0.0f,sinf(rad) * length
            });
    }
    if (mTimer <= 2.0f && mRushVec[1]->fGetIsStart() == false)
    {
        constexpr float rad = { DirectX::XMConvertToRadians(120.0f) };
        constexpr float length = 200.0f * 0.7f;
        mRushVec[1]->fStartAppear(
            {
                cosf(rad)*length,0.0f,sinf(rad)*length
            });
    }
    if (mTimer <= 0.5f && mRushVec[2]->fGetIsStart() == false)
    {
        constexpr float rad = { DirectX::XMConvertToRadians(240.0f) };
        constexpr float length = 200.0f * 0.7f;
        mRushVec[2]->fStartAppear(
            {
                cosf(rad) * length,0.0f,sinf(rad) * length
            });
    }

    if(mTimer<-2.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonRushAppear);
            }
        }
        else
        {
            fChangeState(DivideState::DragonRushAppear);
        }
    }
}

void LastBoss::fDragonRushAppearInit()
{
    mPosition = { 0.0f,0.0f,0.0f };
    mOrientation = { 0.0f,0.0f,0.0f,1.0f };
    mpModel->play_animation(mAnimPara, AnimationName::dragon_idle);
    mDissolve = 1.0f;

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonRushAppear;

}

void LastBoss::fDragonRushAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    mpSecondGunLeft->fSetDissolve(mDissolve);
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpTurretLeft->fSetDissolve(mDissolve);
    mpTurretRight->fSetDissolve(mDissolve);
    mPosition = { 0.0f,0.0f,0.0f };
    if(mDissolve<0.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonIdle);
            }
        }
        else
        {
            fChangeState(DivideState::DragonIdle);
        }
    }
}

void LastBoss::fDragonBeamMoveInit()
{
    mDragonMoveThreshold = 0.0f;
    mMoveBegin = mPosition;
    mpModel->play_animation(mAnimPara, AnimationName::dragon_idle, true);

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonMoveStart;
}

void LastBoss::fDragonBeamMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
   // �����܂ňړ�
    mDragonMoveThreshold += elapsedTime_;
    mPosition = Math::lerp(mMoveBegin, { 0.0f,0.0f,0.0f }, mDragonMoveThreshold);

    if(mDragonMoveThreshold >=1.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonBeamStart);
            }
        }
        else
        {
            fChangeState(DivideState::DragonBeamStart);
        }

    }
}

void LastBoss::fDragonBeamStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_beam_ready);

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonBeamStart;

}

void LastBoss::fDragonBeamStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    if(mpModel->end_of_animation(mAnimPara))
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                fChangeState(DivideState::DragonBeamCharge);
            }
        }
        else
        {
            fChangeState(DivideState::DragonBeamCharge);
        }

    }
}

void LastBoss::fDragonBeamChargeInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_beam_charge);
    mTimer = mkDragonBeamChargeTime;
    audio_manager->play_se(SE_INDEX::BOSS_CHARGE);
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonBeamCharge;

}

void LastBoss::fDragonBeamChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    mTimer -= elapsedTime_;
    if (mTimer < 0.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
                fChangeState(DivideState::DragonBeamShoot);
            }
        }
        else
        {
            audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
            fChangeState(DivideState::DragonBeamShoot);
        }

    }
}
void LastBoss::fDragonBeamShotInit()
{
    // ���݂̊e�c�����߂�

    auto front = Math::Normalize(Math::GetFront(mOrientation));
    DirectX::XMFLOAT3 worldFront = { 0.0f,0.0f,1.0f };
    float dot = Math::Dot(front, worldFront);
    mStartBeamRadian = acosf(dot);
    mAddRadian = 0.0f;
    mIsAttack = true;
    mBeamLength = 0.0f;
    mpBeamEffect->play(effect_manager->get_effekseer_manager(), mPosition);
    mpBeamEffect->set_scale(effect_manager->get_effekseer_manager(), { 10.0f,10.0f,10.0f });
    audio_manager->play_se(SE_INDEX::BOSS_BEAM);
    mAttackPower = 10;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonBeamShoot;

}

void LastBoss::fDragonBeamShotUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    // �r�[������]������
    mpBeamEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);
    DirectX::XMFLOAT3 pos = mPosition + (Math::GetFront(mOrientation) * 20.0f);
    mpBeamEffect->set_position(effect_manager->get_effekseer_manager(), pos);

    mAddRadian += DirectX::XMConvertToRadians(40.0f * elapsedTime_);
    const float radian = mStartBeamRadian + mAddRadian;
    const DirectX::XMFLOAT3 beamFront = { cosf(radian),0.0f,sinf(radian) };

    mBeamLength += elapsedTime_ * 500.0f;

    mAttackCapsule.mBottom = pos;
    mAttackCapsule.mTop =
        pos + (Math::GetFront(mOrientation) * mBeamLength);
    mAttackCapsule.mRadius = 20.0f;
    fTurnToTarget(elapsedTime_, 10.0f, mPosition + beamFront * 20.0f);


    float rotMax = 500.0f * (1.0f - static_cast<float>(mCurrentHitPoint / mMaxHp));

    if (mAddRadian >= DirectX::XMConvertToRadians(rotMax))
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                mIsAttack = false;
                audio_manager->stop_se(SE_INDEX::BOSS_BEAM);
                mpBeamEffect->stop(effect_manager->get_effekseer_manager());
                fChangeState(DivideState::DragonIdle);
            }
        }
        else
        {
            mIsAttack = false;
            audio_manager->stop_se(SE_INDEX::BOSS_BEAM);
            mpBeamEffect->stop(effect_manager->get_effekseer_manager());
            fChangeState(DivideState::DragonIdle);
        }

    }
}


void LastBoss::fDragonBeamEndInit()
{
    throw std::logic_error("Not implemented");
}

void LastBoss::fDragonBeamEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    throw std::logic_error("Not implemented");
}

void LastBoss::fDragonDieStartInit()
{
    audio_manager->stop_all_se();
    // �ʒu�����Z�b�g
    mPosition = { 0.0f,40.0f,50.0f };
    mpModel->play_animation(mAnimPara, AnimationName::dragon_die);
    // TODO �J�������{�X�ɒ��ڂ�����
    mCurrentMode = Mode::DragonDie;
    mAnimationSpeed = 1.0f;
    mDissolve = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonDieStart;
    is_dragon_die = true;
    end_event = false;
    mSkipTimer = 0.0f;
    dragon_die_event = false;
    mpBeamEffect->stop(effect_manager->get_effekseer_manager());
}

void LastBoss::fDragonDieStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
        //<�}���`�v���C�̎��A�C�x���g���I�����Ă��Ȃ��Ƃ���������Ȃ�>//
    if (CorrespondenceManager::Instance().GetMultiPlay() && end_event == false)
    {
        if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)
        {
            //-----�J�E���g�𑝂₷-----//
            mpEnemyManager->EndEnventCount(1);
            end_event = true;
            SendWatchEndEvent();
        }
    }
    else if (CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)dragon_die_event = true;
    }

    if (dragon_die_event)
    {
        DebugConsole::Instance().WriteDebugConsole("�J��", TextColor::Pink);
        fChangeState(DivideState::DragonDieEnd);
    }


}

void LastBoss::fDragonDieMiddleInit()
{
    mDissolve = 0.0f;
    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::DragonDieEnd;

}

void LastBoss::fDragonDieMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve += elapsedTime_;
    if(mDissolve>=1.0f)
    {
        mIsAlive = false;
        BaseEnemy::fDie(Graphics_);
        mCurrentMode = Mode::BossDieEnd;
        mpEnemyManager->fSetBossMode(mCurrentMode);
        mpDieEffect->play(effect_manager->get_effekseer_manager(), mPosition, 10.0f);
    }
}

void LastBoss::fStunInit()
{
    audio_manager->stop_all_se();
    mpBeamEffect->stop(effect_manager->get_effekseer_manager());
    mIsAttack = false;
    mAttackCapsule.mRadius = 0.0f;
    mStunEffect->play(effect_manager->get_effekseer_manager(), mPosition);
    mStunEffect->set_scale(effect_manager->get_effekseer_manager(), { 15.0f,15.0f,15.0f });
    mTimer = 3.0f;
    mpModel->pause_animation(mAnimPara);

    //-----�X�e�[�g�ݒ�-----//
    ai_state = AiState::Stun;

}

void LastBoss::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer -= elapsedTime_;
    if (mTimer < 0.0f)
    {
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (CorrespondenceManager::Instance().GetHost())
            {
                mIsStun = false;
                mpModel->progress_animation(mAnimPara);
                mStunEffect->stop(effect_manager->get_effekseer_manager());

                switch (mCurrentMode) {
                case Mode::None: break;
                case Mode::Ship:
                    fChangeState(DivideState::ShipIdle);
                    break;
                case Mode::ShipToHuman:
                    break;
                case Mode::Human:
                    fChangeState(DivideState::HumanIdle);
                    break;
                case Mode::HumanToDragon: break;
                case Mode::Dragon:
                    fChangeState(DivideState::DragonIdle);
                    break;
                case Mode::DragonDie: break;
                default:;
                }
            }
        }
        else
        {
            mIsStun = false;
            mpModel->progress_animation(mAnimPara);
            mStunEffect->stop(effect_manager->get_effekseer_manager());

            switch (mCurrentMode) {
            case Mode::None: break;
            case Mode::Ship:
                fChangeState(DivideState::ShipIdle);
                break;
            case Mode::ShipToHuman:
                break;
            case Mode::Human:
                fChangeState(DivideState::HumanIdle);
                break;
            case Mode::HumanToDragon: break;
            case Mode::Dragon:
                fChangeState(DivideState::DragonIdle);
                break;
            case Mode::DragonDie: break;
            default:;
            }
        }

    }
}


void LastBoss::SendWatchEndEvent()
{
    char data = CommandList::WatchEndEvent;
    if (CorrespondenceManager::Instance().GetHost())CorrespondenceManager::Instance().TcpSendAllClient(&data, 1);
    else CorrespondenceManager::Instance().TcpSend(&data, 1);
}

void LastBoss::SendEndEvent()
{
    char data = CommandList::EndEvent;
    if (CorrespondenceManager::Instance().GetHost())CorrespondenceManager::Instance().TcpSendAllClient(&data, 1);
}


void LastBoss::SetEndEvent(bool arg)
{
    switch (ai_state)
    {
    case AiState::ShipStart:
        ship_event = arg;
        SendEndEvent();
        break;
    case AiState::ShipToHuman:
        ship_to_human_event = arg;
        SendEndEvent();
        break;
    case AiState::HumanToDragon:
        human_to_dragon_event = arg;
        SendEndEvent();
    case AiState::DragonDieStart:
            dragon_die_event = arg;
        break;
    default:
        break;
    }

}

void LastBoss::fRender(GraphicsPipeline& graphics)
{
    if (mIsInCamera)
    {
        return;
    }

    SkinnedMesh::mesh_tuple cameraTuple = std::make_tuple("camera_mesh", 1);

    graphics.set_pipeline_preset(SHADER_TYPES::PBR);
    mDissolve = (std::max)(0.0f, mDissolve);
    const DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    const DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };

    mpModel->render(graphics.get_dc().Get(), mAnimPara, world, color, mDissolve,0.0f,{1.0f,1.0f,1.0f,1.0f},0.8f,cameraTuple);

    //mRushVec

    //graphics.set_pipeline_preset(SHADER_TYPES::PBR);

    //--------------------<�^���b�g�`��>--------------------//
    // �^���b�g�̐e�̈ʒu���擾����
    DirectX::XMFLOAT3 rightPosition{};
    DirectX::XMFLOAT3 leftPosition{};
    DirectX::XMFLOAT3 up{};

    mpModel->fech_by_bone(mAnimPara, world, mTurretBoneLeft, leftPosition,
        up);
    mpModel->fech_by_bone(mAnimPara, world, mTurretBoneRight, rightPosition,
        up);

    //--------------------<�Z�J���h�K���`��>--------------------//
    DirectX::XMFLOAT4X4 rightMat{};
    DirectX::XMFLOAT4X4 leftMat{};
    mpModel->fech_by_bone(mAnimPara, world, mSecondGunBoneLeft, leftMat);
    mpModel->fech_by_bone(mAnimPara, world, mSecondGunBoneRight, rightMat);

    mpSecondGunLeft->fRender(graphics, leftMat, leftPosition);
    mpSecondGunRight->fRender(graphics, rightMat, rightPosition);

    mpTurretLeft->fRender(graphics, leftMat, leftPosition);
    mpTurretRight->fRender(graphics, rightMat, rightPosition);

    //--------------------<�r�[����`��>--------------------//
    mShipPointer.fRender(graphics);
    mRightPointer.fRender(graphics);
    mLeftPointer.fRender(graphics);

    if (mDrawSkip && CorrespondenceManager::Instance().GetMultiPlay() == false)
    {
        graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
        fonts->yu_gothic->Begin(graphics.get_dc().Get());
        fonts->yu_gothic->Draw(L"B�{�^���������ŃX�L�b�v", { 800.0f,600.0f }, { 0.7f,0.7f }, { 1.0f,1.0f,1.0f,1.0f });
        fonts->yu_gothic->End(graphics.get_dc().Get());
        graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE,DEPTH_STENCIL::DEON_DWON);
    }
}

bool LastBoss::fDamaged(int Damage_, float InvincibleTime_, GraphicsPipeline& Graphics_, float elapsed_time)
{
    if(mCurrentMode==Mode::Ship)
    {
        return false;
    }

    bool ret{ false };
    if (mInvincibleTime <= 0.0f && mCurrentHitPoint > 0)
    {
        if (mIsLockOnOfChain)
        {
            Damage_ = 15;
        }
        mCurrentHitPoint -= Damage_;
        mInvincibleTime = InvincibleTime_;
        mDamageEffect->play(effect_manager->get_effekseer_manager(),
            mPosition, 10.0f);
        ret = true;
    }



    if (mCurrentHitPoint <= 0 && is_dragon_die == false)
    {
        effect_manager->finalize();
        effect_manager->initialize(Graphics_);
        fChangeState(DivideState::DragonDieStart);
    }
    return ret;
}

void LastBoss::fDie(GraphicsPipeline& graphics)
{

}
