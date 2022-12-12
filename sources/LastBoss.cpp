#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"LastBoss.h"

#include "BulletManager.h"
#include"EnemyManager.h"
#include"Operators.h"
#include"DragonBreath.h"
#include"enemy_hp_gauge.h"
#include"DebugConsole.h"
#include"Correspondence.h"

LastBoss::LastBoss(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_,
    EnemyManager* pEnemyManager_)
    :BaseEnemy(Graphics_,
        "./resources/Models/Enemy/boss_animation_sixth.fbx",
        ParamPack_, EmitterPoint_,L"./resources/Sprites/ui/minimap/minimap_lastboss.png"), mpEnemyManager(pEnemyManager_)
{
    mIsBoss = true;

    // �{�[����������
    mShipFace = mpModel->get_bone_by_name("shipface_top_joint");

    // �^���b�g�̃��f����������
    fRegisterFunctions();


    // laser��������
    mShipPointer.fInitialize(Graphics_.get_device().Get(),  L"");
    mRightPointer.fInitialize(Graphics_.get_device().Get(), L"");
    mLeftPointer.fInitialize(Graphics_.get_device().Get(),  L"");


    // �^���b�g��������
    mpTurretLeft = std::make_unique<Turret>(Graphics_);
    mpTurretRight = std::make_unique<Turret>(Graphics_);
    // �^���b�g�̃{�[����������
    mTurretBoneLeft = mpModel->get_bone_by_name
    ("armor_L_fire_joint");
    mTurretBoneRight= mpModel->get_bone_by_name
    ("armor_R_fire_joint");

    mpSecondGunLeft = std::make_unique<SecondGun>(Graphics_);
    mpSecondGunRight = std::make_unique<SecondGun>(Graphics_);
    mSecondGunBoneLeft = mpModel->get_bone_by_name
    ("secondarygun_L_root_joint");
    mSecondGunBoneRight = mpModel->get_bone_by_name
    ("secondarygun_R_root_joint");

    // �e���˗p�̊֐����擾
    mfAddBullet= BulletManager::Instance().fGetAddFunction();


    // ���b�V������G��o�^
    //-----�`��̓}�l�[�W���[�ōs��-----//
    DirectX::XMFLOAT3 entryPos{ 0.0f,500.0f,0.0f };
    auto  enemy = new BossRushUnit(Graphics_,entryPos);
    enemy->SetEnemyType(EnemyType::BossRush);
    pEnemyManager_->fAddRushBoss(enemy);
    mRushVec.emplace_back(enemy);

    auto  enemy1 = new BossRushUnit(Graphics_,entryPos);
    enemy1->SetEnemyType(EnemyType::BossRush);
    pEnemyManager_->fAddRushBoss(enemy1);
    mRushVec.emplace_back(enemy1);

    auto  enemy2 = new BossRushUnit(Graphics_,entryPos);
    enemy2->SetEnemyType(EnemyType::BossRush);
    pEnemyManager_->fAddRushBoss(enemy2);
    mRushVec.emplace_back(enemy2);

    //�G�t�F�N�g��������
    mpAllAttackEffect = std::make_unique<Effect>(Graphics_,
        effect_manager->get_effekseer_manager(),
        "./resources/Effect/boss_wave.efk");

    mpBeamEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/boss_beam_big.efk");

    mpBeamBaseEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/beam_base2.efk");

    mpBeamRightEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/boss_beam_big.efk");
    mpBeamLeftEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/boss_beam_big.efk");
    mpDieEffect = std::make_unique<Effect>(Graphics_,
        effect_manager->get_effekseer_manager(),
        "./resources/Effect/boss_death.efk");


   // �{�[�����擾
   mCameraEyeBone = mpModel->get_bone_by_name("camera_joint");
   mCameraFocusBone = mpModel->get_bone_by_name("camera_focus_joint");


    // �p�����[�^�[�����[�h
   fLoadParam();

    // ���e�ɉ����ď����X�e�[�^�X��؂�ւ���
   if (mBossParam.BossStateNumber == 0) // ���
   {
       // �������Ȃ�
   }
   else if (mBossParam.BossStateNumber == 1) // �l�^
   {
       fChangeState(DivideState::ShipToHuman);
   }
   else if(mBossParam.BossStateNumber == 2) // �h���S��
   {
       fChangeState(DivideState::HumanToDragon);
   }

   auto a = this;

   mVernierEffect->stop(effect_manager->get_effekseer_manager());

   //-----�o���t���O��true�ɂ���-----//
   is_appears = true;
}

