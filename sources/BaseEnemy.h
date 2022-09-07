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

    //--------------------<セッター関数>--------------------//
    virtual void fSetStun(bool Arg_, bool IsJust_ = false);
    void fSetPlayerPosition(const DirectX::XMFLOAT3& PlayerPosition_);
    void fSetAttack(bool Arg_);
    void fSetIsLockOnOfChain(bool RockOn_);
    void fSetIsPlayerSearch(bool Arg_);
    void fSetLaunchDissolve();
    //--------------------<ゲッター関数>--------------------//
    [[nodiscard]] bool fGetAttack() const;
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

    DirectX::XMFLOAT3 mPlayerPosition{};

    SkinnedMesh::anim_Parameters mAnimPara{};
    float mDissolve{};
public:
    std::unique_ptr<SpriteBatch> mpIcon{ nullptr };//ミニマップで使う用のアイコン
    bool mIsSuccesGuard = false;
protected:
    float mAnimationSpeed{1.0f};
    bool mIsStun{}; // スタン状態かどうか
    int mCurrentHitPoint{};
    float mInvincibleTime{};
    const int mMaxHp{};
    bool mIsAlive{true};// 死亡演出のため
protected:
    int mAttackPower{};
    float mAttackInvTime{};
protected:
    const float mStunTime{}; // ステートの初期化でこの値をTimerに代入する
    bool mIsAttack{};
private:
    float mCubeHalfSize{};
protected:
    // StateMachine
    std::map<std::string, FunctionTuple> mFunctionMap{};

    // ボスかどうか
    bool mIsBoss{};
private:
    FunctionTuple mCurrentTuple{};
protected:
    virtual void fRegisterFunctions() = 0;
    void fChangeState(const char* Tag_);
    bool mIsLockOnOfChain = false;

    bool mIsPlayerSearch{}; // チェイン攻撃でロックオンされてるかどうか
protected:
    // 攻撃の当たり判定
    Capsule mAttackCapsule{};
    // ダメージを食らう当たり判定
    Capsule mBodyCapsule{};
private:
    skeleton::bone mVenierBone{};

protected:
    //--------------------<エフェクト>--------------------//
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

