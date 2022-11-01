#pragma once
#include"BaseEnemy.h"
//****************************************************************
//
// ���̓G�̃G�[�X
//
//****************************************************************
class SpearEnemy_Ace final : public BaseEnemy
{
    struct DivideState
    {
        inline static const char* Start       = "Start";
        inline static const char* Idle        = "Idle";
        inline static const char* Move        = "Move";
        inline static const char* WipeReady   = "WipeReady";   // �ガ��������
        inline static const char* WipeBegin   = "WipeBegin";   // �ガ�����͂���
        inline static const char* WipeAttack  = "WipeAttack";  // �ガ������
        inline static const char* WipeEnd     = "WipeEnd";     // �ガ�����I���
        inline static const char* ThrustBegin = "ThrustBegin"; // �˂��͂���
        inline static const char* Thrust      = "Thrust";      // �˂�
        inline static const char* ThrustEnd   = "ThrustEnd";   // �˂��I���
        inline static const char* Stun        = "Stun";        // �X�^��

    };
    enum AiState
    {
        Start,
        Idle,
        Move,
        WipeBegin,
        WipeAttack,
        WipeEnd,
        ThrustBegin,
        Thrust,
        Stun,
    };

    enum AnimationName {
        idle,
        walk,
        attack_idle,
        attack_up,
        attack_down,
        damage,
        ace_attack_ready,
        ace_attack_idle,
        ace_attack_start,
        ace_attack,
        ace_attack_end,
        stun,
    };
public:
    SpearEnemy_Ace(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_/*�X�|�[���ʒu*/,
        const EnemyParamPack& ParamPack_);
    SpearEnemy_Ace(GraphicsPipeline& Graphics_);

    ~SpearEnemy_Ace() override = default;
    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fUpdateAttackCapsule() override;
    void fSetEnemyState(int state) override;

protected:
    void fRegisterFunctions() override;

private:
    float mWaitTimer{}; // �ҋ@����
    skeleton::bone mpSpearBoneBottom{};
    skeleton::bone mpSpearBoneTop{};
    const float mAttackLength{ 8.0f }; // �v���C���[�Ƃ̋��������̋����ȉ��ɂȂ�����

private:
    // �X�e�[�g�}�V���̊֐��Q
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fMoveInit();
    void fMoveUpdate( float elapsedTime_, GraphicsPipeline& Graphics_);

    void fWipeReadyInit();
    void fWipeReadyUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fWipeBeginInit();
    void fWipeBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fWipeAttackInit();
    void fWipeAttackUpdate( float elapsedTime_, GraphicsPipeline& Graphics_);

    void fStunInit();
    void fStunUpdate(float elapsedTime_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;
    //----------�G�̑�܂���AI�̑J�ڊ֐��ƃX�e�[�g----------//
#pragma region TransitionAiFunc
protected:
    //-----�ҋ@�ɑJ��-----//
    void AiTransitionIdle() override;

    //-----�ړ��ɑJ��-----//
    void AiTransitionMove() override;

    //-----�U���ɑJ��-----//
    void AiTransformAttack() override;

#pragma endregion
};