LastBoss::LastBoss(GraphicsPipeline& Graphics_)
    : BaseEnemy(Graphics_, "./resources/Models/Enemy/boss_animation_sixth.fbx")
{

    mScale = { 0.05f,0.05f,0.05f };
    mPosition = { 0.0f,20.0f,250.0f };
}

LastBoss::~LastBoss()
{
    fSaveParam();
    mpBeamEffect->stop(effect_manager->get_effekseer_manager());
    mpBeamBaseEffect->stop(effect_manager->get_effekseer_manager());
    mpBeamRightEffect->stop(effect_manager->get_effekseer_manager());
    mpBeamLeftEffect->stop(effect_manager->get_effekseer_manager());
    mpDieEffect->stop(effect_manager->get_effekseer_manager());
}

void LastBoss::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    BossHpGauge::set_hp_percent(fGetPercentHitPoint());

    elapsedTime_ = fBaseUpdate(elapsedTime_, Graphics_);
    fGuiMenu();

    // ���[�U�[�|�C���^�[���X�V
    mShipPointer.fUpdate();
    mRightPointer.fUpdate();
    mLeftPointer.fUpdate();

    // �e���[�h���玟�̃��[�h�ɑJ�ڂ���
    switch (mCurrentMode) {
    case Mode::Ship:
        break;
    case Mode::Human:
        if (CorrespondenceManager::Instance().GetMultiPlay() && CorrespondenceManager::Instance().GetHost())
        {
            //�̗͂�����̊��������������
            fChangeHumanToDragon();
        }
        else if (CorrespondenceManager::Instance().GetMultiPlay() == false)
        {
            fChangeHumanToDragon();
        }
        break;
    case Mode::Dragon:
        break;
    default:;
    }

    // �^���b�g���X�V
    mpTurretLeft->fUpdate(elapsedTime_, Graphics_);
    mpTurretRight->fUpdate(elapsedTime_, Graphics_);


    mpEnemyManager->fSetBossMode(mCurrentMode);

    // �{�X�ɃJ����������������
    DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale,
        mOrientation, mPosition);
    DirectX::XMFLOAT3 eyePosition{};
    DirectX::XMFLOAT3 focusPosition{};
    DirectX::XMFLOAT3 dummyUp{};
    // Eye���擾
    mpModel->fech_by_bone(mAnimPara,world, mCameraEyeBone, eyePosition, dummyUp);
    mpModel->fech_by_bone(mAnimPara,world, mCameraFocusBone, focusPosition, dummyUp);
    if(mpEnemyManager)
    {
        mpEnemyManager->fSetBossEye(eyePosition);
        mpEnemyManager->fSetBossFocus(focusPosition);
    }
    else
    {
        throw std::logic_error("EnemyManager Not Find");
    }

}

void LastBoss::fUpdateAttackCapsule()
{
    throw std::logic_error("Not implemented");
}


void LastBoss::fSetStun(bool Arg_, bool IsJust_)
{
    // �D�̏�Ԃ̓X�^�����Ȃ�
    if (mCurrentMode == Mode::Ship || mPosition.y >= 10.0f)
    {
        return;
    }

    if(IsJust_)
    {
        mIsStun = true;
        fChangeState(DivideState::Stun);
        //-----�X�^���ɂȂ������Ƃ𑗐M����-----//
        SendEnemyConditionData(EnemySendData::EnemyConditionEnum::Stun);

    }
}

