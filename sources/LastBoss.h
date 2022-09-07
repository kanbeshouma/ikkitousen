#pragma once
#include <random>

#include"BaseEnemy.h"
#include "Common.h"
#include"LaserBeam.h"
#include"BossRushUnit.h"
//****************************************************************
//
// ���X�g�{�X
//
//****************************************************************

  // �O���錾
class EnemyManager;

class LastBoss final : public BaseEnemy
{
public:
    enum class Mode // �{�X�̃��[�h
    {
        None,          // �����Ȃ��i���݂��Ȃ��j
        ShipAppear,    // ��͓o��A�j���[�V����
        Ship,          // ��́i�_���[�W�͎󂯂Ȃ��j
        ShipToHuman,   // ��͂���l�ɕϊ�
        Human,         // �l�^�i�̗͂�100%~20%�j
        HumanToDragon, // �l�^����h���S���ɑJ�ڂ��Ă���r���i�_���[�W�͎󂯂Ȃ��j
        Dragon,        // �h���S���i�̗͂�20%~0%�j
        DragonDie,     // �h���S������
        BossDieEnd,
    };

private:
    struct DivideState
    {
        //--------------------<���>--------------------//
        inline static const char* ShipStart  = "ShipStart";
        inline static const char* ShipIdle   = "ShipIdle";
        inline static const char* ShipAttack = "ShipAttack";
        inline static const char* ShipBeamStart   = "ShipBeamStart";
        inline static const char* ShipBeamCharge   = "ShipBeamCharge";
        inline static const char* ShipBeamShoot   = "ShipBeamShoot";
        inline static const char* ShipBeamEnd   = "ShipBeamEnd";
        inline static const char* ShipToHuman   = "ShipToHuman";

        //--------------------<�l�^>--------------------//
        inline static const char* HumanIdle = "HumanIdle"; // �ҋ@
        inline static const char* HumanMove = "HumanMove"; // �ړ�
        inline static const char* HumanAllShot = "HumanAllShot"; // ���˂���
        inline static const char* HumanRotAttack = "HumanRotAttack"; // ��]�U��
        inline static const char* HumanWithdrawal_Begin = "HumanWithdrawal_Begin"; // ��U�Δ䂷��
        inline static const char* HumanWithdrawal_Middle = "HumanWithdrawal_Middle"; // �͈͊O�ő҂�
        // ��莞�ԓ��Ɏ艺���R�U�点�Ȃ���ΕK�E�Z
        inline static const char* HumanWithdrawal_End = "HumanWithdrawal_End";
        inline static const char* HumanBlowAttack = "HumanBlowAttack";

        inline static const char* HumanSpAway = "HumanSpAway"; // �����ɔ��
        inline static const char* HumanSpWait = "HumanSpWait"; // �҂��Ă���
        inline static const char* HumanSpOver = "HumanSpOver"; // ���Ԑ؂�
        inline static const char* HumanSpCharge = "HumanSpCharge"; // ����
        inline static const char* HumanSpShoot = "HumanSpShoot"; // ����
        inline static const char* HumanSpDamage = "HumanSpDamage"; // �_���[�W

        inline static const char* HumanRush = "HumanRush"; // �_���[�W


        //--------------------<�l�^�̎��S>--------------------//
        inline static const char* HumanDieStart = "HumanDieStart";   // �l�^�̎��S�J�n
        inline static const char* HumanDieMiddle = "HumanDieMiddle"; // �l�^�̎��S�ҋ@
        inline static const char* HumanToDragon = "HumanToDragon";// �l�^����h���S��

        //--------------------<�h���S��>--------------------//
        inline static const char* DragonIdle = "DragonIdle"; // �ҋ@
        inline static const char* DragonDieStart = "DragonDieStart"; // ���S
        inline static const char* DragonDieEnd = "DragonDieEnd"; // ���S

