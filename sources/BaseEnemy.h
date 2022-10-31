#pragma once

#include"practical_entities.h"
#include"graphics_pipeline.h"
#include"skinned_mesh.h"
#include"EnemyStructuer.h"
#include"Effects.h"
#include<memory>
#include<functional>

//-----�������֐�������function-----//
typedef std::function<void()> InitFunc;

//-----�X�V����������function-----//
typedef std::function<void(float, GraphicsPipeline&)> UpdateFunc;

//-----�������ƍX�V��������̕ϐ��ɂ܂Ƃ߂邽�߂�tuple-----//
typedef std::tuple<InitFunc, UpdateFunc > FunctionTuple;

class BaseEnemy :public PracticalEntities
{
public:
    BaseEnemy(GraphicsPipeline& Graphics_,
              const char* FileName_,
              const EnemyParamPack& Param_,
              const DirectX::XMFLOAT3& EntryPosition_,
              const wchar_t* IconFileName = L"./resources/Sprites/ui/minimap/minimap_enemy.png");
protected:
    BaseEnemy(GraphicsPipeline& Graphics_, const char* FileName_);
public:
    ~BaseEnemy() override;

    virtual void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) = 0;
    virtual void fUpdateAttackCapsule() = 0;
    virtual void fDie(GraphicsPipeline& Graphics_);

    [[nodiscard]]float fBaseUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    virtual void fRender(GraphicsPipeline& Graphics_);
    virtual bool  fDamaged(int Damage_, float InvincibleTime_, GraphicsPipeline& Graphics_, float elapsedTime_);
    void fUpdateVernierEffectPos();
    void fTurnToPlayer(float elapsedTime_,float RotSpeed_);
    void fTurnToTarget(float elapsedTime_,float RotSpeed_,DirectX::XMFLOAT3 Target_);
    void fTurnToPlayerXYZ(float elapsedTime_,float RotSpeed_);
    void fMoveFront(float elapsedTime_, float MoveSpeed_);
    void fComputeInCamera();
    void fLimitPosition();

    //--------------------<�Z�b�^�[�֐�>--------------------//
    virtual void fSetStun(bool Arg_, bool IsJust_ = false);
    void fSetPlayerPosition(const DirectX::XMFLOAT3& PlayerPosition_);
    void fSetAttack(bool Arg_);
    void fSetAttackOperation(bool Arg_);
    void fSetIsLockOnOfChain(bool RockOn_);
    void fSetIsPlayerSearch(bool Arg_);
    void fSetLaunchDissolve();
    //--------------------<�Q�b�^�[�֐�>--------------------//
    [[nodiscard]] bool fGetAttack() const;
    [[nodiscard]] bool fGetAttackOperation() const;
    [[nodiscard]] const Capsule& fGetBodyCapsule();
    [[nodiscard]] const Capsule& fGetAttackCapsule()const;
    [[nodiscard]] const DirectX::XMFLOAT3& fGetPosition()const;
    [[nodiscard]] bool fGetIsAlive()const;
    [[nodiscard]] void fSetIsAlive(bool arg);
    [[nodiscard]] bool fComputeAndGetIntoCamera()const;
    [[nodiscard]] int fGetAttackPower()const;
    [[nodiscard]] float fGetAttackInvTime()const;
    [[nodiscard]] float fGetLengthFromPlayer()const;
    [[nodiscard]] float fGetPercentHitPoint()const;
    [[nodiscard]] bool fGetStun()const;
    [[nodiscard]] float fGetCurrentHitPoint()const;
    [[nodiscard]] void fSetCurrentHitPoint(int hp);
    //-----�G���o�����Ă��邩�ǂ���----//
    [[nodiscard]] bool fGetAppears()const;
    [[nodiscard]] bool fIsLockOnOfChain()const;
    [[nodiscard]] float fGetDissolve()const;
    [[nodiscard]] DirectX::XMFLOAT3 fGetForward()const;
    bool fGetIsBoss()const;
    bool fGetInnerCamera();
