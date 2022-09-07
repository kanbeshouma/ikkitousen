#pragma once
#include <random>
#include"BaseEnemy.h"

class TutorialEnemy_NoAttack final :public BaseEnemy
{
    struct DivideState
    {
        inline static const char* Start = "Start";
        inline static const char* Idle = "Idle";
        inline static const char* Stun = "Stun";
        inline static const char* Move = "Move";
    };
    enum  AnimationName {
        idle,
        walk,
        damage,
        stun,
        attack_ready,
        attack,
    };

public:
    TutorialEnemy_NoAttack(GraphicsPipeline& graphics,
        const DirectX::XMFLOAT3& entry_position, const EnemyParamPack& param);

    TutorialEnemy_NoAttack(GraphicsPipeline& graphics);

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;
protected:
    void fRegisterFunctions() override;


private:
    float mTimer{};
    DirectX::XMFLOAT3 mWonderTarget{};
    DirectX::XMFLOAT3 mWonderBegin{};
    std::mt19937 mt{ std::random_device{}() };
private:
    // NoMoveは待機・スタン
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fMoveInit();
    void fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;
};

inline void TutorialEnemy_NoAttack::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivideState::Stun);
    }
}



class TutorialEnemy final :public BaseEnemy
{
    struct DivideState
    {
        inline static const char* Start = "Start";
        inline static const char* Idle = "Idle";
        inline static const char* AttackStart = "AttackStart";
        inline static const char* Attack = "Attack";
        inline static const char* Stun = "Stun";
        inline static const char* Move = "Move";
    };
    enum AnimationName {
        idle,
        walk,
        damage,
        stun,
        attack_ready,
        attack,
    };

public:
    TutorialEnemy(GraphicsPipeline& graphics,
        const DirectX::XMFLOAT3& entry_position, const EnemyParamPack& param);

    TutorialEnemy(GraphicsPipeline& graphics);

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;
protected:
    void fRegisterFunctions() override;


private:
    float mTimer{};
    DirectX::XMFLOAT3 mWonderTarget{};
    DirectX::XMFLOAT3 mWonderBegin{};
    std::mt19937 mt{ std::random_device{}() };
    skeleton::bone bone{};
private:
    //  NoMoveは待機・スタン
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fMoveInit();
    void fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fAttackStartInit();
    void fAttackStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fAttackInit();
    void fAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);


    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;
};

inline void TutorialEnemy::fSetStun(bool Arg_, bool IsJust_)
{
    if (!mIsStun)
    {
        mIsStun = Arg_;
        fChangeState(DivideState::Stun);
    }
}