        // �u���X
        inline static const char* DragonHideStart= "DragonHideStart"; // ������
        inline static const char* DragonAppear= "DragonAppear"; // �����
        inline static const char* DragonBreathCharge= "DragonBreathCharge"; // ���߂�
        inline static const char* DragonBreathShot= "DragonBreathShot"; // ����

        // �ːi
        inline static const char* DragonRushHide = "DragonRushHide"; // ������
        inline static const char* DragonRushWait = "DragonRushWait"; // �ҋ@
        inline static const char* DragonRushAppear = "DragonRushAppear"; // �����

        // �r�[��
        inline static const char* DragonMoveStart = "DragonMoveStart"; // �ړ�
        inline static const char* DragonBeamStart = "DragonBeamStart"; // ����
        inline static const char* DragonBeamCharge = "DragonBeamCharge"; // ����
        inline static const char* DragonBeamShoot = "DragonBeamShoot"; // ����
        inline static const char* DragonBeamEnd = "DragonBeamEnd";     // �I��


        inline static const char* Stun = "Stun";     // �X�^��

    };

    enum  AnimationName {
        ship_idle,
        ship_beam_charge_start,
        ship_beam_charge_idle,
        ship_beam_shot_start,
        ship_beam_shot_idle,
        ship_to_human,
        human_idle,
        human_move,
        human_beam_charge,
        human_beam_shoot,
        human_beam_end,
        human_bullet,
        human_rush_ready,
        human_rush_start,
        human_rush_idle,
        human_shockwave,
        human_damage,
        human_die,
        human_die_idle,
        human_to_dragon,
        dragon_idle,
        dragon_dive_ready,
        dragon_dive,
        dragon_dive_end,
        dragon_hide,
        dragon_beam_ready,
        dragon_beam_charge,
        dragon_beam_start,
        dragon_beam_idle,
        dragon_beam_end,
        dragon_breath_ready,
        dragon_breath_start,
        dragon_breath_idle,
        dragon_breath_end,
        dragon_damage,
        dragon_die,
        ship_to_human_quick,
        human_to_ship_quick,
        human_to_dragon_quick,
        dragon_to_human_quick,
        ship_appearing_scene,
    };
    enum class AttackKind // �������璊�o����U���̎��
    {
        //--------------------<�l�^>--------------------//
        HumanRotAttack,     // ��]�U��
        HumanAllShot,       // ����
        HumanSpecialAttack  // �K�E�Z
    };
    
    //--------------------<�C�g�̃N���X>--------------------//
    class Turret final : public PracticalEntities
    {
    public:
        Turret(GraphicsPipeline& Graphics_);
        void fUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
        void fRender(GraphicsPipeline& graphics_,
                     const DirectX::XMFLOAT4X4& ParentWorld_,
                     const DirectX::XMFLOAT3& Position_);
        void fSetDissolve(float Dissolve_);

        DirectX::XMFLOAT3 fGetPosition()const;
    private:
        DirectX::XMFLOAT4 mOrientation{0.0f,0.0f,0.0f,1.0f}; // �p�x
        std::shared_ptr<SkinnedMesh> mpModel{ nullptr };
        SkinnedMesh::anim_Parameters mAnimPara{};
        DirectX::XMFLOAT3 mPosition;
        float mDissolve{};
    };

    //--------------------<���C�g�̃N���X>--------------------//
    class SecondGun final : public PracticalEntities
    {
    public :
        SecondGun(GraphicsPipeline& Graphics_);
        void fUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
        void fRender(GraphicsPipeline& graphics_,
            const DirectX::XMFLOAT4X4& ParentWorld_,
            const DirectX::XMFLOAT3& Position_);

        void fSetDissolve(float Dissolve_);
    private:
        std::shared_ptr<SkinnedMesh> mpModel{ nullptr };
        SkinnedMesh::anim_Parameters mAnimPara{};
        float mDissolve;
    };

public:
     struct BossParamJson
     {
         int BossStateNumber{}; // �{�X�̌��݂̃X�e�[�g�i�^�C�g���ɖ߂�ƃ��Z�b�g����j
         bool mShowMovie_ShipToHuman{};
         bool mShowMovie_HumanToDragon{};
         bool mShowMovie_DragonDie{};

