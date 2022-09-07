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
        inline static const char* WipeBegin   = "WipeBegin";   // 薙ぎ払いはじめ
        inline static const char* WipeAttack  = "WipeAttack";  // 薙ぎ払い中
        inline static const char* WipeEnd     = "WipeEnd";     // 薙ぎ払い終わり
        inline static const char* ThrustBegin = "ThrustBegin"; // 突きはじめ
        inline static const char* Thrust      = "Thrust";      // 突き
        inline static const char* ThrustEnd   = "ThrustEnd";   // 突き終わり
        inline static const char* Stun        = "Stun";        // スタン

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
  
protected:
    void fRegisterFunctions() override;

private:
    float mWaitTimer{}; // 待機時間
    skeleton::bone mpSpearBoneBottom{};
    skeleton::bone mpSpearBoneTop{};

private:
    // ステートマシンの関数群
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fMoveInit();
    void fMoveUpdate( float elapsedTime_, GraphicsPipeline& Graphics_);

    void fWipeBeginInit();
    void fWipeBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fWipeAttackInit();
    void fWipeAttackUpdate( float elapsedTime_, GraphicsPipeline& Graphics_);

    void fStunInit();
    void fStunUpdate(float elapsedTime_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;
};