void LastBoss::fSetEnemyState(int state)
{
    //-----����AI�Ɠ����Ȃ珈�������Ȃ�-----//
    if (ai_state == state) return;
    //-----�X�^�����Ă�Ȃ珈�������Ȃ�-----//
    if (mIsStun) return;

    if (ai_state == AiState::ShipStart || ai_state == AiState::ShipToHuman || ai_state == AiState::HumanToDragon) return;
    std::string text = "ReceiveState : " + std::to_string(state);
    DebugConsole::Instance().WriteDebugConsole(text,TextColor::SkyBlue);
    //-----���ꂼ��̃X�e�[�g�ɑJ��-----//
    switch (state)
    {
    case AiState::ShipStart:
    {
        if(ship_event == false)fChangeState(DivideState::ShipStart);
        break;
    }
    case AiState::ShipIdle: fChangeState(DivideState::ShipIdle); break;
    case AiState::ShipAttack: fChangeState(DivideState::ShipAttack); break;
    case AiState::ShipBeamStart: fChangeState(DivideState::ShipBeamStart); break;
    case AiState::ShipBeamCharge: fChangeState(DivideState::ShipBeamCharge); break;
    case AiState::ShipBeamShoot: fChangeState(DivideState::ShipBeamShoot); break;
    case AiState::ShipBeamEnd: fChangeState(DivideState::ShipBeamEnd); break;
    case AiState::ShipToHuman:
    {
        if (ship_to_human_event == false)fChangeState(DivideState::ShipToHuman);
        break;
    }
    case AiState::HumanIdle: fChangeState(DivideState::HumanIdle); break;
    case AiState::HumanMove: fChangeState(DivideState::HumanMove); break;
    case AiState::HumanAllShot: fChangeState(DivideState::HumanAllShot); break;
    case AiState::HumanRotAttack: fChangeState(DivideState::HumanRotAttack); break;
    case AiState::HumanWithdrawal_Begin: fChangeState(DivideState::HumanWithdrawal_Begin); break;
    case AiState::HumanWithdrawal_Middle: fChangeState(DivideState::HumanWithdrawal_Middle); break;
    case AiState::HumanWithdrawal_End: fChangeState(DivideState::HumanWithdrawal_End); break;
    case AiState::HumanBlowAttack: fChangeState(DivideState::HumanBlowAttack); break;
    case AiState::HumanSpAway: fChangeState(DivideState::HumanSpAway); break;
    case AiState::HumanSpWait: fChangeState(DivideState::HumanSpWait); break;
    case AiState::HumanSpOver: fChangeState(DivideState::HumanSpOver); break;
    case AiState::HumanSpCharge: fChangeState(DivideState::HumanSpCharge); break;
    case AiState::HumanSpShoot: fChangeState(DivideState::HumanSpShoot); break;
    case AiState::HumanSpDamage: fChangeState(DivideState::HumanSpDamage); break;
    case AiState::HumanRush: fChangeState(DivideState::HumanRush); break;
    case AiState::HumanDieStart:
    case AiState::HumanDieMiddle:
    case AiState::HumanToDragon:
    {
        //-----�C�x���g������Ȃ��Ƃ��ɂ�������Ȃ�-----//
        if (human_to_dragon_event == false && mCurrentMode != Mode::HumanToDragon)
        {
            fChangeState(DivideState::HumanDieStart);
            // �ό`�����[�h�ɑJ�ځi�_���[�W�͎󂯂Ȃ��j
            mCurrentMode = Mode::HumanToDragon;
        }
        break;
    }
    case AiState::DragonIdle: fChangeState(DivideState::DragonIdle); break;
    case AiState::DragonDieStart: fChangeState(DivideState::DragonDieStart); break;
    case AiState::DragonDieEnd: fChangeState(DivideState::DragonDieEnd); break;
    case AiState::DragonHideStart: fChangeState(DivideState::DragonHideStart); break;
    case AiState::DragonAppear: fChangeState(DivideState::DragonAppear); break;
    case AiState::DragonBreathCharge: fChangeState(DivideState::DragonBreathCharge); break;
    case AiState::DragonBreathShot: fChangeState(DivideState::DragonBreathShot); break;
    case AiState::DragonRushHide: fChangeState(DivideState::DragonRushHide); break;
    case AiState::DragonRushAppear: fChangeState(DivideState::DragonRushAppear); break;
    case AiState::DragonMoveStart: fChangeState(DivideState::DragonMoveStart); break;
    case AiState::DragonBeamStart: fChangeState(DivideState::DragonBeamStart); break;
    case AiState::DragonBeamCharge: fChangeState(DivideState::DragonBeamCharge); break;
    case AiState::DragonBeamShoot: fChangeState(DivideState::DragonBeamShoot); break;
    case AiState::DragonBeamEnd: fChangeState(DivideState::DragonBeamEnd); break;
    case AiState::Stun: fChangeState(DivideState::Stun); break;
    default:
        break;
    }


}

