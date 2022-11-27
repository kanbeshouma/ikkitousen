#pragma once
#include"BaseEnemy.h"
#include"BasePlayer.h"
#include"BossRushUnit.h"
#include "Common.h"
#include"EnemiesEditor.h"
#include"EnemyFileSystem.h"
#include "LastBoss.h"
#include"MasterEnemyDataAdmin.h"
#include"NetWorkInformationStucture.h"
#include"practical_entities.h"
#include<vector>

//****************************************************************
//
// �G�̊Ǘ��N���X
//
//****************************************************************
class EnemyManager final :public PracticalEntities
{
    //****************************************************************
    //
    // �֐�
    //
    //****************************************************************
public:
    EnemyManager();
    ~EnemyManager() override;

    void fInitialize(GraphicsPipeline& graphics_, AddBulletFunc Func_);

    //-----���g���C���ď�����-----//
    void RestartInitialize();

    //-----�ʏ펞���ɌĂԍX�V����-----//
    void fUpdate(GraphicsPipeline & graphics_,float elapsedTime_, AddBulletFunc Func_);

    //-----�}���`�v���C���̃z�X�g���ĂԍX�V����-----//
    void fHostUpdate(GraphicsPipeline& graphics_, float elapsedTime_, AddBulletFunc Func_, EnemyAllDataStruct& receive_data);

    //-----�}���`�v���C���ɃN���C�A���g�����ĂԍX�V����-----//
    void fClientUpdate(GraphicsPipeline & graphics_,float elapsedTime_, AddBulletFunc Func_, EnemyAllDataStruct& receive_data);

    //-----�_���[�W�f�[�^��ݒ肷��-----//
    void SetReciveDamageData(int obj_id, int damage, GraphicsPipeline& graphics_);

    //-----�G�̏�ԃf�[�^�̐ݒ�-----//
    void fSetReceiveConditionData(EnemySendData::EnemyConditionData data);


    void fRender(GraphicsPipeline& graphics_);

    void fFinalize();

    bool fGetSlow()const;
private:
    //-----�G�̃f�[�^�𑗐M����-----//
    void fCheckSendEnemyData(float elapsedTime_);

    //-----�G�̃f�[�^�𑗐M-----//
    void fSendEnemyData(float elapsedTime_);

    //-----�G�̃_���[�W�f�[�^�𑗐M-----//
    void fSendEnemyDamage(int obj_id,int damage);

    //-----��M�����G�̃f�[�^��ݒ�-----//
    void fSetReceiveEnemyData(float elapsedTime_, EnemySendData::EnemyData data, GraphicsPipeline& graphics_);

    //<�{�X�̍ďo��>//
    void BossExistence(GraphicsPipeline& graphics_);
public:
    //--------------------<�����蔻��>--------------------//

    // �v���C���[�̍U���ƓG�̓����蔻��
    int fCalcPlayerAttackVsEnemies(           // �߂�l �F ���������G�̐�
        DirectX::XMFLOAT3 PlayerCapsulePointA_,// �v���C���[�̃J�v�Z���̏��
        DirectX::XMFLOAT3 PlayerCapsulePointB_,// �v���C���[�̃J�v�Z���̏��
        float PlayerCapsuleRadius_,            // �v���C���[�̃J�v�Z���̏��
        int PlayerAttackPower_   ,              // �v���C���[�̍U����
        GraphicsPipeline& Graphics_,
        float elapsedTime_,
        bool& is_shield
    );
    // �G�̍U���ƃv���C���[�̓����蔻��
    bool fCalcEnemiesAttackVsPlayer(
        DirectX::XMFLOAT3 PlayerCapsulePointA_,// �v���C���[�̃J�v�Z���̏��
        DirectX::XMFLOAT3 PlayerCapsulePointB_,// �v���C���[�̃J�v�Z���̏��
        float PlayerCapsuleRadius_,             // �v���C���[�̃J�v�Z���̏��
        AddDamageFunc Func_
    );


    // �G�̍U���ƃv���C���[�̃J�E���^�[�̓����蔻��
    bool fCalcEnemiesAttackVsPlayerCounter(
        DirectX::XMFLOAT3 PlayerCapsulePointA_,// �v���C���[�̃J�v�Z���̏��
        DirectX::XMFLOAT3 PlayerCapsulePointB_,// �v���C���[�̃J�v�Z���̏��
        float PlayerCapsuleRadius_);             // �v���C���[�̃J�v�Z���̏��)

    // �v���C���[�̉�荞�ݍU���ƓG�̓����蔻��
    void fCalcPlayerStunVsEnemyBody(const DirectX::XMFLOAT3 PlayerPosition_, float Radius_);

