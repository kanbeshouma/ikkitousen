#pragma once
#include<memory>
#include<vector>

#include"BasePlayer.h"

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

    ////-----�`��֐�-----//
    void Render(GraphicsPipeline& graphics, float elapsed_time);
public:
    ////-----�v���C���[�o�^�֐�-----//
    void RegisterPlayer(BasePlayer* player);

    ////-----�X�e�[�W�ړ��J�n-----//
    void TransitionStageMove();

    ////-----�X�e�[�W�ړ��I��-----//
    void TransitionStageMoveEnd();

    ////-----�W���X�g����������ǂ���-----//
    bool GetIsJustAvoidance();

    ////-----�v���C���[�̈ʒu�擾-----//
    //=========================
    //�߂�l : �o�^����Ă���v���C���[�̈ʒu�̔z��
    std::vector<DirectX::XMFLOAT3> GetPosition();


private:
    ////-----�v���C���[������ϐ�-----//
    std::vector<std::shared_ptr<BasePlayer>> players;
private:
    //-----���̒[���ő��삷�邱�Ƃ��ł���v���C���[�̃I�u�W�F�N�g�ԍ�-----//
    int private_object_id{ -1 };
};