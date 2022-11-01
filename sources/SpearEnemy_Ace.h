#pragma once
#include"BaseEnemy.h"
//****************************************************************
//
// 槍の敵のエース
//
//****************************************************************
class SpearEnemy_Ace final : public BaseEnemy
{
    struct DivideState
    {
        inline static const char* Start       = "Start";
        inline static const char* Idle        = "Idle";
        inline static const char* Move        = "Move";
        inline static const char* WipeReady   = "WipeReady";   // 薙ぎ払い準備
        inline static const char* WipeBegin   = "WipeBegin";   // 薙ぎ払いはじめ
        inline static const char* WipeAttack  = "WipeAttack";  // 薙ぎ払い中
        inline static const char* WipeEnd     = "WipeEnd";     // 薙ぎ払い終わり
        inline static const char* ThrustBegin = "ThrustBegin"; // 突きはじめ
        inline static const char* Thrust      = "Thrust";      // 突き
        inline static const char* ThrustEnd   = "ThrustEnd";   // 突き終わり
        inline static const char* Stun        = "Stun";        // スタン

    };
    enum AiState
    {
        Start,
        Idle,
        Move,
        WipeBegin,
        WipeAttack,
        WipeEnd,
        ThrustBegin,
        Thrust,
        Stun,
    };

    enum AnimationName {
        idle,
        walk,
        attack_idle,
        attack_up,
        attack_down,
        damage,
        ace_attack_ready,
        ace_attack_idle,
        ace_attack_start,
        ace_attack,
        ace_attack_end,
        stun,
    };
public:
    SpearEnemy_Ace(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_/*スポーン位置*/,
        const EnemyParamPack& ParamPack_);
    SpearEnemy_Ace(GraphicsPipeline& Graphics_);

    ~SpearEnemy_Ace() override = default;
    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;
    void fSetEnemyState(int state) override;

protected:
    void fRegisterFunctions() override;

private:
    float mWaitTimer{}; // 待機時間
    skeleton::bone mpSpearBoneBottom{};
    skeleton::bone mpSpearBoneTop{};
    const float mAttackLength{ 8.0f }; // プレイヤーとの距離がこの距離以下になったら

private:
    // ステートマシンの関数群
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fMoveInit();
    void fMoveUpdate( float elapsedTime_, GraphicsPipeline& Graphics_);

    void fWipeReadyInit();
    void fWipeReadyUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fWipeBeginInit();
    void fWipeBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fWipeAttackInit();
    void fWipeAttackUpdate( float elapsedTime_, GraphicsPipeline& Graphics_);

    void fStunInit();
    void fStunUpdate(float elapsedTime_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;
    //----------敵の大まかなAIの遷移関数とステート----------//
#pragma region TransitionAiFunc
protected:
    //-----待機に遷移-----//
    void AiTransitionIdle() override;

    //-----移動に遷移-----//
    void AiTransitionMove() override;

    //-----攻撃に遷移-----//
    void AiTransformAttack() override;

#pragma endregion
};
