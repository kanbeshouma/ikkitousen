#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>
#include"game_pad.h"
#include"SocketCommunication.h"
//---------------------------------------------------------------
//
//�ʐM�ŕK�v�ȃR�}���h�C�\���̂�錾����
//
//---------------------------------------------------------------


//�ǂ̃^�C�v�̃f�[�^�𑗂�����
enum CommandList
{
    //���O�C��
    Login = 1,
    //�}�b�`���O�I��
    MachingEnd,
    //���O�A�E�g
    Logout,
    //�X�V
    Update,
    //IP�A�h���X�擾
    IpAddress,
};

enum UpdateCommand
{
    //-----�v���C���[�̓����ƃ^�[�Q�b�g�Ɋւ���f�[�^-----//
    PlayerMoveCommand,

    //-----�v���C���[�̈ʒu�f�[�^-----//
    PlayerPositionCommand,

    //-----�v���C���[�̃A�N�V�����f�[�^-----//
    PlayerActionCommand,

};


//-----�R�}���h������z��̃f�[�^�̎��-----//
enum ComLocation
{
    //-----�R�}���h��0�Ԗ�-----//
    ComList,
    //-----�R�}���h��1�Ԗ�-----//
    UpdateCom,

};


////-----���O�C�����------//
struct LoginData
{
    //�ʐM�R�}���h
    char cmd[4]{};
    //������ID(���Ԗڂɕۑ�����Ă��邩)
    int operation_private_id{ -1 };
    //���̔z��͐ڑ����Ă���v���C���[(�����͊܂߂Ȃ�)��ID������
    int opponent_player_id[MAX_CLIENT];
    //���ڑ����Ă���(�������܂߂�)�N���C�A���g�̃A�h���X���擾
    sockaddr_in game_udp_server_addr[MAX_CLIENT];
    //�z�X�g�̔ԍ�
    int host_id{ -1 };
};


////-----�z�X�g�ɑ��郍�O�C���f�[�^-----//
struct SendHostLoginData
{
    //�ʐM�R�}���h
    char cmd[4]{};
    ////�N���C�A���g�̎�M�p�̃|�[�g�ԍ�
    char port[8] = { "7000" };
};


////-----���q�����Ă���N���C�A���g�ɑ��郍�O�C���f�[�^-----//
struct SendClientLoginData
{
    //�ʐM�R�}���h
    char cmd[4]{};
    //�ڑ����Ă����N���C�A���g�ԍ�
    int new_client_id{ -1 };
    //���ڑ����Ă���(�������܂߂�)�N���C�A���g�̃A�h���X���擾
    sockaddr_in addr;
};

////-----���O�A�E�g���ɑ���f�[�^-----//
struct LogoutData
{
    //�ʐM�R�}���h
    char cmd[4]{};

    //������ID
    int id{ -1 };
};

////------�}�b�`���O�I���ɑ���f�[�^-----//
struct MachingEndData
{
    //�ʐM�R�}���h
    char cmd[4]{};
};

//-----�v���C���[�����铮���ƃ^�[�Q�b�g�̃f�[�^-----//
struct PlayerMoveData
{
    //�ʐM�R�}���h
    char cmd[4]{};

    //-----�v���C���[�̔ԍ�-----//
    int player_id{ -1 };

    //-----���͕���-----//
    DirectX::XMFLOAT3 move_vec{};

    //-----���b�N�I�����Ă���G�̔ԍ�-----//
    int lock_on_enemy_id{ -1 };

    //-----���b�N�I�����Ă邩�ǂ���-----//
    bool lock_on{ false };
};


//-----�v���C���[�̈ʒu�f�[�^-----//
struct PlayerPositionData
{
    //�ʐM�R�}���h
    char cmd[4]{};

    //-----�v���C���[�̔ԍ�-----//
    int player_id{ -1 };

    //-----�ʒu-----//
    DirectX::XMFLOAT3 position{};
};

//-----�v���C���[�̃A�N�V�����f�[�^(�{�^������)-----//
struct PlayerActionData
{
    //�ʐM�R�}���h
    char cmd[4]{};

    //-----�v���C���[�̔ԍ�-----//
    int player_id{ -1 };

    //-----�{�^���̓���-----//
    GamePadButton new_button_state;
};


struct AllDataStruct
{
    //-----�v���C���[�̓����f�[�^-----//
    std::vector<PlayerMoveData> player_move_data;

    //-----�v���C���[�̈ʒu�f�[�^-----//
    std::vector<PlayerPositionData> player_position_data;

    //-----�v���C���[�̃A�N�V�����f�[�^-----//
    std::vector<PlayerActionData> player_action_data;

};