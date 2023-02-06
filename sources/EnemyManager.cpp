#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"EnemyManager.h"
#include "EnemyFileSystem.h"
#include"EnemyStructuer.h"
#include"WaveManager.h"
#include"TestEnemy.h"
#include"NormalEnemy.h"
#include"ChaseEnemy.h"
#include"ArcherEnemy.h"
#include"ArcherEnemy_Ace.h"
#include"FrontShieldEnemy.h"
#include"ShieldEnemy_Ace.h"
#include"MiddleBoss.h"
#include"SwordEnemy.h"
#include"SpearEnemy.h"
#include"SpearEnemy_Ace.h"
#include"LastBoss.h"
#include"TutorialEnemy.h"
#include"SwordEnemyAce.h"
#include"BossUnit.h"
#include "game_icon.h"

#include"imgui_include.h"
#include "user.h"
#include"collision.h"
#include"Operators.h"

#include"Correspondence.h"
#include"NetWorkInformationStucture.h"

#include <fstream>
#include <chrono>


//****************************************************************
//
// �G�̊Ǘ��N���X
//
//****************************************************************

EnemyManager::EnemyManager()
{
    //-----�G�̃}�X�^�[�f�[�^�Ǘ��N���X�̎��Ԑ���-----//
    master_enemy_data = std::make_unique<MasterEnemyDataAdmin>();
}

EnemyManager::~EnemyManager()
{
}

void EnemyManager::fInitialize(GraphicsPipeline& graphics_, AddBulletFunc Func_)
{
    //--------------------<������>--------------------//
    fAllClear();
    mUniqueCount = 0;


    // �L���b�V���ɓo�^
    fRegisterCash(graphics_);

    // �`���[�g���A�����ǂ�����������
    mIsTutorial = false;

}

void EnemyManager::RestartInitialize()
{
    //--------------------<������>--------------------//
    fAllClear();
    mUniqueCount = 0;
}

void EnemyManager::fUpdate(GraphicsPipeline& graphics_, float elapsedTime_,AddBulletFunc Func_)
{
    //--------------------<�Ǘ��N���X���̂̍X�V����>--------------------//


    mSloeTime -= elapsedTime_;

    // �E�F�[�u�J�n����̎��Ԃ��X�V
    mDelay-=elapsedTime_;
    if (!mDebugMode && !mIsTutorial && mDelay <= 0.0f)
    {
        //--------------------<�G�����܂肷������>--------------------//
        if (mEnemyVec.size() < 30)
        {
            mWaveTimer += elapsedTime_;
        }
    }

    // �J�����V�F�C�N
    if(mCameraShakeTime>0.0f)
    {
        if (GameFile::get_instance().get_shake()) camera_shake->shake(graphics_, elapsedTime_);
    }
    mCameraShakeTime -= elapsedTime_;
    mCameraShakeTime = (std::max)(0.0f, mCameraShakeTime);

    //--------------<�v���C���[���`�F�C�����̓G�l�~�[�̍s�������ׂĒ�~������>-------------//
    if(mIsPlayerChainTime)
    {
        for (const auto enemy : mEnemyVec)
        {
            //-----�̗͂�0�̎��Ɏ��S���������ʂ�-----//
            if (enemy->fGetCurrentHitPoint() > 0) continue;

            enemy->fDie(graphics_);


            if (enemy->fGetIsAlive() == false)
            {
                mRemoveVec.emplace_back(enemy);
            }
        }
        return;
    }

    //-----�G�̃��[�_�[�̃f�[�^��ݒ�-----//
    SetEnemyGropeHostData();

    //--------------------<�G�̍X�V����>--------------------//
    fEnemiesUpdate(graphics_,elapsedTime_);

    //-----�G�̃f�[�^��ݒ�,���M-----//
    fCheckSendEnemyData(elapsedTime_);

    //--------------------<�G���m�̓����蔻��>--------------------//
    fCollisionEnemyVsEnemy();

    //--------------------<�G�̃X�|�i�[>--------------------//
    fSpawn(graphics_);
    // ImGui�̃��j���[
    fGuiMenu(graphics_,Func_);

    //--------------------<�{�X���G����������>--------------------//
    fCreateBossUnit(graphics_);

    bool isCreate{};
    for (const auto& source : mReserveVec)
    {
        isCreate = true;
        fSpawn(source, graphics_);
    }
    if(isCreate)
    {
        mReserveVec.clear();
    }

    // �U�R�I������S��������
    if(mIsReserveDelete)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetIsBoss() == false)
            {
                enemy->fDie(graphics_);
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }
}

void EnemyManager::fDebugUpdate(GraphicsPipeline& graphics_, float elapsedTime_, AddBulletFunc Func_)
{
    for (const auto enemy : mEnemyVec)
    {
        //-----�G�̍X�V-----//
        enemy->fUpdate(graphics_, elapsedTime_);
    }
    // ImGui�̃��j���[
    fGuiMenu(graphics_, Func_);

}

void EnemyManager::fHostUpdate(GraphicsPipeline& graphics_, float elapsedTime_, AddBulletFunc Func_, EnemyAllDataStruct& receive_data)
{
    //--------------------<�Ǘ��N���X���̂̍X�V����>--------------------//
    mSloeTime -= elapsedTime_;

    // �E�F�[�u�J�n����̎��Ԃ��X�V
    mDelay -= elapsedTime_;
    if (!mDebugMode && !mIsTutorial && mDelay <= 0.0f)
    {
        //--------------------<�G�����܂肷������>--------------------//
        if (mEnemyVec.size() < 30)
        {
            mWaveTimer += elapsedTime_;
        }
    }

    // �J�����V�F�C�N
    if (mCameraShakeTime > 0.0f)
    {
        if (GameFile::get_instance().get_shake()) camera_shake->shake(graphics_, elapsedTime_);
    }
    mCameraShakeTime -= elapsedTime_;
    mCameraShakeTime = (std::max)(0.0f, mCameraShakeTime);

    //--------------<�v���C���[���`�F�C�����̓G�l�~�[�̍s�������ׂĒ�~������>-------------//
    if (mIsPlayerChainTime)
    {
        for (const auto enemy : mEnemyVec)
        {
            //-----�̗͂�0�̎��Ɏ��S���������ʂ�-----//
            if (enemy->fGetCurrentHitPoint() > 0) continue;

            enemy->fDie(graphics_);


            if (enemy->fGetIsAlive() == false)
            {
                mRemoveVec.emplace_back(enemy);
            }
        }
        return;
    }


    //-----��M�����G�̃f�[�^��ݒ肷��-----//
    for (const auto& data : receive_data.enemy_die_data)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetObjectId() == data.object_id)
            {
                enemy->fDie(graphics_);
            }
        }
    }

    //----��ԃf�[�^��ݒ�-----//
    for (const auto& data : receive_data.enemy_condition_data)
    {
        fSetReceiveConditionData(data);
    }

    //-----�ړ��f�[�^��ݒ�-----//
    for (const auto& all_data : receive_data.enemy_move_data)
    {
        for (const auto& e_data : all_data.enemy_data)
        {
            fSetReceiveEnemyData(elapsedTime_, e_data,graphics_);
        }
    }


    //-----�G�̃��[�_�[�̃f�[�^��ݒ�-----//
    SetEnemyGropeHostData();

    //--------------------<�G�̍X�V����>--------------------//
    fEnemiesUpdate(graphics_, elapsedTime_);

    //-----�G�̃f�[�^��ݒ�,���M-----//
    fCheckSendEnemyData(elapsedTime_);

    //--------------------<�G���m�̓����蔻��>--------------------//
    fCollisionEnemyVsEnemy();

    //--------------------<�G�̃X�|�i�[>--------------------//
    if (CorrespondenceManager::Instance().GetMultiPlay())
    {
        //-----�}���`�v���C���̓z�X�g�����G�̏o�����������Ȃ�-----//
        if (CorrespondenceManager::Instance().GetHost())fSpawn(graphics_);
    }

    // ImGui�̃��j���[
    fGuiMenu(graphics_, Func_);

    //--------------------<�{�X���G����������>--------------------//
    fCreateBossUnit(graphics_);

    bool isCreate{};
    for (const auto& source : mReserveVec)
    {
        isCreate = true;
        fSpawn(source, graphics_);
    }
    if (isCreate)
    {
        mReserveVec.clear();
    }

    // �U�R�I������S��������
    if (mIsReserveDelete)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetIsBoss() == false)
            {
                enemy->fDie(graphics_);
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }

}

