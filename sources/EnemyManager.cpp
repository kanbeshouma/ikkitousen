#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"EnemyManager.h"
#include "EnemyFileSystem.h"

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

//****************************************************************
//
// �G�̊Ǘ��N���X
//
//****************************************************************

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
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }
}

void EnemyManager::fCheckSendEnemyData(float elapsedTime_)
{

    switch (send_enemy_type)
    {
    case SendEnemyType::Sword:
        //-----�f�[�^�𑗂�-----//
        fSendEnemyData(elapsedTime_, SendEnemyType::Sword);
        //-----���ɑ���f�[�^�̃X�e�[�g�ɐݒ�-----//
        send_enemy_type = SendEnemyType::Archer;
        break;
    case SendEnemyType::Archer:
        fSendEnemyData(elapsedTime_, SendEnemyType::Archer);
        //-----���ɑ���f�[�^�̃X�e�[�g�ɐݒ�-----//
        send_enemy_type = SendEnemyType::Spear;
        break;
    case SendEnemyType::Spear:
        fSendEnemyData(elapsedTime_, SendEnemyType::Spear);
        //-----���ɑ���f�[�^�̃X�e�[�g�ɐݒ�-----//
        send_enemy_type = SendEnemyType::Shield;
        break;
    case SendEnemyType::Shield:
        fSendEnemyData(elapsedTime_, SendEnemyType::Shield);
        //-----���ɑ���f�[�^�̃X�e�[�g�ɐݒ�-----//
        send_enemy_type = SendEnemyType::Sword;
        break;
    case SendEnemyType::Boss:
        fSendEnemyData(elapsedTime_, SendEnemyType::Boss);
        break;
    default:
        break;
    }

}

void EnemyManager::fSendEnemyData(float elapsedTime_, SendEnemyType type)
{
    using namespace EnemySendData;

    EnemiesMoveData send_data;

    //-----�R�}���h��ݒ肷��-----//
    send_data.cmd[ComLocation::ComList] = CommandList::Update;
    send_data.cmd[ComLocation::UpdateCom] = UpdateCommand::EnemiesMoveCommand;
    send_data.cmd[ComLocation::DataKind] = type;

    EnemySendData::EnemyData data;

    for (const auto enemy : mEnemyVec)
    {
        if (enemy->GetEnemyType() != type) continue;
        //-----�I�u�W�F�N�g�ԍ��ݒ�-----//
        data.enemy_data[EnemyDataArray::ObjectId] = enemy->fGetObjectId();

        //-----AI�̃X�e�[�g�ݒ�-----//
        data.enemy_data[EnemyDataArray::AiState] = enemy->fGetEnemyState();

        //-----�����̈ʒu��ݒ�-----//
        data.pos = enemy->fGetPosition();

        //-----�^�[�Q�b�g�̈ʒu�ݒ�-----//
        data.target_pos = enemy->GetTargetPosition();

        send_data.enemy_data.emplace_back(data);
    }

    //-----�f�[�^�T�C�Y��ݒ�-----//
    send_data.cmd[ComLocation::Other] = send_data.enemy_data.size();


    CorrespondenceManager::Instance().UdpSend((char*)&send_data, sizeof(send_data));

}

