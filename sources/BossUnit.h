#pragma once
#include <random>

#include"BaseEnemy.h"
#include"BulletManager.h"
#include"LaserBeam.h"
class BossUnit final : public BaseEnemy
{

    struct DivideState
    {
        inline static constexpr const char* Start = "Start";
        inline static constexpr const char* Idle = "Idle";
        inline static constexpr const char* Wonder = "Wonder";
        inline static constexpr const char* AttackBegin = "AttackBegin";
        inline static constexpr const char* AttackBeam = "AttackBeam";
        inline static constexpr const char* Stun = "Stun";
       
    };

    enum AnimationName
    {
        IDLE,
        MOVE,
        BEAM_CHARGE_START,
        BEAM_CHARGE_IDLE,
        BEAM_SHOOT_START,
        BEAM_SHOOT_IDLE,
        BEAM_END,
        DAMAGE,
        DIE,
        STUN,
    };
public:
    BossUnit(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EntryPoint_,
        const EnemyParamPack& ParamPack_,
        AddBulletFunc fAddBullet_);
    BossUnit(GraphicsPipeline& Graphics_);
    ~BossUnit()
    {
        mpBeamEffect->stop(effect_manager->get_effekseer_manager());
    }
    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;
   
    void fSetStun(bool Arg_, bool IsJust_) override;
protected:
    void fRegisterFunctions() override;

    void fSelectWonderOrAttack();
    void fResetLaser();
    void fGuiMenu();
private:
    float mTimer = 0.0f;
    std::mt19937 mt{ std::random_device{}() };
    DirectX::XMFLOAT3 mMoveVec{}; // 移動ベクトル
    float mMoveSpeed{20.0f};
    bool mMoveRev{};

    //--------------------<ビーム 予測線>--------------------//
    LaserBeam mWarningLine{};
    float mWarningThreshold{};
    float mBeamThreshold{};
    inline static bool msOnPlayer{ true };
    bool mOnPlayer{};
    DirectX::XMFLOAT3 TargetPos{};

    float mLifeTimer{};

    std::unique_ptr<Effect> mpBeamEffect{};

    const DirectX::XMFLOAT3 mStartPosition{};
    const float length = { 100.0f };
    const float mkIdleTime = 3.0f;
    const float mkWalkTime = 3.0f;
    const float mkChargeTime = 2.0f;
    const float mkLifeTime = 20.0f;
private:
    //--------------------<ステートマシン>--------------------//
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fWonderInit();
    void fWonderUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fAttackChargeInit();
    void fAttackChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fAttackBeamInit();
    void fAttackBeamUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fRender(GraphicsPipeline& Graphics_) override;
};