protected:
    std::shared_ptr<SkinnedMesh> mpModel{ nullptr };

    DirectX::XMFLOAT3 mPosition{};
    DirectX::XMFLOAT3 mScale{1.0f,1.0f,1.0f};
    DirectX::XMFLOAT4 mOrientation{ 0.0f,0.0f,0.0f,1.0f };
    std::vector<DirectX::XMFLOAT4X4> transform{};

    DirectX::XMFLOAT3 mPlayerPosition{};

    //-----���^�[�Q�b�g���Ă���v���C���[�̔ԍ�-----//
    int target_player_id{ -1 };

    //-----��M�����ʒu��ݒ�-----//
    DirectX::XMFLOAT3 mReceivePositiom{};

    //-----��M�f�[�^�̈ʒu�̋��e�l-----//
    const float AllowableLimitPosition{ 2.0f };

    //-----�ʒu�̕�Ԃ�����t���O-----//
    bool mStartlerp{ false };

    //-----�ʒu�̕��-----//
    void LerpPosition(float elapsedTime_);
public:
    void fSetPosition(DirectX::XMFLOAT3 pos) { mPosition = pos; }

    void fSetReceivePosition(DirectX::XMFLOAT3 pos);

    DirectX::XMFLOAT3 GetTargetPosition() { return mPlayerPosition; }

    void fSetTargetPlayerId(int id) { target_player_id = id; }

    int fGetTargetPlayerId() { return target_player_id; }

protected:
    SkinnedMesh::anim_Parameters mAnimPara{};
    float mDissolve{};

    //-----�G�̎�ނ�ݒ�-----//
    SendEnemyType type;
public:
    void SetEnemyType(SendEnemyType t) { type = t; }
    SendEnemyType GetEnemyType() { return type; }
protected:
    int ai_state{};
public:
    int fGetEnemyAiState() { return ai_state; }
    virtual void fSetEnemyState(int state) = 0;
public:
    std::unique_ptr<SpriteBatch> mpIcon{ nullptr };//�~�j�}�b�v�Ŏg���p�̃A�C�R��
    bool mIsSuccesGuard = false;
protected:
    float mAnimationSpeed{1.0f};
    bool mIsStun{}; // �X�^����Ԃ��ǂ���

    //-----�G���o���������ǂ���-----//
    bool is_appears{ false };

    int mCurrentHitPoint{};
    float mInvincibleTime{};
    const int mMaxHp{};
    bool mIsAlive{true};// ���S���o�̂���
protected:
    int mAttackPower{};
    float mAttackInvTime{};
protected:
    const float mStunTime{}; // �X�e�[�g�̏������ł��̒l��Timer�ɑ������
    bool mIsAttack{};
    //-----�U������ɓ����Ă��邩�ǂ���-----//
    //-----�W���X�g���p-----//
    bool attack_operation{};
private:
    float mCubeHalfSize{};
protected:
    // StateMachine
    std::map<std::string, FunctionTuple> mFunctionMap{};

    // �{�X���ǂ���
    bool mIsBoss{};
private:
    FunctionTuple mCurrentTuple{};
protected:
    virtual void fRegisterFunctions() = 0;
    void fChangeState(const char* Tag_);
    bool mIsLockOnOfChain = false;

    bool mIsPlayerSearch{}; // �`�F�C���U���Ń��b�N�I������Ă邩�ǂ���
protected:
    // �U���̓����蔻��
    Capsule mAttackCapsule{};
    // �_���[�W��H�炤�����蔻��
    Capsule mBodyCapsule{};
private:
    skeleton::bone mVenierBone{};
protected:
    char object_id{};
public:
    void fSetObjectId(char id) { object_id = id; }
    char fGetObjectId() { return object_id; }
    //----------�G�̃z�X�g�A���n���A�O���[�v�̔ԍ��̕ϐ�----------//
    //==============================================//
#pragma region EnemyMasterCheck
protected:
    //-----�z�X�g���ǂ���-----//
    bool master{ false };