void EnemyManager::fSetReceiveEnemyData(float elapsedTime_, char type, EnemySendData::EnemyData data)
{
    using namespace EnemySendData;
    for (const auto& enemy : mEnemyVec)
    {
        //-----��M�����G�̎�ނƈႤ�Ȃ�Ƃ΂�-----//
        if (enemy->GetEnemyType() != static_cast<SendEnemyType>(type)) continue;

        //-----�����̃I�u�W�F�N�g�ԍ��ƃf�[�^�̔ԍ����Ⴄ�Ȃ�Ƃ΂�-----//
        if (enemy->fGetObjectId() != data.enemy_data[EnemyDataArray::ObjectId]) continue;

        //-----�����̈ʒu��ݒ�-----//
        enemy->fSetPosition(data.pos);

        //-----AI�X�e�[�g�ݒ�-----//
        enemy->fSetEnemyState(data.enemy_data[EnemyDataArray::AiState]);

        //-----�^�[�Q�b�g�̈ʒu��ݒ�-----//
        enemy->fSetPlayerPosition(data.target_pos);
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
    for (const auto& all_data : receive_data.enemy_move_data)
    {
        for (const auto& e_data : all_data.enemy_data)
        {
            fSetReceiveEnemyData(elapsedTime_,all_data.cmd[ComLocation::DataKind] ,e_data);
        }
    }
    //--------------------<�G�̍X�V����>--------------------//
    fEnemiesUpdate(graphics_,elapsedTime_);

    //--------------------<�G���m�̓����蔻��>--------------------//
    fCollisionEnemyVsEnemy();

    //--------------------<�G�̃X�|�i�[>--------------------//

    for (auto data : receive_data.enemy_spawn_data)
    {
        fSpawn(data,graphics_);
    }
    // ImGui�̃��j���[
    fGuiMenu(graphics_,Func_);

    //--------------------<�{�X���G����������>--------------------//
    fCreateBossUnit(graphics_);

    bool isCreate{};
    for (const auto& source : mReserveVec)
    {
        isCreate = true;
        for (auto data : receive_data.enemy_spawn_data)
        {
            fSpawn(data, graphics_);
        }
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
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
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
                if(enemy->fDamaged(PlayerAttackPower_, 0.1f,Graphics_,elapsedTime_))
                {
                    audio_manager->play_se(SE_INDEX::ATTACK_SWORD);

                    //�U����h���ꂽ�瑦���^�[��
                    hitCounts++;
                    mSloeTime = 0.1f;
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

void EnemyManager::fSetPlayerPosition(std::vector<DirectX::XMFLOAT3> Position_)
{
    mPlayerPosition = Position_;

    //-----��ԋ߂��v���C���[�̈ʒu-----//
    DirectX::XMFLOAT3 near_pos{};
    //-----��ԋ߂��v���C���[�̈ʒu�Ƃ̋���-----//
    float near_length{ 100000.0f };
    //-----�v�Z��������������-----//
    float length{};

    for(const auto& enemy:mEnemyVec)
    {
        near_pos = {};
        near_length = 100000.0f;
        for (const auto p : Position_)
        {
            length = Math::calc_vector_AtoB_length(enemy->fGetPosition(), p);
            //-----��ԋ߂������ƍ��̓G�̈ʒu�Ƃ̋������r���ď�����������l���X�V-----//
            if (near_length > length)
            {
                near_length = length;
                near_pos = p;
            }
        }
        enemy->fSetPlayerPosition(near_pos);
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

    // �G���X�|�[������֐�
    for (const auto data : mCurrentWaveVec)
    {
        // �o�������𖞂����Ă�����o��
        if (data.mSpawnTimer <= mWaveTimer)
        {
            fSpawn(data, graphics);
            spawnCounts++;
        }
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

    //-----�}���`�v���C�̎��Ƀf�[�^�𑗐M-----//
    fSendSpawnData(Source_);

    switch (Source_.mType)
    {
    case EnemyType::Archer:
    {
        BaseEnemy* enemy = new ArcherEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Shield:
    {
        BaseEnemy* enemy = new ShieldEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Sword:
    {
        BaseEnemy* enemy = new SwordEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear:
    {
        BaseEnemy* enemy = new SpearEnemy(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Archer_Ace:
    {
        BaseEnemy* enemy = new ArcherEnemy_Ace(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Shield_Ace:
    {
        BaseEnemy* enemy = new ShieldEnemy_Ace(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Sword_Ace:
    {
        BaseEnemy* enemy = new SwordEnemy_Ace(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear_Ace:
    {
        BaseEnemy* enemy = new SpearEnemy_Ace(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Boss:
        {
        BaseEnemy* enemy = new LastBoss(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType),this);
        enemy->fSetObjectId(object_count);
        mEnemyVec.emplace_back(enemy);
        }
        break;
    case EnemyType::Count: break;
case EnemyType::Tutorial_NoMove:
{
    BaseEnemy* enemy = new TutorialEnemy_NoAttack(graphics_,
        Source_.mEmitterPoint,
        mEditor.fGetParam(Source_.mType));
    enemy->fSetObjectId(object_count);
    mEnemyVec.emplace_back(enemy);
}
    break;
case EnemyType::Boss_Unit:
    break;
    default:;
    }

    object_count++;
}

void EnemyManager::fSpawn(EnemySendData::EnemySpawnData data, GraphicsPipeline& graphics_)
{
    // �����Ă����f�[�^�����ƂɓG���o��������
    const auto param = mEditor.fGetParam(data.type);


    switch (data.type)
    {
    case EnemyType::Archer:
    {
        BaseEnemy* enemy = new ArcherEnemy(graphics_,
            data.emitter_point, param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Archer);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Shield:
    {
        BaseEnemy* enemy = new ShieldEnemy(graphics_,
            data.emitter_point, param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Shield);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Sword:
    {
        BaseEnemy* enemy = new SwordEnemy(graphics_,
            data.emitter_point, param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Sword);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear:
    {
        BaseEnemy* enemy = new SpearEnemy(graphics_,
            data.emitter_point,
            param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Spear);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Archer_Ace:
    {
        BaseEnemy* enemy = new ArcherEnemy_Ace(graphics_,
            data.emitter_point,
            param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Archer);
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Shield_Ace:
    {
        BaseEnemy* enemy = new ShieldEnemy_Ace(graphics_,
            data.emitter_point, param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Shield);
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Sword_Ace:
    {
        BaseEnemy* enemy = new SwordEnemy_Ace(graphics_,
            data.emitter_point,
            param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Sword);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear_Ace:
    {
        BaseEnemy* enemy = new SpearEnemy_Ace(graphics_,
            data.emitter_point,
            param);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Spear);
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Boss:
        {
        BaseEnemy* enemy = new LastBoss(graphics_,
            data.emitter_point,
            param,this);
        enemy->fSetObjectId(data.enemy_id);
        enemy->SetEnemyType(SendEnemyType::Boss);
        mEnemyVec.emplace_back(enemy);
        }
        break;
    case EnemyType::Count: break;
    case EnemyType::Tutorial_NoMove:
    {
        BaseEnemy* enemy = new TutorialEnemy_NoAttack(graphics_,
            data.emitter_point,
            param);
        enemy->fSetObjectId(data.enemy_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
case EnemyType::Boss_Unit:
    break;
    default:;
    }

}


void EnemyManager::fSendSpawnData(EnemySource Source_)
{
    EnemySendData::EnemySpawnData data;

    //-----�R�}���h��ݒ�-----//
    data.cmd[ComLocation::ComList] = CommandList::Update;

    data.cmd[ComLocation::UpdateCom] = UpdateCommand::EnemySpawnCommand;

    //-----�o������G�̔ԍ��ݒ�-----//
    data.enemy_id = object_count;

    //-----�o���^�C�~���O-----//
    data.spawn_timer = Source_.mSpawnTimer;

    //-----�o���ʒu-----//
    data.emitter_point = Source_.mEmitterPoint;

    //-----�G�̎��-----//
    data.type = Source_.mType;

    CorrespondenceManager::Instance().UdpSend((char*)&data, sizeof(EnemySendData::EnemySpawnData));
}

void EnemyManager::fEnemiesUpdate(GraphicsPipeline& Graphics_,float elapsedTime_)
{
    //-----��ԋ߂��v���C���[�̈ʒu-----//
    DirectX::XMFLOAT3 near_pos{};
    //-----��ԋ߂��v���C���[�̈ʒu�Ƃ̋���-----//
    float near_length{ 100000.0f };
    //-----�v�Z��������������-----//
    float length{};

    // �X�V
    for (const auto enemy : mEnemyVec)
    {
        near_pos = {};
        near_length = 100000.0f;

        for (const auto p : mPlayerPosition)
        {
            length = Math::calc_vector_AtoB_length(enemy->fGetPosition(), p);
            //-----��ԋ߂������ƍ��̓G�̈ʒu�Ƃ̋������r���ď�����������l���X�V-----//
            if (near_length > length)
            {
                near_length = length;
                near_pos = p;
            }
        }

            enemy->fSetPlayerPosition(near_pos);
            enemy->fUpdate(Graphics_,elapsedTime_);
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

void EnemyManager::fCreateRandomEnemy(
    GraphicsPipeline& Graphics_,
    DirectX::XMFLOAT3 SeedPosition_)
{
    if(mEnemyVec.size()>30)
    {
        return;
    }

    // �����œG�̃^�C�v���擾
    std::mt19937 mt{ std::random_device{}() };
    std::uniform_int_distribution<int> RandTargetAdd(0, 10);
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
        // ���݂��Ă���΍폜
        if (enemy)
        {
            delete enemy;
            enemy = nullptr;
        }
    }
    mEnemyVec.clear();
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

void EnemyManager::fGuiMenu(GraphicsPipeline& Graphics_, AddBulletFunc Func_)
{
    imgui_menu_bar("Game", "EnemyManager", mOpenGuiMenu);


#ifdef USE_IMGUI
    if (mOpenGuiMenu)
    {
        ImGui::Begin("EnemyManager");

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

    for(const auto unit:mUnitEntryPointVec)
    {
        BaseEnemy* enemy = new BossUnit(Graphics_,
            unit,
            mEditor.fGetParam(EnemyType::Boss_Unit),
            BulletManager::Instance().fGetAddFunction());
        mEnemyVec.emplace_back(enemy);
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
