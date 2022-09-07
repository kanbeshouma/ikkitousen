#pragma once
#include "BaseEnemy.h"
//****************************************************************
// 
// 剣の中ボス 
// 
//****************************************************************
class SwordEnemy_Ace final :public BaseEnemy
{
    struct DivideState
    {
        inline static char* Start = "Start";
        inline static char* Idle = "Idle";
        inline static char* CounterStart = "CounterStart";
        inline static char* CounterMiddle = "CounterMiddle";
        inline static char* CounterAttack = "CounterAttack";
        inline static char* CounterEnd = "CounterEnd";
        inline static char* Move = "Move";
        inline static char* Stun = "Stun";
    };
    enum AnimationName {
        idle,
        walk,
        attack_ready,
        attack_move,
        attack_slush,
        damage,
        ace_attack_ready,
        ace_attack_idle,
        ace_attack,
        ace_attack_end,
        stun,
    };

public:
    //****************************************************************
    SwordEnemy_Ace(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_/*スポーン位置*/,
        const EnemyParamPack& ParamPack_);
    SwordEnemy_Ace(GraphicsPipeline& Graphics_);
    ~SwordEnemy_Ace() override;
    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;

private:
    void fRegisterFunctions() override;
    float mWaitTimer{};

private:
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fCounterStartInit();
    void fCounterStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fCounterMiddleInit();
    void fCounterMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fCounterAttackInit();
    void fCounterAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fCounterEndInit();
    void fCounterEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fMoveInit();
    void fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    bool fDamaged(int Damage_, float InvincibleTime_, GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fSetStun(bool Arg_, bool IsJust_) override;
private:
    bool mIsHit{};
    bool mIsWaitCounter{};
    skeleton::bone mBone{};
};