void EnemyManager::fClientUpdate(GraphicsPipeline& graphics_, float elapsedTime_,AddBulletFunc Func_, EnemyAllDataStruct& receive_data)
{
    //--------------------<�Ǘ��N���X���̂̍X�V����>--------------------//
    mSloeTime -= elapsedTime_;

    // �E�F�[�u�J�n����̎��Ԃ��X�V
    mDelay-=elapsedTime_;
    if (!mDebugMode && !mIsTutorial && mDelay <= 0.0f)
    {
        //--------------------<�G�����܂肷������>--------------------//
        if (mEnemyVec.size() < 30)
        {
            mWaveTimer += elapsedTime_;
        }
    }

    // �J�����V�F�C�N
    if(mCameraShakeTime>0.0f)
    {
        if (GameFile::get_instance().get_shake()) camera_shake->shake(graphics_, elapsedTime_);
    }
    mCameraShakeTime -= elapsedTime_;
    mCameraShakeTime = (std::max)(0.0f, mCameraShakeTime);

    //--------------<�v���C���[���`�F�C�����̓G�l�~�[�̍s�������ׂĒ�~������>-------------//
    if(mIsPlayerChainTime)
    {
        for (const auto enemy : mEnemyVec)
        {
            //-----�̗͂�0�̎��Ɏ��S���������ʂ�-----//
            if (enemy->fGetCurrentHitPoint() > 0) continue;

            enemy->fDie(graphics_);


            if (enemy->fGetIsAlive() == false)
            {
                mRemoveVec.emplace_back(enemy);
            }
        }
        return;
    }

    //-----��M�����G�̃f�[�^��ݒ肷��-----//
    for (const auto& data : receive_data.enemy_die_data)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetObjectId() == data.object_id)
            {
                enemy->fDie(graphics_);
            }
        }
    }

    //----��ԃf�[�^��ݒ�-----//
    for (const auto& data : receive_data.enemy_condition_data)
    {
        fSetReceiveConditionData(data);
    }

    //-----�ړ��f�[�^��ݒ�-----//
    for (const auto& all_data : receive_data.enemy_move_data)
    {
        for (const auto& e_data : all_data.enemy_data)
        {
            fSetReceiveEnemyData(elapsedTime_,e_data,graphics_);
        }
    }

    //-----�G�̃��[�_�[�̃f�[�^��ݒ�-----//
    SetEnemyGropeHostData();

    //--------------------<�G�̍X�V����>--------------------//
    fEnemiesUpdate(graphics_,elapsedTime_);

    //--------------------<�G���m�̓����蔻��>--------------------//
    fCollisionEnemyVsEnemy();

    //--------------------<�G�̃X�|�i�[>--------------------//
    for (auto data : receive_data.enemy_spawn_data)
    {
        fSpawn(data, graphics_);
    }

    // ImGui�̃��j���[
    fGuiMenu(graphics_,Func_);


    bool isCreate{};
    for (const auto& source : mReserveVec)
    {
        isCreate = true;
        fSpawn(source, graphics_);
    }
    if(isCreate)
    {
        mReserveVec.clear();
    }

    // �U�R�I������S��������
    if(mIsReserveDelete)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetIsBoss() == false)
            {
                enemy->fDie(graphics_);
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }
}

void EnemyManager::fCheckSendEnemyData(float elapsedTime_)
{
    if (CorrespondenceManager::Instance().GetMultiPlay() == false) return;

    //------AI�̒����ł̃f�[�^���M-----//
    if (fGetIsEventCamera())
    {
        check_boss_ai_timer += elapsedTime_ * 1.0f;
    }

    //-----���Ԃ��擾-----//
    static auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();

    //-----�X�^�[�g�ƃG���h�̍������o��-----//
    auto dur = end - start;

    //-----�~���b�ɕϊ�����----//
    milliseconds = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());


    float value{};
    if (milliseconds > EnemyDataFrequency)
    {
        auto grah_start = std::chrono::system_clock::now();
        //-----�f�[�^�𑗂�-----//
        fSendEnemyData(elapsedTime_);

        //-----�^�C�}�[��������-----//
        start = std::chrono::system_clock::now();
        auto grah_end = std::chrono::system_clock::now();
        auto grah_dur = grah_end - grah_start;
        value = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(grah_dur).count());
    }

}

void EnemyManager::fSendEnemyData(float elapsedTime_)
{
    using namespace EnemySendData;

    if (fGetIsEventCamera())
    {
        if (check_boss_ai_timer < CheckBossAiTime) return;
        else
        {
            check_boss_ai_timer = 0.0f;
        }
    }


    char data[512]{};
    //-----�R�}���h��ݒ肷��-----//
    data[ComLocation::ComList] = CommandList::Update;
    data[ComLocation::UpdateCom] = UpdateCommand::EnemiesMoveCommand;

    int data_set_count = 0;

    EnemySendData::EnemyData enemy_d;

    //-----�}�X�^�[�̃f�[�^�𑗐M����-----//
    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fGetMaster() == false) continue;

        //-----�I�u�W�F�N�g�ԍ��ݒ�-----//
        enemy_d.enemy_data[EnemyDataArray::ObjectId] = enemy->fGetObjectId();

        //-----AI�̃X�e�[�g�ݒ�-----//
        enemy_d.enemy_data[EnemyDataArray::AiState] = enemy->fGetEnemyAiState();

        //-----�^�[�Q�b�g���Ă���v���C���[��Id-----//
        enemy_d.enemy_data[EnemyDataArray::TargetId] = enemy->fGetTargetPlayerId();

        //-----�G�̃^�C�v-----//
        enemy_d.enemy_data[EnemyDataArray::EnemyTypeId] = static_cast<char>(enemy->GetEnemyType());

        //-----�̗�-----//
        enemy_d.hp = static_cast<int>(enemy->fGetCurrentHitPoint());

        //-----�����̈ʒu��ݒ�-----//
        enemy_d.pos.x = static_cast<int16_t>(enemy->fGetPosition().x);
        enemy_d.pos.y = static_cast<int16_t>(enemy->fGetPosition().y);
        enemy_d.pos.z = static_cast<int16_t>(enemy->fGetPosition().z);

        std::memcpy(data + SendEnemyDataComSize + (sizeof(EnemyData) * data_set_count), (char*)&enemy_d,sizeof(EnemyData));

        data_set_count++;
    }

    //-----����f�[�^�������Ƃ��͂����ŏI���-----//
    if (data_set_count <= 0) return;

    //-----�f�[�^�T�C�Y��ݒ�-----//
    data[ComLocation::Other] = data_set_count;

    int size = SendEnemyDataComSize + (sizeof(EnemyData) * data_set_count);

    CorrespondenceManager::Instance().MultiCastSend(data,size);

}

