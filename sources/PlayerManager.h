#pragma once
#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include<memory>
#include<vector>

#include"BasePlayer.h"
#include"EnemyManager.h"
#include"graphics_pipeline.h"
#include"BulletManager.h"
#include"post_effect.h"
#include"NetWorkInformationStucture.h"

//===================================//
////----------�v���C���[���Ǘ�����N���X----------//
//===================================//
class PlayerManager
{
public:
    PlayerManager();
    ~PlayerManager();
public:
    ////----------�X�V�֐�----------//
    void Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);

    ////----------�X�V�֐�----------//
    void PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);

    ////-----�R���t�B�O�̃����_�[//
    void ConfigRender(GraphicsPipeline& graphics, float elapsed_time);

    ////-----�`��֐�-----//
    void Render(GraphicsPipeline& graphics, float elapsed_time);
public:
    ////-----�v���C���[�o�^�֐�-----//
    void RegisterPlayer(BasePlayer* player);

    ////-----�v���C���[���폜-----//
    //=====================
    //��1���� : �폜����v���C���[��ID
    void DeletePlayer(int id);

    ////-----�v���C���[�̍s���͈͂�ݒ�-----//
    void ChangePlayerJustificationLength();

    ////-----�X�e�[�W�ړ��J�n-----//
    void TransitionStageMove();

    ////-----�X�e�[�W�ړ��I��-----//
    void TransitionStageMoveEnd();

    ////-----�W���X�g����������ǂ���-----//
    bool GetIsJustAvoidance();

    ////-----�v���C���[�̈ʒu�擾-----//
    //=========================
    //�߂�l : �o�^����Ă���v���C���[�̃I�u�W�F�N�g�ԍ��ƈʒu
    //std::tuple<object_id,position>
    std::vector<std::tuple<int,DirectX::XMFLOAT3>> GetPosition();

    ////-----�`�F�C���U���̍쐬���Ԓ����ǂ���-----//
    bool DuringSearchTime();

    ////-----�v���C���[�������Ă��邩�ǂ���-----//
    bool GetIsAlive();

    ////-----�G�ƃv���C���[�̍U���̓����蔻��-----//
    void PlayerAttackVsEnemy(EnemyManager* enemy_manager,GraphicsPipeline& graphics,float elapsed_time);

    ////-----�W���X�g������\���ǂ����̓����蔻��-----//
    void PlayerCounterVsEnemyAttack(EnemyManager* enemy_manager);

    ////-----�G�̍U���Ƃ̓����蔻��-----//
    void EnemyAttackVsPlayer(EnemyManager* enemy_manager);

    ////-----�N���C�A���g�����b�N�I�����Ă���G����������-----//
    void SearchClientPlayerLockOnEnemy(EnemyManager* enemy_manager);

    ////-----�G�̒e�ƃv���C���[�̓����蔻��-----//
    void BulletVsPlayer(BulletManager& bullet_manager);

    ////-----�W���X�g����������͈̔̓X�^���̓����蔻��-----//
    void PlayerStunVsEnemy(EnemyManager* enemy_manager);

    ////-----�`�F�C���U�����ɍs�����~������-----//
    void SetPlayerChainTime(EnemyManager* enemy_manager);

    ////-----�{�X�̃J�����ɐ؂�ւ���-----//
    void SetBossCamera(bool flag);

    ////-----�J�����̕�����ݒ肷��-----//
    void SetCameraDirection(DirectX::XMFLOAT3 f, DirectX::XMFLOAT3 r);

    ////-----�J������view�s��ƃv���W�F�N�V�����s���ݒ肷��-----//
    void SetCameraView(const DirectX::XMFLOAT4X4 view);
    void SetCameraProjection(const DirectX::XMFLOAT4X4 projection);

    ////-----���b�N�I���������̃|�X�g�G�t�F�N�g��ݒ肷��-----//
    void LockOnPostEffect(float elapsed_time,PostEffect* post_efc);

    ////-----�v���C���[�ɃJ�����̈ʒu��ݒ肷��-----//
    void SetCameraPosition(DirectX::XMFLOAT3 eye);

    ////-----��ԋ߂��G��ݒ肷��-----//
    void SetTarget(BaseEnemy* enemy);

    ////-----�J�����̃^�[�Q�b�g��ݒ肷��-----//
    void SetCameraTarget(DirectX::XMFLOAT3 target);

    ////-----�_�b�V���G�t�F�N�g��������-----//
    void DashPostEffect(GraphicsPipeline& graphics,PostEffect* post_efc);

    ////-----���̒[���̃v���C���[�̃^�[�Q�b�g�ɂ��Ă���G���擾-----//
    BaseEnemy* GetTargetEnemy();

    ////-----���̒[���̃v���C���[�����b�N�I���������ǂ���-----//
    bool GetEnemyLockOn();

    ////-----���̒[���̃v���C���[�̉�荞�݉�����ł��邩�ǂ����̃t���O���擾-----//
    bool GetBehaindCharge();
public:
    ////-----�v���C���[�̈ʒu��ݒ肷��-----//
    void SetPlayerPosition(DirectX::XMFLOAT3 pos);

    ////-----���̒[���̃v���C���[�̈ʒu���擾-----//
    DirectX::XMFLOAT3 GetMyTerminalPosition();

    ////-----���̒[���̃v���C���[�̑O�������擾����-----//
    DirectX::XMFLOAT3 GetMyTerminalForward();

    ////-----���̒[���̃v���C���[�̃C�x���g�J�����̃W���C���g���擾-----//
    DirectX::XMFLOAT3 GetMyTerminalJoint();

    ////-----���̒[���̃v���C���[�̃C�x���g�J�����̈ʒu���擾����-----//
    DirectX::XMFLOAT3 GetMyTerminalEye();

    ////-----���̒[���̃v���C���[�̃C�x���g���[�V�������I��������ǂ������擾����-----//
    bool GetMyTerminalEndClearMotion();

    ////-----�f�[�^��ݒ肷��-----//
    //=====================
    //��1���� : �f�[�^
    void SetPlayerMoveData(PlayerMoveData data);

    ////-----�f�[�^��ݒ肷��-----//
    //=====================
    //��1���� : �f�[�^
    void SetPlayerPositionData(PlayerPositionData data);

    ////-----�f�[�^��ݒ肷��-----//
    //=====================
    //��1���� : �f�[�^
    void SetPlayerActionData(PlayerActionData data);

    ////-----�f�[�^��ݒ肷��-----//
    //=====================
    //��1���� : �f�[�^
    void SetPlayerPlayerAttackResultData(PlayerAttackResultData data);

private:
    ////-----�v���C���[������ϐ�-----//
    std::vector<std::shared_ptr<BasePlayer>> players;

    ////-----�폜����v���C���[������ϐ�-----//
    std::vector<std::shared_ptr<BasePlayer>> remove_players;
private:
    //-----���̒[���ő��삷�邱�Ƃ��ł���v���C���[�̃I�u�W�F�N�g�ԍ�-----//
    int private_object_id{ -1 };
public:
    void SetPrivateObjectId(int id) { private_object_id = id; }
     int GetPrivatePlayerId() { return private_object_id; }
};