public:
    bool fGetMaster() { return master; }
    void fSetMaster(bool arg) { master = arg; }
protected:
    //-----�z�X�g�̏��n��-----//
    int transfer_host{ -1 };
public:
    int fGetTransfer() { return transfer_host; }
    void fSetMaster(int arg) { transfer_host = arg; }
 protected:
     //-----�O���[�v��ID-----//
     int grope_id{ -1 };
public:
    int fGetGropeId() { return grope_id; }
    void fSetGropeId(int arg) { grope_id = arg; }
public:
    ////-----�G�̃O���[�v�f�[�^�̈ꊇ�ݒ�-----//
    //================================
    //������ : �z�X�g���ǂ���
    //������ : ���n��
    //��O���� : �O���[�v�ԍ�
    void SetEnemyGropeData(bool master_arg,int transfer_arg,int grope_arg);
#pragma endregion
    //=================================================//

    //----------���[�_�[�̈ʒu�AAI,�^�[�Q�b�g���Ă���v���C���[�̔ԍ�-----//
    //=====================================================//
#pragma region EnemyMasterData
protected:
    //-----���[�_�[�̈ʒu-----//
    DirectX::XMFLOAT3 master_pos{};
public:
    void fSetMasterPos(DirectX::XMFLOAT3 arg) { master_pos = arg; }
protected:
    //-----���[�_�[��AI�X�e�[�g-----//
    int master_ai_state{ -1 };
public:
    void fSetMasterAi(int arg) { master_ai_state = arg; }
protected:
    //-----���[�_�[���^�[�Q�b�g���Ă���v���C���[�̔ԍ�-----//
    int master_target_id{ -1 };
public:
    void fSetMasterTargetId(int arg) { master_target_id = arg; }

    //-----�S�Ẵf�[�^���ꊇ�Őݒ�-----//
    //==========================//
    //������ : �}�X�^�[�̈ʒu
    //������ : �}�X�^�[��AI
    //��O���� : �}�X�^�[�̃^�[�Q�b�g���Ă���ID
    void fSetMasterData(DirectX::XMFLOAT3 m_pos, int m_ai, int  m_target);

#pragma endregion
    //=====================================================//

    //----------�G�̑�܂���AI�̑J�ڊ֐��ƃX�e�[�g----------//
    //========================================//
protected:
    //-----�}�X�^�[���ݒ肷��AI-----//
    enum MasterAiState
                    {
                        //-----�ҋ@-----//
                        Idle,
                        //-----�ړ�-----//
                        Move,
                        //-----�U��-----//
                        Attack
                    };
#pragma region TransitionAiFunc
protected:
    //-----�ҋ@�ɑJ��-----//
    virtual void AiTransitionIdle() = 0;

    //-----�ړ��ɑJ��-----//
    virtual void AiTransitionMove() = 0;

    //-----�U���ɑJ��-----//
    virtual void AiTransformAttack() = 0;

#pragma endregion
    //========================================//

    //----------�G�t�F�N�g---------//
    //===================//
#pragma region EnemyEffect
protected:
    //--------------------<�G�t�F�N�g>--------------------//
    std::unique_ptr<Effect> mVernierEffect{ nullptr };
    inline static const char* mkVernierPath = "./resources/Effect/sluster_enemy2.efk";
    std::unique_ptr<Effect> mStunEffect{ nullptr };
    inline static const char* mkStunPath = "./resources/Effect/stun.efk";
protected:
    std::unique_ptr<Effect> mBombEffect{ nullptr };
    inline static const char* mkBombPath = "./resources/Effect/enemy_die.efk";
    std::unique_ptr<Effect> mDeathEffect{ nullptr };
    inline static const char* mkDeathPath = "./resources/Effect/Bomb2.efk";
    std::unique_ptr<Effect> mDamageEffect{ nullptr };
    inline static const char* mkDamagePath = "./resources/Effect/enemy_hit.efk";

#pragma endregion
    //===================//
protected:
    bool mIsInCamera{};

    bool display_scape_imgui{ false };
};