void EnemyManager::fSendEnemyDamage(int obj_id, int damage)
{
    EnemySendData::EnemyDamageData d;
    d.data[ComLocation::ComList] = CommandList::Update;
    d.data[ComLocation::UpdateCom] = UpdateCommand::EnemyDamageCommand;
    d.data[EnemySendData::EnemyDamageCmdArray::DamageComEnemyId] = obj_id;
    d.data[EnemySendData::EnemyDamageCmdArray::DamageComDamage] = damage;

    //-----�z�X�g�Ƀ_���[�W�f�[�^�𑗐M-----//
    CorrespondenceManager::Instance().UdpSend(CorrespondenceManager::Instance().GetHostId(),(char*)&d, sizeof(EnemySendData::EnemyDamageData));
}

void EnemyManager::fSetReceiveEnemyData(float elapsedTime_, EnemySendData::EnemyData data, GraphicsPipeline& graphics_)
{

    for (const auto& enemy : mEnemyVec)
    {
        //-----�����̃I�u�W�F�N�g�ԍ��ƃf�[�^�̔ԍ����Ⴄ�Ȃ�Ƃ΂�-----//
        if (enemy->fGetObjectId() != data.enemy_data[EnemySendData::EnemyDataArray::ObjectId]) continue;

        DirectX::XMFLOAT3 pos{};
        pos.x = static_cast<int16_t>(data.pos.x);
        pos.y = static_cast<int16_t>(data.pos.y);
        pos.z = static_cast<int16_t>(data.pos.z);
        //-----�����̈ʒu��ݒ�-----//
        enemy->fSetReceivePosition(pos);

        //-----AI�X�e�[�g�ݒ�-----//
        enemy->fSetEnemyState(data.enemy_data[EnemySendData::EnemyDataArray::AiState]);

        //-----�^�[�Q�b�g�̈ʒu��ݒ�-----//
        enemy->fSetTargetPlayerId(data.enemy_data[EnemySendData::EnemyDataArray::TargetId]);

        //-----�̗͐ݒ�------//
        enemy->fSetCurrentHitPoint(data.hp);
        break;
    }

}


void EnemyManager::fSetReceiveConditionData(EnemySendData::EnemyConditionData data)
{
    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fGetObjectId() == data.data[EnemySendData::EnemyConditionArray::EnemyConditionObjectId])
        {
            switch (data.data[EnemySendData::EnemyConditionArray::EnemyCondition])
            {
                //-----�X�^���ɂȂ��Ă��Ȃ��Ȃ�X�^��������-----//
            case EnemySendData::EnemyConditionEnum::Stun:
                enemy->fSetStun(true);
                break;
            default:
                break;
            }

        }
    }
}


void EnemyManager::SetReciveDamageData(int obj_id, int damage, GraphicsPipeline& graphics_)
{
    for (const auto& enemy : mEnemyVec)
    {
        //-----ID���Ⴄ�Ȃ�Ƃ΂�-----//
        if (enemy->fGetObjectId() != obj_id) continue;

            //----�_���[�W��ݒ肷��-----//
        enemy->fDamaged(damage, 0.0f, graphics_, 0.0f);
    }
}

void EnemyManager::fRender(GraphicsPipeline& graphics_)
{
    //--------------------<�G�̕`�揈��>--------------------//
    fEnemiesRender(graphics_);
}

void EnemyManager::fFinalize()
{
    fAllClear();
    fDeleteCash();
}

bool EnemyManager::fGetSlow() const
{
    return mSloeTime > 0.0f;
}

int EnemyManager::fCalcPlayerAttackVsEnemies(DirectX::XMFLOAT3 PlayerCapsulePointA_,
                                             DirectX::XMFLOAT3 PlayerCapsulePointB_, float PlayerCapsuleRadius_, int PlayerAttackPower_,
                                             GraphicsPipeline& Graphics_,float elapsedTime_,bool& is_shield)
{
    //--------------------<�v���C���[�̍U���ƓG�̓����蔻��>--------------------//
      // �U�������̂̓G�ɓ���������
    int  hitCounts = 0;

    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fGetAppears() == false) continue;
        // �����蔻������邩�m�F
        if (enemy->fComputeAndGetIntoCamera())
        {
            Capsule capsule = enemy->fGetBodyCapsule();

            const bool result = Collision::capsule_vs_capsule(
                PlayerCapsulePointA_, PlayerCapsulePointB_, PlayerCapsuleRadius_,
                capsule.mTop, capsule.mBottom, capsule.mRadius);

            // �������Ă�����
            if (result)
            {
                if(enemy->fDamaged(PlayerAttackPower_, EnemyInvincibleTime,Graphics_,elapsedTime_))
                {
                    audio_manager->play_se(SE_INDEX::ATTACK_SWORD);

                    //�U����h���ꂽ�瑦���^�[��
                    hitCounts++;
                    mSloeTime = 0.1f;

                    //-----�}���`�v���C�̎����z�X�g�ł͖����Ƃ�-----//
                    if (CorrespondenceManager::Instance().GetMultiPlay() &&
                        CorrespondenceManager::Instance().GetHost() == false)
                    {
                        //-----�_���[�W�̃f�[�^��ݒ肷��-----//
                        fSendEnemyDamage(enemy->fGetObjectId(), PlayerAttackPower_);
                    }
                }
                else
                {
                    if (enemy->mIsSuccesGuard)
                    {
                        is_shield = true;
                        return hitCounts;
                    }
                }
            }
        }
    }

    return hitCounts;
}

bool EnemyManager::fCalcEnemiesAttackVsPlayer(DirectX::XMFLOAT3 PlayerCapsulePointA_,
    DirectX::XMFLOAT3 PlayerCapsulePointB_, float PlayerCapsuleRadius_, AddDamageFunc Func_)
{
    //--------------------<�v���C���[�ƓG�̍U���̓����蔻��>--------------------//

    for (const auto enemy : mEnemyVec)
    {
        // �����蔻������邩�m�F
        if (enemy->fComputeAndGetIntoCamera())
        {
            if (enemy->fGetAttack())
            {
                Capsule capsule = enemy->fGetAttackCapsule();

                const bool result = Collision::capsule_vs_capsule(
                    PlayerCapsulePointA_, PlayerCapsulePointB_, PlayerCapsuleRadius_,
                    capsule.mTop, capsule.mBottom, capsule.mRadius);

                // �������Ă�����
                if (result)
                {
                    Func_(enemy->fGetAttackPower(), enemy->fGetAttackInvTime());
                    return  true;
                }
            }
        }
    }
    return false;
}

