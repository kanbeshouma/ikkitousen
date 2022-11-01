#pragma once
#include"BaseEnemy.h"
#include "EnemiesEditor.h"
#include"EventFlag.h"
class SwordEnemy final :public BaseEnemy
{
public:
    //****************************************************************
    //
    // �\����
    //
    //****************************************************************
     struct DivedState
     {
         inline static const char* Start = "Start";
         inline static const char* Idle = "Idle";
         inline static const char* Move = "Move";
         inline static const char* Damaged = "Damaged";
         inline static const char* AttackBegin = "AttackBegin"; // �U��グ
         inline static const char* AttackRun = "AttackRun"; // �U��グ
         inline static const char* AttackMiddle = "AttackMiddle";  // ����
         inline static const char* AttackEnd = "AttackEnd"; // �U�艺�낵
         inline static const char* Die = "Die";
         inline static const char* Escape = "Escape";
         inline static const char* Stun = "Stun";
     };
     enum AiState
     {
         Start,
         Idle,
         Move,
         Damaged,
         AttackBegin,
         AttackRun,
         AttackMiddle,
         AttackEnd,
         Die,
         Escape,
         Stun,
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
    // �֐�
    //
    //****************************************************************
    SwordEnemy(GraphicsPipeline& Graphics_,
              const DirectX::XMFLOAT3& EmitterPoint_/*�X�|�[���ʒu*/,
              const EnemyParamPack& ParamPack_);
    SwordEnemy(GraphicsPipeline& Graphics_);
    ~SwordEnemy() override = default;

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fRegisterFunctions() override; // �X�e�[�g��o�^
    void fUpdateAttackCapsule() override;
    void fSetEnemyState(int state) override;
   //****************************************************************
   //
   // �ϐ�
   //
   //****************************************************************
private:
    float mWaitTimer{}; // �҂�����
    skeleton::bone mSwordBone{};
    float mMoveTimer{};
    //****************************************************************
    //
    // �萔
    //
    //****************************************************************
#pragma region Constant
    const float mMoveSpeed{ 30.0f };      // �ړ����x
    const float mAttackRange{ 60.0f };    // �U���͈�
    const float mAttackDelaySec{ 1.0f };  // �U����̌��̒����i�b�j
    const float mSpawnDelaySec{ 1.0f };   // �o���̒����i�b�j

    //--------------------<�e�X�e�[�g�̑҂�����>--------------------//
    const float mAttackBeginTimeSec{ 0.85f };    // ����U�肠����܂ł̎���
    const float mAttackPreActionTimeSec{ 0.1f };   // ����U�艺�낷�\������
    const float mAttackDownSec{ 1.0f };          // ����U�艺�낷
    const float mMoveTimeLimit{ 2.0f };          // �v���C���[�Ɍ������̂ƃv���C���[����ނ���������

#pragma endregion
private:
    //****************************************************************
    //
    // �X�e�[�g�}�V��
    //
    //****************************************************************
    //--------------------<���̓G�̋��ʂ̓���>--------------------//
    // �X�|�[��
    void fSpawnInit();
    void fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //�ҋ@
    void fIdleInit();
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // ����
    void fWalkInit();
    void fWalkUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // ����U��グ��
    void fAttackBeginInit();
    void fAttackBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fAttackRunInit();
    void fAttackRunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // ����U�艺�낷�\������
    void fAttackPreActionInit();
    void fAttackPreActionUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);;

    // �U��
    void fAttackEndInit();
    void fAttackEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fEscapeInit();
    void fEscapeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // ����
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // ����
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



