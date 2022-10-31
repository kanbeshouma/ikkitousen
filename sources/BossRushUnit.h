#pragma once
#include"BaseEnemy.h"

class BossRushUnit final :public BaseEnemy
{
    enum AnimationName
    {
        Idle,
        BeginChange,
        Rush,
        EndChange
    };


    struct DivideState
    {
        inline static constexpr const char* None = "None";
        inline static constexpr const char* Start = "Start";
        inline static constexpr const char* Change = "Change";
        inline static constexpr const char* Rush = "Rush";
        inline static constexpr const char* End = "End";
    };

    enum AiState
    {
        None,
        Start,
        Change,
        RushAi,
        End,
    };


public:
    BossRushUnit(GraphicsPipeline& Graphics_);
    BossRushUnit(GraphicsPipeline& Graphics_,const DirectX::XMFLOAT3& EntryPosition);
    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;

    // ディゾルブで現れる
    void fStartAppear(const DirectX::XMFLOAT3& EntryPosition);

    bool fGetIsStart()const;

    void fSetEnemyState(int state) override;

private:
    DirectX::XMFLOAT3 mVelocity{};
    bool mIsStart{};

protected:
    void fRegisterFunctions() override;

    //ステートマシン
    // 何もしない
    void fNoneInit();
    void fNoneUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    // 出現
    void fAppearInit();
    void fAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 変形
    void fChangeInit();
    void fChangeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 突進
    void fRushInit();
    void fRushUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // 終了
    void fEndInit();
    void fEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //----------敵の大まかなAIの遷移関数とステート----------//
#pragma region TransitionAiFunc
protected:
    //-----待機に遷移-----//
    void AiTransitionIdle() override {};

    //-----移動に遷移-----//
    void AiTransitionMove() override {};

    //-----攻撃に遷移-----//
    void AiTransformAttack() override {};

#pragma endregion
};