    // �X�^�����̓G�̂����v���C���[�ɂ����Ƃ��߂����̂�Ԃ��i���Ȃ����null�j
    BaseEnemy* fGetNearestStunEnemy();


// �G��ǉ�����
    void fAddRushBoss(BossRushUnit* enemy);
public:
    //--------------------<�Q�b�^�[�֐�>--------------------//
    [[nodiscard]]  BaseEnemy* fGetNearestEnemyPosition();
    [[nodiscard]]  BaseEnemy* fGetSecondEnemyPosition();
    [[nodiscard]] bool fGetClearWave() const;
    //--------------------<�Z�b�^�[�֐�>--------------------//
    void fSetIsPlayerChainTime(bool IsChain_);//�v���C���[���`�F�C����Ԃł��邩�ǂ������G�l�~�[���󂯎��֐�

    //-----��ԋ߂��v���C���[�̈ʒu��ݒ肷��-----//
    void fSetPlayerPosition(std::vector<std::tuple<int, DirectX::XMFLOAT3>> Position_);

    //-----�^�[�Q�b�gID����v���C���[�̈ʒu���擾����-----//
    void fSetPlayerIdPosition(const std::vector<std::shared_ptr<BasePlayer>>& players);

    void fSetPlayerSearch(bool Arg_); // �v���C���[���`�F�C���U������True
    void fSetIsTutorial(bool Arg_);
public:
    //--------------------<ImGui>--------------------//
    void fGuiMenu(GraphicsPipeline& Graphics_, AddBulletFunc Func_);
    //--------------------<�E�F�[�u�؂�ւ��֐�>--------------------//
    void fStartWave(int WaveIndex_);
    //--------------------<�E�F�[�u�N���A�������ǂ����𔻒肷��>--------------------//
    [[nodiscard]] bool fWaveClear() const;

    //--------------------<�G�̎��̂��폜����֐�>--------------------//
    void fDeleteEnemies();

    [[nodiscard]] std::vector<BaseEnemy*> fGetEnemies() const;
    size_t fGetEnemyCounts()const;

    //--------------------<�L���b�V���Ƀ��f���f�[�^��o�^>--------------------//
    void fRegisterCash(GraphicsPipeline& graphics_);
    void fDeleteCash(); // �L���b�V�����폜

    // �{�X����Ăяo��
    void fReserveBossUnit(std::vector<DirectX::XMFLOAT3> Vec_);
    void fCreateRandomMasterEnemy(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 SeedPosition_,int grope_id);
    void fCreateRandomEnemy(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 SeedPosition_,int grope_id,int transfer_id);


