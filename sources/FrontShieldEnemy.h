#pragma once

#pragma once
#include"BaseEnemy.h"
#include "EnemiesEditor.h"
#include"EventFlag.h"
class ShieldEnemy final :public BaseEnemy
{
public:
    //****************************************************************
    // 
    // 構造体
    // 
    //****************************************************************
    struct DivedState
    {
        inline static const char* Start = "Start";
        inline static const char* Move = "Move";
        inline static const char* ShieldReady = "ShieldReady"; //シールドを構える
        inline static const char* ShieldAttack = "ShieldAttack"; // 振り上げ
        inline static const char* Shield = "Shield";  // ため
        inline static const char* Damaged = "Damaged";
        inline static const char* Die = "Die";
        inline static const char* Stun = "Stun";
    };
    enum  AnimationName {
        idle,
        move,
        shield_ready,
        shield_Attack,
        shield,
        stun,
        damage,
        die
    };
public:
    //****************************************************************
    // 
    // 関数
    // 
    //****************************************************************
    ShieldEnemy(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_/*スポーン位置*/,
        const EnemyParamPack& ParamPack_);
    ShieldEnemy(GraphicsPipeline& Graphics_);
    ~ShieldEnemy() override = default;

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fRegisterFunctions() override; // ステートを登録
    void fUpdateAttackCapsule() override;

    bool fDamaged(int Damage_, float InvincibleTime_, 
        GraphicsPipeline& Graphics_, float elapsedTime_) ;

private:
    bool fJudge_in_view() const;

    //****************************************************************
    // 
    // 変数
    // 
    //****************************************************************
private:
    float mWaitTimer{}; // 待ち時間
    bool is_shield{};

    std::unique_ptr<Effect> mShieldEffect{ nullptr };
    inline static const char* mkShieldPath = "./resources/Effect/shield.efk";
    //****************************************************************
    // 
    // 定数 
    // 
    //****************************************************************
    const float mMoveSpeed{ 10.0f };      // 移動速度
    const float mDifenceRange{ 40.0f };    // 攻撃範囲
    const float mAttackDelaySec{ 1.0f };  // 攻撃後の隙の長さ（秒）
    const float mSpawnDelaySec{ 1.0f };   // 登場後の長さ（秒）
    const float mViewingAngle{ 60.0f };   // 視野角

    //--------------------<各ステートの待ち時間>--------------------//
    const float mShieldReadySec{ 3.0f };          // 盾を構える
private:
    //****************************************************************
    // 
    // ステートマシン
    // 
    //****************************************************************
    //--------------------<剣の敵の共通の動き>--------------------//
    // スポーン
    void fSpawnInit();
    void fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 歩き
    void fMoveInit();
    void fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 盾を構える
    void fShieldReadyInit();
    void fShieldReadyUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //盾攻撃
    void fShieldAttackInit();
    void fShieldAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 盾防ぎ
    void fShieldInit();
    void fShieldUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // スタン
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // しぬ
    void fDieInit();
    void fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
	void fSetStun(bool Arg_, bool IsJust_) override;


};

