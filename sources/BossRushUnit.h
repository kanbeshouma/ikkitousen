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

    // �f�B�]���u�Ō����
    void fStartAppear(const DirectX::XMFLOAT3& EntryPosition);

    bool fGetIsStart()const;

    void fSetEnemyState(int state) override;

private:
    DirectX::XMFLOAT3 mVelocity{};
    bool mIsStart{};

protected:
    void fRegisterFunctions() override;

    //�X�e�[�g�}�V��
    // �������Ȃ�
    void fNoneInit();
    void fNoneUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    // �o��
    void fAppearInit();
    void fAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // �ό`
    void fChangeInit();
    void fChangeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // �ːi
    void fRushInit();
    void fRushUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // �I��
    void fEndInit();
    void fEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

};