         template<class Archive>
         void serialize(Archive& archive, std::uint32_t const version)
         {
             archive(
                 cereal::make_nvp("BossStateNumber", BossStateNumber),
                 cereal::make_nvp("mShowMovie_ShipToHuman", mShowMovie_ShipToHuman),
                 cereal::make_nvp("mShowMovie_HumanToDragon", mShowMovie_HumanToDragon),
                 cereal::make_nvp("mShowMovie_DragonDie", mShowMovie_DragonDie));
         }
     };

public:
    LastBoss(GraphicsPipeline& Graphics_,
             const DirectX::XMFLOAT3& EmitterPoint_,
             const EnemyParamPack& ParamPack_,
             EnemyManager* pEnemyManager_);

    LastBoss(GraphicsPipeline& Graphics_);
    ~LastBoss() override;
    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;

    void fSetStun(bool Arg_, bool IsJust_) override;


protected:
    void fRegisterFunctions() override;

private:
    void fGuiMenu();

    [[nodiscard]] float fComputePercentHp() const; // �ő�̗͂ɑ΂��錻�݂̗̑͂̊�����0.0f~1.0f�ŕԂ�
    void fChangeHumanToDragon();

    void fSpawnChildUnit(GraphicsPipeline& Graphics_, int Amounts_) const;

    // �ړ�����
    bool fLimitStageHuman(float elapsedTime_);
private:
    //****************************************************************
    //
    // �ϐ�
    //
    //****************************************************************
    float mTimer{}; // �ėp�^�C�}�[


    std::mt19937 mt{ std::random_device{}() };

    //--------------------<�{�X�̃^���b�g>--------------------//
    std::unique_ptr<Turret> mpTurretRight{ nullptr };
    std::unique_ptr<Turret> mpTurretLeft{ nullptr };
    skeleton::bone mTurretBoneRight{};  // �^���b�g�̃{�[��
    skeleton::bone mTurretBoneLeft{};   // �^���b�g�̃{�[��

    //--------------------<�{�X�̃Z�J���h�K��>--------------------//
    std::unique_ptr<SecondGun> mpSecondGunRight{ nullptr };
    std::unique_ptr<SecondGun> mpSecondGunLeft{ nullptr };
    skeleton::bone mSecondGunBoneRight{};  // �^���b�g�̃{�[��
    skeleton::bone mSecondGunBoneLeft{};   // �^���b�g�̃{�[��


    //--------------------<�l�^�̕K�E�Z>--------------------//
    DirectX::XMFLOAT3 mAwayBegin{}; // ��т̂��n�_�̎n�_
    const DirectX::XMFLOAT3 mAwayEnd{0.0f,40.0f,200.0f}; // ��т̂��n�_�̏I�_
    float mAwayLerp{}; // Lerp�W��
    DirectX::XMFLOAT4 mBeginOrientation{};
    DirectX::XMFLOAT4 mEndOrientation{ 0.0f,0.0f,0.0f,1.0f };

    EnemyManager* mpEnemyManager{nullptr};

    // ���[�r�[�X�L�b�v
    float mSkipTimer{};

public:
    // ���݂̃��[�h
    Mode mCurrentMode{ Mode::Ship };

private:
    // �e��
    AddBulletFunc mfAddBullet{};
    float mShotTimer{ 0.0f };

    // �v���C���[���痣���
    DirectX::XMFLOAT3 mMoveBegin{};
    DirectX::XMFLOAT3 mMoveEnd{};
    float mMoveThreshold{};
    bool mFirstSp{ false };
    bool mSecondSp{ false };

    // �u���X
    int mDragonBreathCount{};
    bool mIsShotBreath{};

    // �������ːi
    std::vector<BossRushUnit*> mRushVec{};


    // �S�̍U���G�t�F�N�g
    std::unique_ptr<Effect> mpAllAttackEffect{ nullptr };

