#pragma once
#include"BaseEnemy.h"
#include "EnemiesEditor.h"

class SpearEnemy final  : public BaseEnemy
{
    //****************************************************************
    //
    //  �\����
    //
    //****************************************************************
    struct DivedState
    {
        inline static const char* Start = "Start";
        inline static const char* Idle = "Idle";
        inline static const char* Move = "Move";
        inline static const char* Damaged = "Damaged";
        inline static const char* ThrustBegin = "ThrustBegin";    // �ːi�O�̍\��
        inline static const char* ThrustMiddle = "ThrustMiddle";  // �ːi��
        inline static const char* ThrustEnd = "ThrustEnd";        // �ːi��̌�
        inline static const char* Die = "Die";
        inline static const char* Stun = "Stun";
    };
    enum AiState
    {
        Start,
        Idle,
        Move,
        Damaged,
        ThrustBegin,
        ThrustMiddle,
        ThrustEnd,
        Die,
        Stun,
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
    // �֐�
    //
    //****************************************************************
    SpearEnemy(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_/*�X�|�[���ʒu*/,
        const EnemyParamPack& ParamPack_);
    SpearEnemy(GraphicsPipeline& Graphics_);
    ~SpearEnemy() override = default;

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;

    void fSetEnemyState(int state) override;
protected:
    void fRegisterFunctions() override;
private:
    void fUpdateAttackCapsule() override;;
private:
    //****************************************************************
    //
    //  �ϐ�
    //
    //****************************************************************
    float mWaitTimer{}; // �ҋ@����
    DirectX::XMFLOAT3 mThrustTarget{}; // �ːi���̃^�[�Q�b�g
private:
    //****************************************************************
    //
    // �萔
    //
    //****************************************************************
#pragma region Constant
    const float mIdleSec{ 4.0f };        // �ҋ@����
    const float mStartSec{ 1.0f };        // �ҋ@����
    const float mAttackLength{ 100.0f }; // �v���C���[�Ƃ̋��������̋����ȉ��ɂȂ�����
    const float mMoveSpeed{ 30.0f };     // �ړ����x
    const float mThrustBeginSec{ 1.0f }; // �ːi�����̎���
    const float mThrustMiddleSec{ 0.2f };// ����˂��o������
    const float mThrustEndSec{ 3.0f };   // �ːi���̒���
    const float mThrustSpeed{ 70.0f };   // �ːi�̑���
    const float mThrustDegree{ 30.0f };  // �ːi�̊p�x

#pragma endregion


private:
    //****************************************************************
    //
    // �X�e�[�g�}�V��
    //
    //****************************************************************
    //--------------------<�J�n��>--------------------//
    void fStartInit();
    void fStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<�ҋ@>--------------------//
    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<�ړ�>--------------------//
    void fMoveInit();
    void fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<�˂��\������>--------------------//
    void fThrustBeginInit();
    void fThrustBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<�˂���>--------------------//
    void fThrustMiddleInit();
    void fThrustMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<�˂��̌㌄>--------------------//
    void fThrustEndInit();
    void fThrustEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<�_���[�W>--------------------//
    void fDamageInit();
    void fDamageUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<�X�^��>--------------------//
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
    //--------------------<���S>--------------------//
    void fDieInit();
    void fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
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