void LastBoss::fRegisterFunctions()
{
    {
        InitFunc ini = [=]()->void
        {
            fShipStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipIdle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamCharge, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamShootInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamShootUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamShoot, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamEndInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamEndUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamEnd, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fChangeShipToHumanInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fChangeShipToHumanUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipToHuman, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanIdle, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanAllShotInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanAllShotUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanAllShot,
            tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanDieStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanDieStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanDieStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanDieMiddleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanDieMiddleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanDieMiddle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanToDragonInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanToDragonUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanToDragon, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanBlowAttackInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanBlowAttackUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanBlowAttack, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackAwayInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackAwayUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpAway, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackWaitInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackWaitUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpWait, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackTimeOverInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackTimeOverUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpOver, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpBeamShootInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpBeamShootUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpShoot,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpCharge, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpDamageInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpDamageUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpDamage, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fMoveAwayInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fMoveAwayUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanMove, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanRushInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanRushUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanRush, tuple));
    }


    // �h���S���F�ҋ@
    {
        InitFunc ini = [=]()->void
        {
            fDragonIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonIdle, tuple));
    }

    // �h���h���u���X�O�ɏ�����
    {
        InitFunc ini = [=]()->void
        {
            fDragonFastBreathStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonFastBreathStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonHideStart, tuple));
    }
    // �h���S���F�����
    {
        InitFunc ini = [=]()->void
        {
            fDragonBreathAppearInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBreathAppearUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonAppear, tuple));
    }
    // �u���X���߂�
    {
        InitFunc ini = [=]()->void
        {
            fDragonBreathChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBreathChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBreathCharge, tuple));
    }
    // �u���X����
    {
        InitFunc ini = [=]()->void
        {
            fDragonBreathShotInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBreathShotUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBreathShot, tuple));
    }

    // �ːi������
    {
        InitFunc ini = [=]()->void
        {
            fDragonRushHideInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonRushHideUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonRushHide, tuple));
    }
    // �ːi�ҋ@
    {
        InitFunc ini = [=]()->void
        {
            fDragonRushWaitInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonRushWaitUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonRushWait ,tuple));
    }
    // �ːi�����
    {
        InitFunc ini = [=]()->void
        {
            fDragonRushAppearInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonRushAppearUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonRushAppear, tuple));
    }

    // �r�[���O�Ɉړ�
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamMoveInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamMoveUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonMoveStart, tuple));
    }
    // �r�[���O�Ɉړ�
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBeamStart, tuple));
    }

    // �r�[������
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBeamCharge, tuple));
    }
    // �r�[������
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamShotInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamShotUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBeamShoot, tuple));
    }

    // �h���S���F���S
    {
        InitFunc ini = [=]()->void
        {
            fDragonDieStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonDieStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonDieStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fDragonDieMiddleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonDieMiddleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonDieEnd, tuple));
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


    fChangeState(DivideState::ShipStart);
}

