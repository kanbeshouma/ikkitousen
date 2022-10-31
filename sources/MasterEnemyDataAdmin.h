#pragma once
#include<map>
#include<tuple>
#include"BaseEnemy.h"

//-----�G�̃��[�_�[�̏����Ǘ����Ă���N���X-----//

class MasterEnemyDataAdmin
{
public:
    MasterEnemyDataAdmin() {}
    ~MasterEnemyDataAdmin();

public:
    //-----���[�_�[���-----//
    struct MasterData
    {
        DirectX::XMFLOAT3 position{};
        int target_id{ -1 };
        int ai_state{ -1 };
    };
private:
     using MasterDataMap = std::map<int, MasterData>;
    ////-----���[�_�[���̃f�[�^-----//
    //========================
    //key : �O���[�v�ԍ� : data : ���̃O���[�v�̃��[�_�[�f�[�^
     MasterDataMap master_data_map;

public:
    ////-----�}�X�^�[�f�[�^�̐ݒ�-----//
    //========================
    //������ : �O���[�v�ԍ�
    //������ : �����̈ʒu
    //��O���� : �^�[�Q�b�g���Ă���v���C���[�̔ԍ�
    //��l���� : ������AI
    void SetMasterData(int group, DirectX::XMFLOAT3 pos, int target_id, int ai_state);

    ////-----�}�X�^�[�f�[�^�̎擾-----//
    //=========================
    //�߂�l : std::tuple<�f�[�^�����������ǂ���,�}�X�^�[�f�[�^>(false : �f�[�^���� true : �f�[�^�L��)
    //������ : �O���[�v�ԍ�
    std::tuple<bool, MasterData> GetMasterData(int group);
};