    // �J�������o�Ɏg���{�[��
    skeleton::bone mCameraEyeBone{};
    skeleton::bone mCameraFocusBone{};

    // �r�[���𔭎˂��钷��
    float mBeamLength{};
    float mStartBeamRadian{};
    //--------------------<���[�U�[�|�C���^�[>--------------------//
    LaserBeam mShipPointer{};  // �D
    LaserBeam mRightPointer{}; // �E��
    LaserBeam mLeftPointer{};  // ����

    // �ʒu
    DirectX::XMFLOAT3 mShipFacePosition{};
    DirectX::XMFLOAT3 mShipPointerEnd{};
    // �|�C���^�[�̒���
    float mPointerLength{};

    //--------------------<���[�U�[�|�C���^�[�̃{�[��>--------------------//
    skeleton::bone mShipFace{};

    //--------------------<�l�^�r�[���̃^�[�Q�b�g>--------------------//
    DirectX::XMFLOAT3 mHumanBeamTarget{};

    //--------------------<�{�X�̌ۓ�>--------------------//
    float mHeartTimer{};
    float mRgbColorPower{};
    float mRgbColorSpeed{ 10.0f };

    //--------------------<�h���S���̈ړ�>--------------------//
    float mDragonMoveThreshold{};

    //--------------------<�G�t�F�N�g>--------------------//
    std::unique_ptr<Effect> mpBeamEffect{ nullptr };
    std::unique_ptr<Effect> mpBeamBaseEffect{ nullptr };

    std::unique_ptr<Effect> mpBeamRightEffect{ nullptr };
    std::unique_ptr<Effect> mpBeamLeftEffect{ nullptr };
    std::unique_ptr<Effect> mpDieEffect{ nullptr };

    // �r�[���̔��ˈʒu
    DirectX::XMFLOAT3 mBeamEffectPosition{};
    DirectX::XMFLOAT4 mBeamEffectOrientation{ 0.0f,0.0f,0.0f,1.0f };

    DirectX::XMFLOAT4 mBeamRightOrientation{ 0.0f,0.0f,0.0f,1.0f };
    DirectX::XMFLOAT4 mBeamLeftOrientation{ 0.0f,0.0f,0.0f,1.0f };

    float mAddRadian{};
    bool mIsSpawnEnemy{};
    float mDragonBeamLength{};
    float mPerformThresold{};

    //--------------------<�{�X�̃p�����[�^�[>--------------------//
    inline  static BossParamJson mBossParam;
public:
    static BossParamJson fLoadParam();
    static void fResetLoadRaram();
    static void fSaveParam();

private:
    bool mSeArrayShipToHuman[5]{}; // �D����l��SE
    bool mSeArrayHumanToDragon[12]{}; // �l����h���S����SE
    bool mShipRoar{};
    bool mDrawSkip{};

private:
    //****************************************************************
    //
    // �萔
    //
    //****************************************************************
    const float mkRotSpeed { 20.0f }; // ��]���x
    const float  mkShipBeamChargeSec{ 4.0f }; // ��͂̃r�[�����`���[�W���钷��
    const float  mkShipBeamShootSec{ 5.0f }; // ��͂̃r�[���𔭎˂��Ă��钷��
    const float  mkHumanDieIdleSec{ 5.0f };  // �l�^�̎��S����

    const float mkPercentToDragon{ 0.5f }; // �h���S���`�ԂɑJ�ڂ���̗͂̊���

    const float mkWaitHeartEffect = 0.5f;
    const float mkHumanAllShotDelay{ 0.1f };
    const float mkHumanAllShotEnd{ 1.3f };
    const float mkHumanAllShotBegin{ 0.3f };

    const float mkHumanSpWaitTime{ 15.0f };
    const float mkHumanSpBeamTime{ 10.0f };
    const float mkSpChargeTime = 7.0f;
    const float mkDistanceToPlayer{ 80.0f };
    const float mkLimitStage{ 400.0f };
    const float mkTimerBlow{ 0.8f };
    const float mkDragonHideTime{ 0.5f };
    const float mkDragonRushWaitTime{ 4.0f };
    const float mkDragonBeamChargeTime{ 1.0f };
private:

