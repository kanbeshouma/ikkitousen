#pragma once
#include"BaseEnemy.h"
#include "EnemiesEditor.h"

class SpearEnemy final  : public BaseEnemy
{
    //****************************************************************
    // 
    //  構造体
    // 
    //****************************************************************
    struct DivedState 
    {
        inline static const char* Start = "Start";
        inline static const char* Idle = "Idle";
        inline static const char* Move = "Move";
        inline static const char* Damaged = "Damaged";
        inline static const char* ThrustBegin = "ThrustBegin";    // 突進前の構え
        inline static const char* ThrustMiddle = "ThrustMiddle";  // 突進中
        inline static const char* ThrustEnd = "ThrustEnd";        // 突進後の隙
        inline static const char* Die = "Die";
        inline static const char* Stun = "Stun";
    };
    enum AnimationName {
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
    SpearEnemy(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_/*スポーン位置*/,
        const EnemyParamPack& ParamPack_);
    SpearEnemy(GraphicsPipeline& Graphics_);
    ~SpearEnemy() override = default;

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
protected:
    void fRegisterFunctions() override;
private:
    void fUpdateAttackCapsule() override;;
private:
    //****************************************************************
    // 
    //  変数
    // 
    //****************************************************************
    float mWaitTimer{}; // 待機時間
    DirectX::XMFLOAT3 mThrustTarget{}; // 突進中のターゲット
private:
    //****************************************************************
    // 
    // 定数
    // 
    //****************************************************************
    const float mIdleSec{ 4.0f };        // 待機時間
    const float mStartSec{ 1.0f };        // 待機時間
    const float mAttackLength{ 100.0f }; // プレイヤーとの距離がこの距離以下になったら
    const float mMoveSpeed{ 20.0f };     // 移動速度
    const float mThrustBeginSec{ 1.0f }; // 突進準備の時間
    const float mThrustMiddleSec{ 0.2f };// 槍を突き出す長さ
    const float mThrustEndSec{ 3.0f };   // 突進中の長さ
    const float mThrustSpeed{ 70.0f };   // 突進の速さ
    const float mThrustDegree{ 30.0f };  // 突進の角度


private:
    //****************************************************************
    // 
    // ステートマシン
    // 
    //****************************************************************
    //--------------------<開始時>--------------------//
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<待機>--------------------//
    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<移動>--------------------//
    void fMoveInit();
    void fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<突き予備動作>--------------------//
    void fThrustBeginInit();
    void fThrustBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<突き中>--------------------//
    void fThrustMiddleInit();
    void fThrustMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<突きの後隙>--------------------//
    void fThrustEndInit();
    void fThrustEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<ダメージ>--------------------//
    void fDamageInit();
    void fDamageUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<スタン>--------------------//
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<死亡>--------------------//
    void fDieInit();
    void fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;
    
};