bool EnemyManager::fCalcEnemiesAttackVsPlayerCounter(DirectX::XMFLOAT3 PlayerCapsulePointA_,
    DirectX::XMFLOAT3 PlayerCapsulePointB_, float PlayerCapsuleRadius_)
{
    //--------------------<�v���C���[�ƓG�̍U���̓����蔻��>--------------------//

    for (const auto enemy : mEnemyVec)
    {
        // �����蔻������邩�m�F
        if (enemy->fComputeAndGetIntoCamera())
        {
            //-----�U������ɓ����Ă�����-----//
            if (enemy->fGetAttackOperation())
            {
                Capsule capsule = enemy->fGetAttackCapsule();

                const bool result = Collision::capsule_vs_capsule(
                    PlayerCapsulePointA_, PlayerCapsulePointB_, PlayerCapsuleRadius_,
                    capsule.mTop, capsule.mBottom, capsule.mRadius);

                // �������Ă�����
                if (result)
                {
                    return  true;
                }
            }
        }
    }
    return false;
}

void EnemyManager::fCalcPlayerStunVsEnemyBody(const DirectX::XMFLOAT3 PlayerPosition_, float Radius_)
{

    if(Radius_<=0.0f)
    {
        return;
    }
    // ��������Ńv���C���[����X�^�������������󂯂�
    for(const auto & enemy: mEnemyVec)
    {
        //-----�o�����Ă��Ȃ������画�肵�Ȃ�-----//
        if (enemy->fGetAppears() == false) continue;
        const DirectX::XMFLOAT3 enemyPosition = enemy->fGetPosition();
        const float enemyRad = enemy->fGetBodyCapsule().mRadius;
        for (const auto p : mPlayerPosition)
        {
            if (Math::Length(p - enemyPosition) <= Radius_ + enemyRad)
            {
                enemy->fSetStun(true);
            }
        }
    }
}

BaseEnemy* EnemyManager::fGetNearestStunEnemy()
{
    BaseEnemy* pEnemy{ nullptr };
    float lengthVsPlayer{ FLT_MAX };
    for(int i=0;i<mEnemyVec.size();++i)
    {
        if(mEnemyVec[i]->fGetStun())
        {
            if(lengthVsPlayer>mEnemyVec[i]->fGetLengthFromPlayer())
            {
                lengthVsPlayer = mEnemyVec[i]->fGetLengthFromPlayer();
                pEnemy = mEnemyVec[i];
            }
        }
    }
    return pEnemy;

}

void EnemyManager::fAddRushBoss(BossRushUnit* enemy)
{
    mEnemyVec.emplace_back(enemy);
}

BaseEnemy* EnemyManager::fGetNearestEnemyPosition()
{
    auto func = [](const BaseEnemy* A_, const BaseEnemy* B_)->bool
    {
        return A_->fGetLengthFromPlayer() < B_->fGetLengthFromPlayer();
    };

    fSort(func);

    for(const auto enemy :mEnemyVec)
    {
        if (enemy->fGetStun() || enemy->fGetAppears() == false || enemy->fGetIsAlive() == false) continue;


        if (enemy->fComputeAndGetIntoCamera())
        {
            //// ���̓G����̋������v�Z����
            //for(const auto enemy2:mEnemyVec)
            //{
            //    if (enemy2->fComputeAndGetIntoCamera())
            //    {
            //        if (enemy != enemy2)
            //        {
            //            //enemy2->fCalcNearestEnemy(enemy->fGetPosition());
            //        }
            //    }
            //}
            return enemy;
        }
    }

    return nullptr;
}

BaseEnemy* EnemyManager::fGetSecondEnemyPosition()
{
    auto func = [](const BaseEnemy* A_, const BaseEnemy* B_)->bool
    {
        return A_->fGetLengthFromPlayer() < B_->fGetLengthFromPlayer();
    };
    fSort(func);
    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fComputeAndGetIntoCamera())
        {
            // ���̓G����̋������v�Z����
            for (const auto enemy2 : mEnemyVec)
            {
                if (enemy2->fComputeAndGetIntoCamera())
                {
                    if (enemy != enemy2)
                    {
                        //enemy2->fCalcNearestEnemy(enemy->fGetPosition());
                    }
                }
            }
            return enemy;
        }
    }

    return nullptr;
}

bool EnemyManager::fGetClearWave() const
{
    // �c��f�[�^���O���t�B�[���h�ɓG���c���Ă��Ȃ�
    return (mCurrentWaveVec.size() <= 0 && mEnemyVec.size() <= 0);
}



void EnemyManager::fSetIsPlayerChainTime(bool IsChain_)
{
    mIsPlayerChainTime = IsChain_;
}

void EnemyManager::fSetPlayerPosition(std::vector<std::tuple<int, DirectX::XMFLOAT3>> Position_)
{

    //std::tuple<object_id,position>

    //-----���ɓ����Ă���f�[�^���폜-----//
    mPlayerPosition.clear();

    //-----��ԋ߂��v���C���[�̈ʒu-----//
    DirectX::XMFLOAT3 near_pos{};

    //-----��ԋ߂��v���C���[�̈ʒu�Ƃ̋���-----//
    float near_length{ FLT_MAX };

    int id{ -1 };
    //-----�v�Z��������������-----//
    float length{};

    for(const auto& enemy:mEnemyVec)
    {
        near_pos = {};
        near_length = FLT_MAX;
        for (const auto p : Position_)
        {
            length = Math::calc_vector_AtoB_length(enemy->fGetPosition(), std::get<1>(p));

            //-----�ʒu��z��ɕۑ�-----//
            mPlayerPosition.emplace_back(std::get<1>(p));

            //-----��ԋ߂������ƍ��̓G�̈ʒu�Ƃ̋������r���ď�����������l���X�V-----//
            if (near_length > length)
            {
                near_length = length;

                //-----�ʒu��ۑ�-----//
                near_pos = std::get<1>(p);
                //-----id��ۑ�------//
                id = std::get<0>(p);
            }
        }
        enemy->fSetPlayerPosition(near_pos);
        enemy->fSetTargetPlayerId(id);
    }


}


void EnemyManager::fSetPlayerIdPosition(const std::vector<std::shared_ptr<BasePlayer>>& players)
{
    for (const auto& enemy : mEnemyVec)
    {
        for (const auto player : players)
        {
            //-----�^�[�Q�b�g��ID�ƈႤ�Ȃ�Ƃ΂�-----//
            if (enemy->fGetTargetPlayerId() != player->GetObjectId()) continue;

            //-----�ʒu��ݒ肷��-----//
            enemy->fSetPlayerPosition(player->GetPosition());
            break;
        }
    }
}

void EnemyManager::fSetPlayerSearch(bool Arg_)
{
    for (const auto& enemy : mEnemyVec)
    {
        enemy->fSetIsPlayerSearch(Arg_);
    }
}

void EnemyManager::fSetIsTutorial(bool Arg_)
{
    mIsTutorial = Arg_;
}


