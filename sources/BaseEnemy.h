#pragma once

#include"practical_entities.h"
#include"graphics_pipeline.h"
#include"skinned_mesh.h"
#include"EnemyStructuer.h"
#include"Effects.h"
#include<memory>
#include<functional>

//-----初期化関数を入れるfunction-----//
typedef std::function<void()> InitFunc;

//-----更新処理を入れるfunction-----//
typedef std::function<void(float, GraphicsPipeline&)> UpdateFunc;

//-----初期化と更新処理を一つの変数にまとめるためのtuple-----//
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
    void fSetAttackOperation(bool Arg_);
    void fSetIsLockOnOfChain(bool RockOn_);
    void fSetIsPlayerSearch(bool Arg_);
    void fSetLaunchDissolve();
    //--------------------<ゲッター関数>--------------------//
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
    //-----敵が出現しているかどうか----//
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

    //-----今ターゲットしているプレイヤーの番号-----//
    int target_player_id{ -1 };

    //-----受信した位置を設定-----//
    DirectX::XMFLOAT3 mReceivePositiom{};

    //-----受信データの位置の許容値-----//
    const float AllowableLimitPosition{ 2.0f };

    //-----位置の補間をするフラグ-----//
    bool mStartlerp{ false };

    //-----位置の補間-----//
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

    //-----敵の種類を設定-----//
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
    std::unique_ptr<SpriteBatch> mpIcon{ nullptr };//ミニマップで使う用のアイコン
    bool mIsSuccesGuard = false;
protected:
    float mAnimationSpeed{1.0f};
    bool mIsStun{}; // スタン状態かどうか

    //-----敵が出現したかどうか-----//
    bool is_appears{ false };

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
    //-----攻撃動作に入っているかどうか-----//
    //-----ジャスト回避用-----//
    bool attack_operation{};
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
    char object_id{};
public:
    void fSetObjectId(char id) { object_id = id; }
    char fGetObjectId() { return object_id; }
    //----------敵のホスト、譲渡順、グループの番号の変数----------//
    //==============================================//
#pragma region EnemyMasterCheck
protected:
    //-----ホストかどうか-----//
    bool master{ false };
public:
    bool fGetMaster() { return master; }
    void fSetMaster(bool arg) { master = arg; }
protected:
    //-----ホストの譲渡順-----//
    int transfer_host{ -1 };
public:
    int fGetTransfer() { return transfer_host; }
    void fSetMaster(int arg) { transfer_host = arg; }
 protected:
     //-----グループのID-----//
     int grope_id{ -1 };
public:
    int fGetGropeId() { return grope_id; }
    void fSetGropeId(int arg) { grope_id = arg; }
public:
    ////-----敵のグループデータの一括設定-----//
    //================================
    //第一引数 : ホストかどうか
    //第二引数 : 譲渡順
    //第三引数 : グループ番号
    void SetEnemyGropeData(bool master_arg,int transfer_arg,int grope_arg);
#pragma endregion
    //=================================================//

    //----------リーダーの位置、AI,ターゲットしているプレイヤーの番号-----//
    //=====================================================//
#pragma region EnemyMasterData
protected:
    //-----リーダーの位置-----//
    DirectX::XMFLOAT3 master_pos{};
public:
    void fSetMasterPos(DirectX::XMFLOAT3 arg) { master_pos = arg; }
protected:
    //-----リーダーのAIステート-----//
    int master_ai_state{ -1 };
public:
    void fSetMasterAi(int arg) { master_ai_state = arg; }
protected:
    //-----リーダーがターゲットしているプレイヤーの番号-----//
    int master_target_id{ -1 };
public:
    void fSetMasterTargetId(int arg) { master_target_id = arg; }

    //-----全てのデータを一括で設定-----//
    //==========================//
    //第一引数 : マスターの位置
    //第二引数 : マスターのAI
    //第三引数 : マスターのターゲットしているID
    void fSetMasterData(DirectX::XMFLOAT3 m_pos, int m_ai, int  m_target);

#pragma endregion
    //=====================================================//

    //----------敵の大まかなAIの遷移関数とステート----------//
    //========================================//
protected:
    //-----マスターが設定するAI-----//
    enum MasterAiState
                    {
                        //-----待機-----//
                        Idle,
                        //-----移動-----//
                        Move,
                        //-----攻撃-----//
                        Attack
                    };
#pragma region TransitionAiFunc
protected:
    //-----待機に遷移-----//
    virtual void AiTransitionIdle() = 0;

    //-----移動に遷移-----//
    virtual void AiTransitionMove() = 0;

    //-----攻撃に遷移-----//
    virtual void AiTransformAttack() = 0;

#pragma endregion
    //========================================//

    //----------エフェクト---------//
    //===================//
#pragma region EnemyEffect
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

#pragma endregion
    //===================//
protected:
    bool mIsInCamera{};

    bool display_scape_imgui{ false };
};

