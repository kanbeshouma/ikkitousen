#pragma once
#include"BaseEnemy.h"
#include"EnemyFileSystem.h"
#include"EnemiesEditor.h"
#include"practical_entities.h"
#include"BossRushUnit.h"
#include<vector>


#include "Common.h"
#include "LastBoss.h"

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
    EnemyManager() = default;
    ~EnemyManager() override;

    void fInitialize(GraphicsPipeline& graphics_, AddBulletFunc Func_);
    void fUpdate(GraphicsPipeline & graphics_,float elapsedTime_, AddBulletFunc Func_);
    void fRender(GraphicsPipeline& graphics_);
    void fFinalize();

    bool fGetSlow()const;
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
    void fSetPlayerPosition(std::vector<DirectX::XMFLOAT3> Position_);
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
    void fCreateRandomEnemy(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 SeedPosition_);


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
private:
    //--------------------<�G�Ɗ֘A���鏈��>--------------------//
    void fSpawn(GraphicsPipeline& graphics); // �G�̐������Ǘ�
    void fSpawn(EnemySource Source_, GraphicsPipeline& graphics_);
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
    bool mIsBossEvent{};// �C�x���g��
    DirectX::XMFLOAT3 mBossCameraEye{ 0.0f,0.0f,-80.0f };
    DirectX::XMFLOAT3 mBossCameraFocus{0.0f,0.0f,0.0f};

    // �`���[�g���A���̎g�p
    bool mIsTutorial{ false };

    float mDelay{};

    float mSloeTime{};

    //-----�X�|�[���ԍ�-----//
    int object_count{};

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

};