    //****************************************************************
    //
    // �X�e�[�g�}�V��
    //
    //****************************************************************
    // ��͂̍ŏ��̓����i���o�j
    void fShipStartInit();
    void fShipStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    // ��͂̑ҋ@���
    void fShipIdleInit();
    void fShipIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    // ��͂̍U��
    void fShipAttackInit();
    void fShipAttackUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    // ��͂̍Ō�̍U���i�����ς���Ύ��ցj
    void fShipBeamStartInit();
    void fShipBeamStartUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    void fShipBeamChargeInit();
    void fShipBeamChargeUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    void fShipBeamShootInit();
    void fShipBeamShootUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    void fShipBeamEndInit();
    void fShipBeamEndUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    void fChangeShipToHumanInit();
    void fChangeShipToHumanUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    //--------------------<�l�^>--------------------//
    void fHumanIdleInit();
    void fHumanIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanMoveInit();
    void fHumanMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanAllShotInit();
    void fHumanAllShotUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanBlowAttackInit();
    void fHumanBlowAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fMoveAwayInit();
    void fMoveAwayUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanRushInit();
    void fHumanRushUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //--------------------<�l�^�K�E�Z>--------------------//
    void fHumanSpAttackAwayInit(); // ��т̂�
    void fHumanSpAttackAwayUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanSpAttackSummonInit(); // �G������
    void fHumanSpAttackSummonUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanSpAttackWaitInit(); // ��莞�ԑҋ@
    void fHumanSpAttackWaitUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanSpAttackCancelInit(); // �S�G��|���ꂽ��C�x���g
    void fHumanSpAttackCancelUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fHumanSpAttackTimeOverInit(); // �������Ԃ𒴂�����U��
    void fHumanSpAttackTimeOverUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    void fHumanSpAttackChargeInit();
    void fHumanSpAttackChargeUpdate(float elapsedTime_,GraphicsPipeline& Graphics_);

    void fHumanSpBeamShootInit();
    void fHumanSpBeamShootUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    void fHumanSpDamageInit();
    void fHumanSpDamageUpdate(float elapsedTime_,
        GraphicsPipeline& Graphics_);

    //--------------------<�l�^�̎��S�G�t�F�N�g>--------------------//
    void fHumanDieStartInit();
    void fHumanDieStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fHumanDieMiddleInit();
    void fHumanDieMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fHumanToDragonInit();
    void fHumanToDragonUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //--------------------<�h���S��>--------------------//
    void fDragonIdleInit();
    void fDragonIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // �f�B�]���u�ŏ�����
    void fDragonFastBreathStartInit();
    void fDragonFastBreathStartUpdate(float elapsedTime_,GraphicsPipeline& Graphics_);

    // �����
    void fDragonBreathAppearInit();
    void fDragonBreathAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonBreathChargeInit();
    void fDragonBreathChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonBreathShotInit();
    void fDragonBreathShotUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonRushHideInit();
    void fDragonRushHideUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonRushWaitInit();
    void fDragonRushWaitUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonRushAppearInit();
    void fDragonRushAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //--------------------<�r�[��>--------------------//
    void fDragonBeamMoveInit();
    void fDragonBeamMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonBeamStartInit();
    void fDragonBeamStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonBeamChargeInit();
    void fDragonBeamChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);


    void fDragonBeamShotInit();
    void fDragonBeamShotUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDragonBeamEndInit();
    void fDragonBeamEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //--------------------<�h���S�����S�G�t�F�N�g>--------------------//
    void fDragonDieStartInit();
    void fDragonDieStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fDragonDieMiddleInit();
    void fDragonDieMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);


    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

public:
    void fRender(GraphicsPipeline& Graphics_) override;
    bool fDamaged(int Damage_, float InvincibleTime_, GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fDie(GraphicsPipeline& Graphics_) override;
};
