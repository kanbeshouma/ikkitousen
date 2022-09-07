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
public:
    BossRushUnit(GraphicsPipeline& Graphics_);
    BossRushUnit(GraphicsPipeline& Graphics_,const DirectX::XMFLOAT3& EntryPosition);
    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;

    // ディゾルブで現れる
    void fStartAppear(const DirectX::XMFLOAT3& EntryPosition);

    bool fGetIsStart()const;

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

};