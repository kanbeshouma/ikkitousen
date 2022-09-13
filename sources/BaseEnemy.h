#pragma once

#include"practical_entities.h"
#include"graphics_pipeline.h"
#include"skinned_mesh.h"
#include"EnemyStructuer.h"
#include"Effects.h"
#include<memory>
#include<functional>

typedef std::function<void()> InitFunc;
typedef std::function<void(float, GraphicsPipeline&)> UpdateFunc;
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
    [[nodiscard]] bool fComputeAndGetIntoCamera()const;
    [[nodiscard]] int fGetAttackPower()const;
    [[nodiscard]] float fGetAttackInvTime()const;
    [[nodiscard]] float fGetLengthFromPlayer()const;
    [[nodiscard]] float fGetPercentHitPoint()const;
    [[nodiscard]] bool fGetStun()const;
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

    SkinnedMesh::anim_Parameters mAnimPara{};
    float mDissolve{};
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

    bool mIsInCamera{};
};

