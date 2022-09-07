#pragma once
#include"BaseEnemy.h"
#include "EnemiesEditor.h"
#include"EventFlag.h"
class SwordEnemy final :public BaseEnemy
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
         inline static const char* Idle = "Idle";
         inline static const char* Move = "Move";
         inline static const char* Damaged = "Damaged";
         inline static const char* AttackBegin = "AttackBegin"; // 振り上げ
         inline static const char* AttackRun = "AttackRun"; // 振り上げ
         inline static const char* AttackMiddle = "AttackMiddle";  // ため
         inline static const char* AttackEnd = "AttackEnd"; // 振り下ろし
         inline static const char* Die = "Die";
         inline static const char* Escape = "Escape";
         inline static const char* Stun = "Stun";
     };
     enum  AnimationName {
         idle,
         walk,
         attack_idle,
         attack_up,
         attack_down,
         damage,
     };
public:
    //****************************************************************
    //
    // 関数
    //
    //****************************************************************
    SwordEnemy(GraphicsPipeline& Graphics_,
              const DirectX::XMFLOAT3& EmitterPoint_/*スポーン位置*/,
              const EnemyParamPack& ParamPack_);
    SwordEnemy(GraphicsPipeline& Graphics_);
    ~SwordEnemy() override = default;

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fRegisterFunctions() override; // ステートを登録
    void fUpdateAttackCapsule() override;

   //****************************************************************
   //
   // 変数
   //
   //****************************************************************
private:
    float mWaitTimer{}; // 待ち時間
    skeleton::bone mSwordBone{};
    float mMoveTimer{};
    //****************************************************************
    //
    // 定数
    //
    //****************************************************************
    const float mMoveSpeed{ 10.0f };      // 移動速度
    const float mAttackRange{ 60.0f };    // 攻撃範囲
    const float mAttackDelaySec{ 1.0f };  // 攻撃後の隙の長さ（秒）
    const float mSpawnDelaySec{ 1.0f };   // 登場後の長さ（秒）

    //--------------------<各ステートの待ち時間>--------------------//
    const float mAttackBeginTimeSec{ 0.85f };    // 剣を振りあげるまでの時間
    const float mAttackPreActionTimeSec{0.1f};   // 剣を振り下ろす予備動作
    const float mAttackDownSec{ 1.0f };          // 剣を振り下ろす
    const float mMoveTimeLimit{ 2.0f };          // プレイヤーに向かうのとプレイヤーから退く制限時間
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
    void fWalkInit();
    void fWalkUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 剣を振り上げる
    void fAttackBeginInit();
    void fAttackBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fAttackRunInit();
    void fAttackRunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 剣を振り下ろす予備動作
    void fAttackPreActionInit();
    void fAttackPreActionUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);;

    // 攻撃
    void fAttackEndInit();
    void fAttackEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fEscapeInit();
    void fEscapeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // やられ
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // やられ
    void fDieInit();
    void fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;
};