void EnemyManager::fSpawn(GraphicsPipeline& graphics)
{

    int spawnCounts = 0;

    std::map<int, EnemySource> s_data;
    // �G���X�|�[������֐�
    for (const auto data : mCurrentWaveVec)
    {
        // �o�������𖞂����Ă�����o��
        if (data.mSpawnTimer <= mWaveTimer)
        {
            s_data.insert(std::make_pair(object_count,data));
            fSpawn(data, graphics);
            spawnCounts++;
        }
    }

    if (s_data.empty() == false)
    {
        fSendSpawnData(s_data);
    }



    // �ǉ�������擪�̃f�[�^������
    for (int i = 0; i < spawnCounts; i++)
    {
        mCurrentWaveVec.erase(mCurrentWaveVec.begin());
    }
}

void EnemyManager::fSpawn(EnemySource Source_, GraphicsPipeline& graphics_)
{
    // �����Ă����f�[�^�����ƂɓG���o��������
    const auto param = mEditor.fGetParam(Source_.mType);


    BaseEnemy* enemy = nullptr;

    //-----switch���̒�-----//
    //���Ԑ���
    //ID�ݒ�
    //��ސݒ�
    //�O���[�v�f�[�^�ݒ�

    switch (Source_.mType)
    {
    case EnemyType::Archer:
    {
        enemy = new ArcherEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Archer);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Shield:
    {
        enemy = new ShieldEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Shield);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Sword:
    {
        enemy = new SwordEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Sword);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Spear:
    {
        enemy = new SpearEnemy(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Spear);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Archer_Ace:
    {
        enemy = new ArcherEnemy_Ace(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Archer_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Shield_Ace:
    {
        enemy = new ShieldEnemy_Ace(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Shield_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Sword_Ace:
    {
        enemy = new SwordEnemy_Ace(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Sword_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Spear_Ace:
    {
        enemy = new SpearEnemy_Ace(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Spear_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Boss:
    {
        enemy = new LastBoss(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType), this);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Boss);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Count: break;
    case EnemyType::Tutorial_NoMove:
    {
        enemy = new TutorialEnemy_NoAttack(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Boss_Unit:
        break;
        //-----�G�̃^�O�������Ƃ��͂����Ń��^�[������-----//
    default:
        return;
        break;
    }



    //-----�G��vector�ɓ����-----//
    if (enemy != nullptr)mEnemyVec.emplace_back(enemy);

    object_count++;
}

void EnemyManager::fSpawn(EnemySendData::EnemySpawnData data, GraphicsPipeline& graphics_)
{
    //-----��M�����f�[�^����G�̎�ނ��擾-----//
    EnemyType type = static_cast<EnemyType>(data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyType]);

    //-----�ԍ����擾-----//
    int id = data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyId];

    // �����Ă����f�[�^�����ƂɓG���o��������
    const auto param = mEditor.fGetParam(type);

    //-----�}�X�^�[���ǂ���-----//
    bool master = static_cast<bool>(data.cmd[EnemySendData::EnemySpawnCmdArray::Master]);

    //-----�O���[�v�ԍ�-----//
    int grope_id = static_cast<int>(data.cmd[EnemySendData::EnemySpawnCmdArray::GropeId]);

    //-----�z�X�g���n�D��x-----//
    int transfer = static_cast<int>(data.grope_data);

    DirectX::XMFLOAT3 emit_pos{};
    emit_pos.x = static_cast<float>(data.emitter_point.x);
    emit_pos.z = static_cast<float>(data.emitter_point.y);

    switch (type)
    {
    case EnemyType::Archer:
    {
        BaseEnemy* enemy = new ArcherEnemy(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Archer);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Shield:
    {
        BaseEnemy* enemy = new ShieldEnemy(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Shield);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Sword:
    {
        BaseEnemy* enemy = new SwordEnemy(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Sword);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear:
    {
        BaseEnemy* enemy = new SpearEnemy(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Spear);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Archer_Ace:
    {
        BaseEnemy* enemy = new ArcherEnemy_Ace(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Archer_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Shield_Ace:
    {
        BaseEnemy* enemy = new ShieldEnemy_Ace(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Shield_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Sword_Ace:
    {
        BaseEnemy* enemy = new SwordEnemy_Ace(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Sword_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear_Ace:
    {
        BaseEnemy* enemy = new SpearEnemy_Ace(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Spear_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Boss:
    {
        BaseEnemy* enemy = new LastBoss(graphics_,
            emit_pos,
            param, this);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Boss);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Count: break;
    case EnemyType::Tutorial_NoMove:
    {
        BaseEnemy* enemy = new TutorialEnemy_NoAttack(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        mEnemyVec.emplace_back(enemy);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
    }
    break;
    case EnemyType::Boss_Unit:
    {
        BaseEnemy* enemy = new BossUnit(graphics_,
            emit_pos,
            param,
            BulletManager::Instance().fGetAddFunction());
        enemy->fSetObjectId(id);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
        std::string text = "�G���j�b�g����" + std::to_string(id);
        DebugConsole::Instance().WriteDebugConsole(text,TextColor::Pink);
    }
    break;
    default:
        break;
    }

}


void EnemyManager::fSendSpawnData(std::map<int, EnemySource> spawn_data)
{
    if (CorrespondenceManager::Instance().GetMultiPlay() == false) return;

    using namespace EnemySendData;

    int data_set_count = 0;

    char data[512]{};
    //-----�R�}���h��ݒ肷��-----//
    data[ComLocation::ComList] = CommandList::EnemySpawnCommand;


    EnemySpawnData d;

    for (const auto s_data : spawn_data)
    {
        //-----�R�}���h��ݒ�-----//
        d.cmd[EnemySpawnCmdArray::Master] = s_data.second.master;
        d.cmd[EnemySpawnCmdArray::GropeId] = s_data.second.grope_id;
        d.cmd[EnemySpawnCmdArray::EnemyId] = s_data.first;
        d.cmd[EnemySpawnCmdArray::EnemyType] = static_cast<int>(s_data.second.mType);

        //-----�o���ʒu-----//
        d.emitter_point.x = static_cast<int16_t>(s_data.second.mEmitterPoint.x);
        d.emitter_point.y = static_cast<int16_t>(s_data.second.mEmitterPoint.z);

        //-----�}�X�^�[�̏��n��-----//
        d.grope_data = s_data.second.transfer_host;

        std::memcpy(data + SendSpawnEnemyDataComSize + (sizeof(EnemySpawnData) * data_set_count), (char*)&d, sizeof(EnemySpawnData));

        data_set_count++;
    }

    //-----����f�[�^�������Ƃ��͂����ŏI���-----//
    if (data_set_count <= 0) return;

    //-----�f�[�^�T�C�Y��ݒ�-----//
    data[static_cast<int>(SendEnemySpawnData::SpawnNum)] = data_set_count;

    int size = SendSpawnEnemyDataComSize + (sizeof(EnemySpawnData) * data_set_count);


    CorrespondenceManager::Instance().TcpSendAllClient(data,size);

#if 0
    //-----�R�}���h��ݒ�-----//
    data.cmd[ComLocation::ComList] = CommandList::EnemySpawnCommand;

    //-----�G��ID-----//
    data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyId] = object_count;

    //-----�G�̃^�C�v-----//
    data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyType] = static_cast<int>(Source_.mType);

    //-----�o���ʒu-----//
    data.emitter_point.x = static_cast<int16_t>(Source_.mEmitterPoint.x);
    data.emitter_point.y = static_cast<int16_t>(Source_.mEmitterPoint.y);
    data.emitter_point.z = static_cast<int16_t>(Source_.mEmitterPoint.z);

    //-----�O���[�v�f�[�^��ݒ�-----//

    //-----�}�X�^�[���ǂ���-----//
    data.grope_data[EnemySendData::EnemySpawnGropeArray::Master] = Source_.master;

    //-----�O���[�v�̔ԍ�-----//
    data.grope_data[EnemySendData::EnemySpawnGropeArray::GropeId] = Source_.grope_id;

    //-----�}�X�^�[�̏��n��-----//
    data.grope_data[EnemySendData::EnemySpawnGropeArray::Transfer] = Source_.transfer_host;


    CorrespondenceManager::Instance().TcpSendAllClient((char*)&data, sizeof(EnemySendData::EnemySpawnData));

#endif // 0

    DebugConsole::Instance().WriteDebugConsole("�G�o���f�[�^���M : " +std::to_string(size) + "�o�C�g" , TextColor::Pink);
}

void EnemyManager::fEnemiesUpdate(GraphicsPipeline& Graphics_,float elapsedTime_)
{
    // �X�V
    for (const auto enemy : mEnemyVec)
    {
        //-----���[�_�[�̃f�[�^���擾-----//
        if (enemy->fGetMaster() == false)
        {
            //-----�\����������tuple�̃f�[�^���擾-----//
            auto [check, data] = master_enemy_data->GetMasterData(enemy->fGetGropeId());
            //-----true�Ȃ烊�[�_�[�����݂��ăf�[�^���擾���邱�Ƃ��ł���-----//
            if (check) enemy->fSetMasterData(data.position, data.ai_state, data.target_id);

        }

        //-----�G�̍X�V-----//
        enemy->fUpdate(Graphics_, elapsedTime_);

        //-----����ł�����폜�p��vector�ɓo�^-----//
        if (enemy->fGetIsAlive() == false)
        {
            mRemoveVec.emplace_back(enemy);
            // ����ł���G��������U�鎞�Ԃ����Z
            mCameraShakeTime += mkOneShakeSec;
        }
    }


}

void EnemyManager::fEnemiesRender(GraphicsPipeline& graphics_)
{
    for (const auto enemy : mEnemyVec)
    {
        //-----�{�X���b�V���U���̃��f�����������ɍU���̃t���O��true����Ȃ�������Ƃ΂�-----//
        if (enemy->GetEnemyType() == EnemyType::BossRush && start_boss_rush == false) continue;
        enemy->fRender(graphics_);
    }
}

void EnemyManager::fReserveBossUnit(std::vector<DirectX::XMFLOAT3> Vec_)
{
    if(mIsReserveBossUnit==false)
    {
        mIsReserveBossUnit = true;
        mUnitEntryPointVec = Vec_;
    }
}

void EnemyManager::fCreateRandomMasterEnemy(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 SeedPosition_, int grope_id)
{
    if (mEnemyVec.size() > 15)
    {
        return;
    }

    // �����œG�̃^�C�v���擾
    std::mt19937 mt{ std::random_device{}() };
    std::uniform_int_distribution<int> RandTargetAdd(0, 7);
    int randNumber = RandTargetAdd(mt);

    // �`���[�g���A���̓G�Ȃ�Ⴄ�G���o��
    if (randNumber == 9 || randNumber == 8)
    {
        randNumber = 10;
    }

    EnemySource source;

    std::uniform_int_distribution<int> RandTargetAdd2(-5, 5);
    const int randPosition = RandTargetAdd2(mt);
    const int randPositionX = RandTargetAdd2(mt);
    const int randPositionY = RandTargetAdd2(mt);
    source.mEmitterPoint =
    {
        SeedPosition_.x +
        (static_cast<float>(randPosition) * static_cast<float>(randPositionX)),
        0.0f,
        SeedPosition_.z +
        static_cast<float>(randPosition) * static_cast<float>(randPositionY),
    };

    source.mType = static_cast<EnemyType>(randNumber);
    source.master = true;
    source.grope_id = grope_id;
    source.transfer_host = 0;
    mReserveVec.emplace_back(source);
}

void EnemyManager::fCreateRandomEnemy(
    GraphicsPipeline& Graphics_,
    DirectX::XMFLOAT3 SeedPosition_, int grope_id, int transfer_id)
{
    if(mEnemyVec.size()>15)
    {
        return;
    }

    // �����œG�̃^�C�v���擾
    std::mt19937 mt{ std::random_device{}() };
    std::uniform_int_distribution<int> RandTargetAdd(0, 7);
    int randNumber = RandTargetAdd(mt);

    // �`���[�g���A���̓G�Ȃ�Ⴄ�G���o��
    if (randNumber == 9 || randNumber == 8)
    {
        randNumber = 10;
    }

    EnemySource source;

    std::uniform_int_distribution<int> RandTargetAdd2(-5, 5);
    const int randPosition = RandTargetAdd2(mt);
    const int randPositionX = RandTargetAdd2(mt);
    const int randPositionY = RandTargetAdd2(mt);
    source.mEmitterPoint =
    {
        SeedPosition_.x+
        (static_cast<float>(randPosition)* static_cast<float>(randPositionX)),
        0.0f,
        SeedPosition_.z+
        static_cast<float>(randPosition)* static_cast<float>(randPositionY),
    };

    source.mType = static_cast<EnemyType>(randNumber);
    source.master = false;
    source.grope_id = grope_id;
    source.transfer_host = transfer_id;
    mReserveVec.emplace_back(source);
}

void EnemyManager::fSort(std::function<bool(const BaseEnemy* A_, const BaseEnemy* B_)> Function_)
{
    // �v���C���[�Ƃ̋������ɓG���\�[�g
    std::sort(mEnemyVec.begin(), mEnemyVec.end(), Function_);
}


void EnemyManager::fAllClear()
{
    //--------------------<�v�f��S�폜>--------------------//
    for (auto enemy : mEnemyVec)
    {
        enemy->StopEffec();
        // ���݂��Ă���΍폜
        if (enemy)
        {
            delete enemy;
            enemy = nullptr;
        }
    }
    mEnemyVec.clear();

    //-----�}�X�^�[�f�[�^�̍폜-----//
    master_enemy_data->ResetMasterData();
}

void EnemyManager::fCollisionEnemyVsEnemy()
{
    for(const auto enemy1 :mEnemyVec)
    {
        for (const auto enemy2 : mEnemyVec)
        {
            // �������g�Ƃ͔��肵�Ȃ�
            if (enemy1 == enemy2)
            {
                continue;
            }
             auto capsule1 = enemy1->fGetBodyCapsule();
             auto capsule2 = enemy2->fGetBodyCapsule();
            // �������a���O�ȉ��Ȃ�v�Z���Ȃ�
            if(capsule1.mRadius<=0.0f||capsule2.mRadius<=0.0f)
            {
                continue;
            }

            //--------------------<�G���m�̋������𒲐�����>--------------------//
            capsule1.mRadius *= mAdjustmentEnemies;
            capsule2.mRadius *= mAdjustmentEnemies;


            const bool result=Collision::capsule_vs_capsule(
                capsule1.mTop, capsule1.mBottom, capsule1.mRadius,
                capsule2.mTop, capsule2.mBottom, capsule2.mRadius);

            // ��������������
            if(result)
            {
                // ���������G�Ԃ̃x�N�g�����Z�o����
                DirectX::XMFLOAT3 vec = enemy2->fGetPosition() - enemy1->fGetPosition();
                if (Math::Length(vec) <= 0.0f)
                {
                    vec = { 0.0f,0.0f,1.0f };
                }
                // ���K��
                vec = Math::Normalize(vec);
                // ��̂̂߂荞��ł��鋗�����v�Z����

                // ��̂̔��a�̍��v
                const float radiusAdd = capsule1.mRadius + capsule2.mRadius;
                // ��̂̋���
                float length = Math::Length(
                    enemy1->fGetPosition() - enemy2->fGetPosition());
                if(length<=0.0f)
                {
                    length = 0.1f;
                }
                // �߂荞�݋���
                const float raidLength = radiusAdd - length;
                DirectX::XMFLOAT3 res = enemy2->fGetPosition() + (vec * raidLength);

            }

        }
    }
}

void EnemyManager::fLoad(const char* FileName_)
{
    // Json�t�@�C������l���擾
    std::filesystem::path path = FileName_;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);
            o_archive(mCurrentWaveVec);
        }
    }
}

void EnemyManager::fSave(const char* FileName_)
{
    // Json�t�@�C������l���擾
    std::filesystem::path path = FileName_;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ofstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONOutputArchive o_archive(ifs);
            o_archive(mCurrentWaveVec);
        }
    }
}

void EnemyManager::SetEnemyGropeHostData()
{
    for (const auto& ene : mEnemyVec)
    {
        //-----���[�_�[����Ȃ��Ȃ�Ƃ΂�-----//
        if (ene->fGetMaster() == false) continue;

        //-----���[�_�[���X�^�����Ă���ꍇ�̓z�X�g���������n����-----//
        if(ene->fGetStun()) TransferMaster(ene->fGetGropeId());

        //-----���[�_�[�̃f�[�^��ݒ肷��-----//
        master_enemy_data->SetMasterData(ene->fGetGropeId(),ene->fGetPosition(),ene->fGetTargetPlayerId(),ene->fGetEnemyAiState());
    }

}

void EnemyManager::TransferMaster(int grope)
{
    int transfer{ INT_MAX };
    BaseEnemy* e{ nullptr };

    for (const auto& enemy : mEnemyVec)
    {
        //-----�}�X�^�[�̏ꍇ���O���[�v�ԍ����Ⴄ�Ȃ�Ƃ΂�-----//
        if (enemy->fGetMaster() || enemy->fGetGropeId() != grope) continue;

        //-----���n�ԍ�����ԏ������G���擾-----//
        if (transfer > enemy->fGetTransfer())
        {
            transfer = enemy->fGetTransfer();
            e = enemy;
        }
    }

    //-----�����l�������Ă���Ȃ�}�X�^�[�ɏ��i����-----//
    if (e != nullptr)e->fSetMaster(true);
    //-----�l�������Ă��Ȃ��Ȃ���n�悪���Ȃ��̂ł��̃f�[�^�͏���-----//
    else master_enemy_data->DeleteSpecificData(grope);
}

void EnemyManager::DebugLoadEnemyParam()
{
    mEditor.HttpLoad();
}

void EnemyManager::fGuiMenu(GraphicsPipeline& Graphics_, AddBulletFunc Func_)
{
    imgui_menu_bar("Game", "EnemyManager", mOpenGuiMenu);


#ifdef USE_IMGUI
    if (mOpenGuiMenu)
    {
        ImGui::Begin("EnemyManager");

        int size = static_cast<int>(mEnemyVec.size());

        ImGui::Text("EnemyNum%d", size);

        ImGui::Text("WaveNumber");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mCurrentWave).c_str());
        ImGui::Text("WaveTimer");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mWaveTimer).c_str());

        ImGui::Separator();

        ImGui::Text("EnemyValues");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mEnemyVec.size()).c_str());
        ImGui::SameLine();
        ImGui::Text("DataSize");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mCurrentWaveVec.size()).c_str());
        ImGui::Separator();

        ImGui::Separator();
        static int elem = static_cast<int>(EnemyType::Sword);
        constexpr int count = static_cast<int>(EnemyType::Count);
        const char* elems_names[count] =

        {
            "Archer","Shield","Sword","Spear","Archer_Ace",
            "Shield_Ace","Sword_Ace","Spear_Ace",
            "Boss","TutorialNoMove","BossUnit"};

        const char* elem_name = (elem >= 0 && elem < count) ? elems_names[elem] : "Unknown";
        ImGui::SliderInt("slider enum", &elem, 0, count - 1, elem_name);

        if (ImGui::Button("CreateEnemy"))
        {
            EnemySource source{};
            source.mEmitterPoint = {};
            source.mType = static_cast<EnemyType>(elem);
            fSpawn(source,Graphics_);
        }

        ImGui::InputInt("WaveNumber", &mCurrentWave);
        if (ImGui::Button("StartWave"))
        {
            fStartWave(mCurrentWave);
        }

        ImGui::Separator();

        if (ImGui::Button("AllClear"))
        {
            fAllClear();
        }

        ImGui::Text("LastTimer");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mLastTimer).c_str());
        ImGui::RadioButton("ClearWave", fWaveClear());

        if (ImGui::Button("AddWaveTimer"))
        {
            mWaveTimer += 10.0f;
        }

        ImGui::Checkbox("DebugMode", &mDebugMode);

        if (ImGui::Button("Close"))
        {
            mOpenGuiMenu = false;
        }

        if(ImGui::Button("AllStun"))
        {
            for(const auto & enemy: mEnemyVec)
            {
                enemy->fSetStun(true);
            }
        }


        ImGui::InputFloat("SlowTime", &mSloeTime);

        ImGui::End();
    }
#endif
    mEditor.fGuiMenu();
}

void EnemyManager::fStartWave(int WaveIndex_)
{
    //--------------------<�E�F�[�u���J�n������֐�>--------------------//
    fAllClear();

    //-----�G�̃J�E���g������������-----//
    object_count = 0;

    mWaveTimer = 0.0f;
    fLoad(mWaveFileNameArray[WaveIndex_]);

    // ���b�ł��̃E�F�[�u���I��邩������������
    mLastTimer = mCurrentWaveVec.back().mSpawnTimer;
    mDelay = 3.0f;
}

bool EnemyManager::fWaveClear() const
{
    // �Ō�̓G���o�����Ă��邩�𔻒肷��
    if (mLastTimer >= mWaveTimer)
    {
        return false;
    }
    // ���ׂĂ̓G������ł�����
    if (mEnemyVec.size() > 0)
    {
        return false;
    }
    return true;
}

void EnemyManager::fDeleteEnemies()
{
    // �폜
    for (const auto enemy : mRemoveVec)
    {
        auto e = std::find(mEnemyVec.begin(), mEnemyVec.end(), enemy);
        if (e != mEnemyVec.end())
        {
            //-----�}�X�^�[�̏ꍇ���n����-----//
            if ((*e)->fGetMaster())
            {
                TransferMaster((*e)->fGetGropeId());
            }

            safe_delete(*e);
            mEnemyVec.erase(e);
        }
    }
    mRemoveVec.clear();
}

std::vector<BaseEnemy*> EnemyManager::fGetEnemies() const
{
    return mEnemyVec;
}

size_t EnemyManager::fGetEnemyCounts() const
{
    return mEnemyVec.size();
}

void EnemyManager::fRegisterCash(GraphicsPipeline& graphics_)
{
    // �L���b�V���Ƀ��f����o�^
    BaseEnemy* enemy = new SwordEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ArcherEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ShieldEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new SpearEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ArcherEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ShieldEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new SpearEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new SwordEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    //enemy = new BossUnit(graphics_);
    //mCashEnemyVec.emplace_back(enemy);

}

void EnemyManager::fDeleteCash()
{

    for (auto enemy : mCashEnemyVec)
    {
        // ���݂��Ă���΍폜
        if (enemy)
        {
            delete enemy;
            enemy = nullptr;
        }
    }
    mCashEnemyVec.clear();
}



void EnemyManager::fCreateBossUnit(GraphicsPipeline& Graphics_)
{
    if (mIsReserveBossUnit == false) return;

    int grope_id{ 1 };

    std::map<int, EnemySource> s_data;

    for(const auto unit:mUnitEntryPointVec)
    {
        //-----�}���`�v���C�̎��Ƀf�[�^�𑗐M-----//
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            EnemySource data;
            data.master = true;
            data.grope_id = grope_id;
            data.mEmitterPoint = unit;
            data.mType = EnemyType::Boss_Unit;

            s_data.insert(std::make_pair(object_count, data));
        }

        BaseEnemy* enemy = new BossUnit(Graphics_,
            unit,
            mEditor.fGetParam(EnemyType::Boss_Unit),
            BulletManager::Instance().fGetAddFunction());
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyGropeData(true, 0, grope_id);
        mEnemyVec.emplace_back(enemy);
        grope_id++;
        object_count++;
    }

    if (s_data.empty() == false)
    {
        fSendSpawnData(s_data);
    }

    mIsReserveBossUnit = false;
    mUnitEntryPointVec.clear();
}

void EnemyManager::fSpawnTutorial_NoAttack(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // �G�̐������ȉ�����Ȃ��ƃ_��
    constexpr int maxEnemies{ 7 };

    if (mEnemyVec.size() < maxEnemies)
    {

        mTutorialTimer += elapsedTime_;
        if (mTutorialTimer > mkSeparateTutorial)
        {
            std::mt19937 mt{ std::random_device{}() };
            std::uniform_int_distribution<int> RandTargetAdd(-5, 5);
            const int randNumber1 = RandTargetAdd(mt);
            const int randNumber2 = RandTargetAdd(mt);
            DirectX::XMFLOAT3 pos{ randNumber1 * 10.0f,0.0f,randNumber2 * 10.0f };

            mTutorialTimer = 0.0f;
            mEnemyVec.emplace_back(new TutorialEnemy_NoAttack(Graphics_, pos, mEditor.fGetParam(EnemyType::Tutorial_NoMove)));
        }
    }
}

void EnemyManager::fSpawnTutorial(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // �G�̐������ȉ�����Ȃ��ƃ_��
    constexpr int maxEnemies{ 7 };

    if (mEnemyVec.size() < maxEnemies)
    {

        mTutorialTimer += elapsedTime_;
        if (mTutorialTimer > mkSeparateTutorial)
        {
            std::mt19937 mt{ std::random_device{}() };
            std::uniform_int_distribution<int> RandTargetAdd(-5, 5);
            const int randNumber1 = RandTargetAdd(mt);
            const int randNumber2 = RandTargetAdd(mt);
            DirectX::XMFLOAT3 pos{ randNumber1 * 10.0f,0.0f,randNumber2 * 10.0f };

            mTutorialTimer = 0.0f;

            BaseEnemy* enemy = new TutorialEnemy(Graphics_, pos, mEditor.fGetParam(EnemyType::Tutorial_NoMove));
            mEnemyVec.emplace_back(enemy);
        }
    }
}

//****************************************************************
//
// �{�X�֘A�̊֐�
//
//****************************************************************
LastBoss::Mode EnemyManager::fGetBossMode() const
{
    return mCurrentMode;
}

void EnemyManager::fSetBossMode(LastBoss::Mode Mode_)
{
    mCurrentMode = Mode_;
}

bool EnemyManager::fGetIsEventCamera() const
{
    bool result{ false };
    switch (mCurrentMode) {
    case LastBoss::Mode::None:break;
    case LastBoss::Mode::Ship:break;
    case LastBoss::Mode::ShipToHuman:
        result = true;
        break;
    case LastBoss::Mode::Human: break;
    case LastBoss::Mode::HumanToDragon:
        result = true;
        break;
    case LastBoss::Mode::Dragon:break;
    case LastBoss::Mode::DragonDie:
        result = true;
        break;
    case LastBoss::Mode::BossDieEnd: break;
    case LastBoss::Mode::ShipAppear:
        result = true;
        break;
    default: ;
    }

    return result;
}

DirectX::XMFLOAT3 EnemyManager::fGetEye() const
{
    return mBossCameraEye;
}

DirectX::XMFLOAT3 EnemyManager::fGetFocus() const
{
    return mBossCameraFocus;
}

void EnemyManager::fSetBossEye(DirectX::XMFLOAT3 Eye_)
{
    mBossCameraEye = Eye_;
}

void EnemyManager::fSetBossFocus(DirectX::XMFLOAT3 Focus_)
{
    mBossCameraFocus = Focus_;
}

bool EnemyManager::fGetBossClear() const
{
    return mCurrentMode == LastBoss::Mode::BossDieEnd;
}

void EnemyManager::fReserveDeleteEnemies()
{
    mIsReserveDelete = true;
}

void EnemyManager::fLimitEnemies()
{
    for(auto enemy: mEnemyVec)
    {
        enemy->fLimitPosition();
    }
}

void EnemyManager::EndEnventCount(int count)
{
    end_event_count += count;
    //-----�ڑ������l���Ɠ����Ȃ�C�x���g��i�߂�-----//
    if (end_event_count == CorrespondenceManager::Instance().GetConnectedPersons())
    {

        for (auto enemy : mEnemyVec)
        {
            //-----�{�X����Ȃ�������Ƃ΂�-----//
            if (enemy->GetEnemyType() != EnemyType::Boss) continue;
            enemy->SetEndEvent(true);
            //----���ɔ����ăJ�E���g�����炷-----//
            end_event_count = 0;
            DebugConsole::Instance().WriteDebugConsole("�C�x���g���I��", TextColor::Blue);
            break;
        }

    }
}

void EnemyManager::EndEvent()
{
    for (auto enemy : mEnemyVec)
    {
        //-----�{�X����Ȃ�������Ƃ΂�-----//
        if (enemy->GetEnemyType() != EnemyType::Boss) continue;
        enemy->SetEndEvent(true);
        break;
    }
}