     // �`���[�g���A���ŌĂяo���֐�
    void fSpawnTutorial_NoAttack(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fSpawnTutorial(float elapsedTime_, GraphicsPipeline& Graphics_);

    //--------------------<�{�X�֘A�̊֐�>--------------------//
    [[nodiscard]] LastBoss::Mode fGetBossMode()const;
    void fSetBossMode(LastBoss::Mode Mode_);
    [[nodiscard]] bool fGetIsEventCamera()const;
    [[nodiscard]] DirectX::XMFLOAT3 fGetEye()const;
    [[nodiscard]] DirectX::XMFLOAT3 fGetFocus()const;
    void fSetBossEye(DirectX::XMFLOAT3 Eye_);
    void fSetBossFocus(DirectX::XMFLOAT3 Focus_);

    bool fGetBossClear()const;

    void fReserveDeleteEnemies();

    void fLimitEnemies();

    //-----�C�x���g�����I������v���C���[���J�E���g����
    //========================================
    //������ : ���I�������
    void EndEnventCount(int count);

    //-----�C�x���g���I��点��-----//
    //=======================
    void EndEvent();
private:
    //--------------------<�G�Ɗ֘A���鏈��>--------------------//

    void fSpawn(GraphicsPipeline& graphics); // �G�̐������Ǘ�
    void fSpawn(EnemySource Source_, GraphicsPipeline& graphics_);

    //-----�}���`�v���C���̃N���C�A���g���̓G�̏o��-----//
    void fSpawn(EnemySendData::EnemySpawnData data, GraphicsPipeline& graphics_);

    void fEnemiesUpdate(GraphicsPipeline& Graphics_,float elapsedTime_); // �G�̍X�V����
    void fEnemiesRender(GraphicsPipeline& graphics_); // �G�̕`�揈��

    void fCreateBossUnit(GraphicsPipeline& Graphics_);

    //-----�G�̏o���f�[�^�𑗐M-----//
    void fSendSpawnData(EnemySource Source_);


    //--------------------<�G���\�[�g����>--------------------//
    void fSort(std::function<bool(const BaseEnemy* A_, const BaseEnemy* B_)> Function_);

    //--------------------<�Ǘ��N���X���Ŋ������鏈��>--------------------//
    void fAllClear(); // �G��S�폜����֐�

    //--------------------<�G���m�̓����蔻��>--------------------//
    void fCollisionEnemyVsEnemy();

    void fLoad(const char* FileName_);
    void fSave(const char* FileName_);

    //-----�G�̃O���[�v�̃z�X�g�̃f�[�^��ݒ肷��-----//
    void SetEnemyGropeHostData();

    //-----�}�X�^�[�����n����-----//
    //======================
    //������ : ���n�Ώۂ̃O���[�v-----//
    void TransferMaster(int grope);

    //****************************************************************
    //
    // �ϐ�
    //
    //****************************************************************
private:
    std::vector<BaseEnemy*> mEnemyVec;  // �G���i�[����R���e�i
    std::vector<BaseEnemy*> mRemoveVec; // �G���i�[����R���e�i
    // ���\�[�X�}�l�[�W���[������ێ��ł���悤�ɃL���b�V���Ƃ��ă��f���f�[��o�^����
    std::vector<BaseEnemy*> mCashEnemyVec{};

    //-----�G�̃}�X�^�[�f�[�^������N���X------//
    std::unique_ptr<MasterEnemyDataAdmin> master_enemy_data;

    //--------------------<�E�F�[�u�Ɋւ���ϐ�>--------------------//
    float mWaveTimer{}; // �e�E�F�[�u�J�n����̌o�ߎ���
    int mCurrentWave{}; // ���݂̃E�F�[�u
    // ���݂̃E�F�[�u�̃f�[�^
    std::vector<EnemySource> mCurrentWaveVec{};
    std::vector<EnemySource> mFirstTest{};
    float mLastTimer{};// �Ō�ɏo�Ă���G�̎���
    EnemyEditor mEditor{};

    bool mDebugMode{};
    bool mIsPlayerChainTime = false;
    //--------------------<�v���C���[�̈ʒu>--------------------//
    std::vector<DirectX::XMFLOAT3> mPlayerPosition{};

    int mUniqueCount{};

    // �J�����V�F�C�N
    float mCameraShakeTime{};
    const float mkOneShakeSec{ 0.15f };

    // �`���[�g���A��
    float mTutorialTimer{};
    const float mkSeparateTutorial{ 1.0f };

    bool mIsReserveDelete{};

    // �{�X�����j�b�g������
    bool mIsReserveBossUnit{};
    std::vector<DirectX::XMFLOAT3> mUnitEntryPointVec{};
    std::vector<EnemySource> mReserveVec{};
    //--------------------<�{�X�֘A�̕ϐ�>--------------------//
    LastBoss::Mode mCurrentMode{ LastBoss::Mode::None };
    //bool mIsBossEvent{};// �C�x���g��
    DirectX::XMFLOAT3 mBossCameraEye{ 0.0f,0.0f,-80.0f };
    DirectX::XMFLOAT3 mBossCameraFocus{0.0f,0.0f,0.0f};

    bool start_boss_rush{ false };
public:
    void SetStartBossRush(bool arg) { start_boss_rush = arg; }
private:
    // �`���[�g���A���̎g�p
    bool mIsTutorial{ false };

    float mDelay{};

    float mSloeTime{};

    //-----�X�|�[���ԍ�-----//
    int object_count{};

    //-----�~���b-----//
    float milliseconds{};

    //-----�C�x���g���I�������l��-----//
    int end_event_count{ 0 };

    //<�{�X�̑��݊m�F�̃f�[�^���M�^�C�}�[>//
    float check_boss_existence_timer{ 0.0f };

    //<�{�X�̑��݊m�F�̃f�[�^���M�t���O>//
    bool check_boss_existence_flg{ false };

    //****************************************************************
    //
    // �萔
    //
    //****************************************************************

    // �G���m�̊Ԋu�𒲐�����
    const float mAdjustmentEnemies{ 1.0f };

    const char* mWaveFileNameArray[7]
    {
        "./resources/Data/Wave1_1.json",
        "./resources/Data/Wave2_1.json",
        "./resources/Data/Wave2_2.json",
        "./resources/Data/Wave3_1.json",
        "./resources/Data/Wave3_2.json",
        "./resources/Data/Wave3_3.json",
        "./resources/Data/WaveBoss.json",
    };

    // ImGui
    bool mOpenGuiMenu{};

    //-----�G�̈ʒu�f�[�^�𑗐M���鎞�̃R�}���h�̃T�C�Y-----//
    const int SendEnemyDataComSize = 4;

    //-----�f�[�^�̑��M�p�x(�~���b)-----//
    const float EnemyDataFrequency = 300.0f;

    //-----�G���_���[�W���󂯂��Ƃ��̖��G����-----//
    const float EnemyInvincibleTime = 0.1f;

    //<�{�X�̏o���`�F�b�N�̃f�[�^���M�p�x>//
    const float CheckBossExistenceTime = 1.5f;

};