void LastBoss::fGuiMenu()
{
    imgui_menu_bar("Enemy", "LastBoss", display_scape_imgui);


#ifdef USE_IMGUI
    if (display_scape_imgui)
    {
        ImGui::Begin("LastBoss");
        ImGui::Text("ID%d", object_id);
        ImGui::Text("Type%d", type);
        ImGui::DragFloat3("Position", &mPosition.x);
        ImGui::DragFloat3("Scale", &mScale.x);
        ImGui::DragFloat4("Orientation", &mOrientation.x);

        if (ImGui::Button("Beam"))
        {
            fChangeState(DivideState::ShipBeamStart);
        }

        // ���݂�Mode��\�L
        switch (mCurrentMode) {
        case Mode::Ship:
            ImGui::Text("Mode::Ship");
            break;
        case Mode::Human:
            ImGui::Text("Mode::Human");
            break;
        case Mode::Dragon:
            ImGui::Text("Mode::Dragon");
            break;
        case Mode::HumanToDragon:
            ImGui::Text("Mode::HumanToDragon");
            break;
        case Mode::None: break;
        case Mode::ShipToHuman: break;
        case Mode::DragonDie: break;
        default:;
        }

        if (ImGui::TreeNode("Hp"))
        {
            ImGui::InputInt("CurrentHp", &mCurrentHitPoint);
            float p = fComputePercentHp();
            ImGui::SliderFloat("Percent", &p, 0.0f, 1.0f);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("State"))
        {
            if (ImGui::Button("HumanAllShot"))
            {
                fChangeState(DivideState::HumanAllShot);
            }
            if (ImGui::Button("HumanBlow"))
            {
                fChangeState(DivideState::HumanBlowAttack);
            }
            if (ImGui::Button("HumanAway"))
            {
                fChangeState(DivideState::HumanSpAway);
            }
            if (ImGui::Button("Rush"))
            {
                fChangeState(DivideState::HumanRush);
            }
            if (ImGui::Button("DragonHide"))
            {
                fChangeState(DivideState::DragonHideStart);
            }
            if (ImGui::Button("DragonRush"))
            {
                fChangeState(DivideState::DragonRushHide);
            }
            if (ImGui::Button("DragonBeam"))
            {
                fChangeState(DivideState::DragonMoveStart);
            }
            ImGui::TreePop();
        }


        if (ImGui::TreeNode("Area"))
        {
            int areaSeed{};
            if (mPosition.x > 0.0f)
            {
                areaSeed++;
            }
            if (mPosition.z < 0.0f)
            {
                areaSeed += 2;
            }
            ImGui::DragInt("areaSeed", &areaSeed);

            ImGui::TreePop();
        }

        if (ImGui::Button("RushUnit"))
        {
            for (auto rush : mRushVec)
            {
                rush->fStartAppear({ 0.0f,0.0f,0.0f });
            }
        }

        ImGui::SliderFloat("MoveThreshold", &mMoveThreshold, 0.0f, 1.0f);
        float v = Math::Length(mPosition);
        ImGui::DragFloat("Length", &v);

        ImGui::RadioButton("FarRand", Math::Length(mPlayerPosition - mPosition) > mkDistanceToPlayer);

        // �J����
        DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale,
            mOrientation, mPosition);
        DirectX::XMFLOAT3 eyePosition{};
        DirectX::XMFLOAT3 focusPosition{};
        DirectX::XMFLOAT3 dummyUp{};
        // Eye���擾
        mpModel->fech_by_bone(mAnimPara, world, mCameraEyeBone, eyePosition, dummyUp);
        mpModel->fech_by_bone(mAnimPara, world, mCameraFocusBone, focusPosition, dummyUp);

        ImGui::DragFloat3("CameraEye", &eyePosition.x);
        ImGui::DragFloat3("CameraFocus", &focusPosition.x);

        int hp = mMaxHp;
        ImGui::DragInt("MaxHp", &hp);

        static DirectX::XMFLOAT3 pos{};
        ImGui::DragFloat3("pp", &pos.x);
        if (ImGui::Button("Effect"))
        {
            mpBeamEffect->play(effect_manager->get_effekseer_manager(), pos);
            mpBeamEffect->set_scale(effect_manager->get_effekseer_manager(), { 15.0f,15.0f,15.0f });
        }

        ImGui::RadioButton("IsAttack", mIsAttack);
        ImGui::Checkbox("Stun", &mIsStun);
        ImGui::DragFloat("Timer", &mTimer);

        ImGui::InputInt("AttackPower", &mAttackPower);

        ImGui::DragFloat("Dissolve", &mDissolve, 0.0f, 1.0f);

        ImGui::End();
    }
#endif

}



float LastBoss::fComputePercentHp() const
{
    return static_cast<float>(mCurrentHitPoint) / static_cast<float>(mMaxHp);
}

void LastBoss::fChangeHumanToDragon()
{
    // �̗͂̊�����20%����������烂�[�h���h���S���ɕς���
    if(fComputePercentHp()<=mkPercentToDragon)
    {
        fChangeState(DivideState::HumanDieStart);
        // �ό`�����[�h�ɑJ�ځi�_���[�W�͎󂯂Ȃ��j
        mCurrentMode = Mode::HumanToDragon;
    }
}

void LastBoss::fSpawnChildUnit(GraphicsPipeline& Graphics_, int Amounts_) const
{
    // ���j�b�g����������
    std::vector<DirectX::XMFLOAT3> vec{};
    vec.reserve(Amounts_);

    // �萔
    const DirectX::XMFLOAT3 SummonCenterPosition = mPlayerPosition;

    // ��̓�����̉�]�p���Z�o����
    const float peaceOfRotation = 360.0f / static_cast<float>(Amounts_);
    for (int i = 0; i < Amounts_; ++i)
    {
        // ��]�p�ɉ������ʒu�����肷��
        const float rot = DirectX::XMConvertToRadians(peaceOfRotation * static_cast<float>(i));
        const DirectX::XMFLOAT3 unitPosition
        = { cosf(rot),0.0f,sinf(rot) };

        vec.emplace_back(SummonCenterPosition + (unitPosition * 60.0f));
    }

    mpEnemyManager->fReserveBossUnit(vec);

}

bool LastBoss::fLimitStageHuman(float elapsedTime_)
{
    // �����X�e�[�W�̒[�𒴂�����
    if (Math::Length(mPosition) < mkLimitStage)
    {
        return false;
    }

    fChangeState(DivideState::HumanRush);
    return true;
}


