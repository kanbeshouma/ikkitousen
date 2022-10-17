#pragma once

#pragma once
#include"BaseEnemy.h"
#include "EnemiesEditor.h"
#include"EventFlag.h"
class ShieldEnemy_Ace final :public BaseEnemy
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
        inline static const char* Move = "Move";
        inline static const char* ShieldReady = "ShieldReady"; //�V�[���h���\����
        inline static const char* ShieldAttack = "ShieldAttack"; // �U��グ
        inline static const char* Shield = "Shield";  // ����
        inline static const char* Damaged = "Damaged";
        inline static const char* Die = "Die";
        inline static const char* Stun = "Stun";
    };
    enum AiState
    {
        Start,
        Move,
        ShieldReady,
        ShieldAttack,
        Shield,
        Damaged,
        Die,
        Stun,
    };

    enum  AnimationName {
        idle,
        move,
        shield_ready,
        shield_Attack,
        shield,
        stun,
        damage,
        die
    };
public:
    //****************************************************************
    // 
    // �֐�
    // 
    //****************************************************************
    ShieldEnemy_Ace(GraphicsPipeline& Graphics_,
        const DirectX::XMFLOAT3& EmitterPoint_/*�X�|�[���ʒu*/,
        const EnemyParamPack& ParamPack_);
    ShieldEnemy_Ace(GraphicsPipeline& Graphics_);
    ~ShieldEnemy_Ace() override = default;

    void fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_) override;
    void fRegisterFunctions() override; // �X�e�[�g��o�^
    void fUpdateAttackCapsule() override;

    bool fDamaged(int Damage_, float InvincibleTime_,
        GraphicsPipeline& Graphics_, float elapsedTime_);

private:
    bool fJudge_in_view() const;

    //****************************************************************
    // 
    // �ϐ�
    // 
    //****************************************************************
private:
    float mWaitTimer{}; // �҂�����
    bool is_shield;
    std::unique_ptr<Effect> mShieldEffect{ nullptr };
    inline static const char* mkShieldPath = "./resources/Effect/shield.efk";

    //****************************************************************
    // 
    // �萔 
    // 
    //****************************************************************
    const float mMoveSpeed{ 10.0f };      // �ړ����x
    const float mDifenceRange{ 40.0f };    // �U���͈�
    const float mAttackDelaySec{ 1.0f };  // �U����̌��̒����i�b�j
    const float mSpawnDelaySec{ 1.0f };   // �o���̒����i�b�j
    const float mViewingAngle{ 60.0f };   // ����p

    //--------------------<�e�X�e�[�g�̑҂�����>--------------------//
    const float mShieldReadySec{ 3.0f };          // �����\����
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

    // ����
    void fMoveInit();
    void fMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // �����\����
    void fShieldReadyInit();
    void fShieldReadyUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    //���U��
    void fShieldAttackInit();
    void fShieldAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // ���h��
    void fShieldInit();
    void fShieldUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // �X�^��
    void fStunInit();
    void fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);

    // ����
    void fDieInit();
    void fDieUpdate(float elapsedTime_, GraphicsPipeline& Graphics_);
public:
    void fSetStun(bool Arg_, bool IsJust_) override;


};

