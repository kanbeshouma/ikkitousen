#pragma once
#include"BaseEnemy.h"
#include<functional>
#include "Common.h"
#include "EnemiesEditor.h"
//****************************************************************
// 
// 盾なし通常攻撃の雑魚敵の派生クラス 
// 
//****************************************************************
class ArcherEnemy final :public BaseEnemy
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
        inline static const char* AttackReady = "AttackReady";
        inline static const char* AttackIdle = "AttackIdle";
        inline static const char* AttackShot = "AttackShot";
        inline static const char* Approach = "Approach";
        inline static const char* Leave = "Leave";
        inline static const char* Die = "Die";
        inline static const char* Stun = "Stun";
    };
    enum  AnimationName {
        idle,
        walk,
        attack_ready,
        attack_idle,
        attack_shot,
        damage,
    };
    //****************************************************************
    // 
    // 関数
    // 
    //****************************************************************
public:

    ArcherEnemy(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_,
        EnemyParamPack ParamPack_);
    ArcherEnemy(GraphicsPipeline& Graphics_);

    ~ArcherEnemy() override = default;

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;

    void fGuiMenu() ;

    //void fDamaged(int damage_, float InvinsibleTime_) override;
    void fMove(float elapsed_time);
private:
    // ステートマシンを追加する関数
    void fRegisterFunctions() override;
    //パラメータ初期化関数
    void fParamInitialize();

    //--------------------<各ステートの関数>--------------------//
    void fSpawnInit();   // 登場の初期化
    void fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); // 登場の更新処理

    void fIdleInit();   // 待機の初期化
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); // 待機の更新処理

    void fMoveInit(); //移動の初期化
    void fmoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //移動の更新処理

	void fMoveApproachInit(); //接近移動の初期化
    void fMoveApproachUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //後退移動の更新処理

    void fMoveLeaveInit(); //接近移動の初期化
    void fMoveLeaveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //後退移動の更新処理



    // 弓を引く
    void fAttackBeginInit();
    void fAttackBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //弓引いて待機
    void fAttackPreActionInit();
    void fAttackPreActionUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //弓を放つ
    void fAttackEndInit();
    void fAttackEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDamagedInit(); //被ダメの初期化
    void fDamagedUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //被ダメの更新処理

    //スタン
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fSetStun(bool Arg_, bool IsJust_ = false) override;
   
private:  
    //****************************************************************
   // 　
   // 変数 
   // 
   //****************************************************************
    DivedState mNowState;
    float mStayTimer;
    float mAttackingTime;
    bool mAttack_flg;
    AddBulletFunc mfAddFunc;
    skeleton::bone mVernierBone{  };


    //****************************************************************
   // 　
   // 定数 
   // 
   //****************************************************************
    //登場後の待機時間
    const float SPAWN_STAY_TIME = 2.0f;
    //プレイヤーに攻撃を開始する最短距離
    const float AT_SHORTEST_DISTANCE = 30.0f;
    //プレイヤーに攻撃を開始する最長距離
    const float AT_LONGEST_DISTANCE = 80.0f;
    //アイドル状態になった時の待機時間
    const float IDLE_STAY_TIME = 2.0f;
    //移動時間（一定時間移動したらアイドル状態に）
    const float MOVE_TIME = 3.0f;
    //移動スピード
    const float MAX_MOVE_SPEED = 3.0f;
    //回転スピード
    const float ROT_SPEED = 10.0f;

};

