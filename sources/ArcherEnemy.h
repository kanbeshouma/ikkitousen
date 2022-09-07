#pragma once
#include"BaseEnemy.h"
#include<functional>
#include "Common.h"
#include "EnemiesEditor.h"
//****************************************************************
// 
// ���Ȃ��ʏ�U���̎G���G�̔h���N���X 
// 
//****************************************************************
class ArcherEnemy final :public BaseEnemy
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
    // �֐�
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
    // �X�e�[�g�}�V����ǉ�����֐�
    void fRegisterFunctions() override;
    //�p�����[�^�������֐�
    void fParamInitialize();

    //--------------------<�e�X�e�[�g�̊֐�>--------------------//
    void fSpawnInit();   // �o��̏�����
    void fSpawnUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); // �o��̍X�V����

    void fIdleInit();   // �ҋ@�̏�����
    void fIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); // �ҋ@�̍X�V����

    void fMoveInit(); //�ړ��̏�����
    void fmoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //�ړ��̍X�V����

	void fMoveApproachInit(); //�ڋ߈ړ��̏�����
    void fMoveApproachUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //��ވړ��̍X�V����

    void fMoveLeaveInit(); //�ڋ߈ړ��̏�����
    void fMoveLeaveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //��ވړ��̍X�V����



    // �|������
    void fAttackBeginInit();
    void fAttackBeginUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //�|�����đҋ@
    void fAttackPreActionInit();
    void fAttackPreActionUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //�|�����
    void fAttackEndInit();
    void fAttackEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    void fDamagedInit(); //��_���̏�����
    void fDamagedUpdate(float elapsedTime_, GraphicsPipeline& Graphics_); //��_���̍X�V����

    //�X�^��
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fSetStun(bool Arg_, bool IsJust_ = false) override;
   
private:  
    //****************************************************************
   // �@
   // �ϐ� 
   // 
   //****************************************************************
    DivedState mNowState;
    float mStayTimer;
    float mAttackingTime;
    bool mAttack_flg;
    AddBulletFunc mfAddFunc;
    skeleton::bone mVernierBone{  };


    //****************************************************************
   // �@
   // �萔 
   // 
   //****************************************************************
    //�o���̑ҋ@����
    const float SPAWN_STAY_TIME = 2.0f;
    //�v���C���[�ɍU�����J�n����ŒZ����
    const float AT_SHORTEST_DISTANCE = 30.0f;
    //�v���C���[�ɍU�����J�n����Œ�����
    const float AT_LONGEST_DISTANCE = 80.0f;
    //�A�C�h����ԂɂȂ������̑ҋ@����
    const float IDLE_STAY_TIME = 2.0f;
    //�ړ����ԁi��莞�Ԉړ�������A�C�h����ԂɁj
    const float MOVE_TIME = 3.0f;
    //�ړ��X�s�[�h
    const float MAX_MOVE_SPEED = 3.0f;
    //��]�X�s�[�h
    const float ROT_SPEED = 10.0f;

};

