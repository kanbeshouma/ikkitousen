#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>
#include"SocketCommunication.h"
#include"game_pad.h"
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
    PlayerMainCommand,

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


////------�}�b�`���O�I���ɑ���f�[�^-----//
struct MachingEndData
{
    //�ʐM�R�}���h
    char cmd[4]{};
};

//-----�v���C���[�����铮���ƃ^�[�Q�b�g�̃f�[�^-----//
struct PlayerMainData
{
    //�ʐM�R�}���h
    char cmd[4]{};

    //-----���͕���-----//
    DirectX::XMFLOAT3 move_vec{};

    //-----�{�^���̓���-----//
    GamePadButton new_button_state = 0;

    //-----���b�N�I�����Ă���G�̔ԍ�-----//
    int lock_on_enemy_id{ -1 };

    //-----���b�N�I�����Ă邩�ǂ���-----//
    bool lock_on{ false };
};
