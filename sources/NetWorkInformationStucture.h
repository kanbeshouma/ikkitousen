#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>
#include"game_pad.h"
#include"SocketCommunication.h"
#include"EnemyStructuer.h"
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

    //-----�G�̑��쌠�̏��n�����N�G�X�g����------//
    TransferEnemyControlRequest,
    //-----�G�̃z�X�g���̏��n���N�G�X�g�̌���-----//
    TransferEnemyControlResult,
    //-----�G�̃z�X�g����Ԃ�-----//
    ReturnEnemyControl,
    //-----�X�e�[�W�N���A-----//
    StageClear,
    //-----�Q�[���N���A-----//
    GameClear,
    //------�Q�[���I�[�o�[-----//
    GameOver
};

enum UpdateCommand
{
    //-----�v���C���[�̓����ƃ^�[�Q�b�g�Ɋւ���f�[�^-----//
    PlayerMoveCommand,

    //-----�v���C���[�̈ʒu�f�[�^-----//
    PlayerPositionCommand,

    //-----�v���C���[�̃A�N�V�����f�[�^-----//
    PlayerActionCommand,

    //-----�v���C���[�̗̑̓f�[�^-----//
    PlayerHealthCommand,

    //-----�G�̏o���f�[�^-----//
    EnemySpawnCommand,

    //-----�G�̊�{�f�[�^-----//
    EnemiesMoveCommand,

    //-----�G�̏�ԃf�[�^-----//
    EnemyConditionCommand,

    //-----�G�̃_���[�W�f�[�^-----//
    EnemyDamageCommand,

    //-----�G�̎��S�f�[�^-----//
    EnemyDieCommand,


};


//-----�R�}���h������z��̃f�[�^�̎��-----//
enum ComLocation
{
    //-----�R�}���h��0�Ԗ�-----//
    ComList,
    //-----�R�}���h��1�Ԗ�-----//
    UpdateCom,
    //-----�R�}���h��2�Ԗ�(�����̓v���C���[��G�ɂ���ē���f�[�^�͕ς���Ă���)-----//
    DataKind,
    //-----�R�}���h��3�Ԗ�(������2�ԖڈȊO�ł��ݒ肵�����ꍇ�Ɏg��)-----//
    Other
};



#pragma region Login
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

    //���O
    std::string name[MAX_CLIENT];
    //-----�F-----//
    int p_color[MAX_CLIENT];

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
    //-----���O-----//
    std::string name;
    //-----�v���C���[�̐F-----//
    int player_color{};
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
    //-----���O-----//
    std::string name{};
    //-----�v���C���[�̐F-----//
    int player_color{};
};

////-----���O�A�E�g���ɑ���f�[�^-----//
struct LogoutData
{
    //�ʐM�R�}���h
    char cmd[4]{};

    //������ID
    int id{ -1 };
};

#pragma endregion

////------�}�b�`���O�I���ɑ���f�[�^-----//
struct MachingEndData
{
    //�ʐM�R�}���h
    char cmd[4]{};
};

#pragma region Player
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

    //-----���͕���-----//
    DirectX::XMFLOAT3 move_vec{};

};

//-----�v���C���[�̃A�N�V�����f�[�^(�{�^�����͂ƈʒu�A��]�Ȃ�)-----//
struct PlayerActionData
{
    //�ʐM�R�}���h
    char cmd[4]{};

    //-----�v���C���[�̔ԍ�-----//
    int player_id{ -1 };

    //-----�ʒu-----//
    DirectX::XMFLOAT3 position{};

    //-----�`���[�W�|�C���g-----//
    DirectX::XMFLOAT3 charge_point{};

    //-----���͒l-----//
    DirectX::XMFLOAT3 move_vec{};

    //-----����-----//
    DirectX::XMFLOAT3 velocity{};

    //-----�{�^���̓���-----//
    GamePadButton new_button_state;
};

enum PlayerHealthEnum
{
    Damage = 2,
};

//-----�v���C���[�̗̑͂̓���-----//
struct PlayerHealthData
{

    //-----�f�[�^-----//
    //[0] : ComList
    //[1] : UpdateCom
    //[2] : damage(�����̒l�̓z�X�g���g�p����)
    char data[4]{};

    int health{};
};

//-----�v���C���[�̃f�[�^�\���̂��S�ē����Ă���-----//
struct PlayerAllDataStruct
{
    //-----�v���C���[�̓����f�[�^-----//
    std::vector<PlayerMoveData> player_move_data;

    //-----�v���C���[�̈ʒu�f�[�^-----//
    std::vector<PlayerPositionData> player_position_data;

    //-----�v���C���[�̃A�N�V�����f�[�^-----//
    std::vector<PlayerActionData> player_action_data;

    //-----�v���C���[�̗̑͂̃f�[�^-----//
    std::vector<PlayerHealthData> player_health_data;

};


#pragma endregion


namespace EnemySendData
{
#pragma region EnemySpawnData
    //-----�G�̏o���f�[�^�̒��̃O���[�v�p�̔z��̒��g-----//
    enum EnemySpawnGropeArray
    {
        //-----�}�X�^�[���ǂ���-----//
        Master,
        //-----�O���[�v�ԍ�-----//
        GropeId,
        //-----�z�X�g���n�D��x-----//
        Transfer,
    };

    //-----�G�̏o���f�[�^�̒��̔z��̓��e-----//
    enum EnemySpawnCmdArray
    {
        //-----cmd�̒�-----//
        EnemyId = 2,
        EnemyType = 3,
        //============//
    };

    //-----�G�̃X�|�[���f�[�^-----//
    struct EnemySpawnData
    {
        //�ʐM�R�}���h
        //[0] : �R�}���h
        //[1] : �X�|�[���f�[�^
        //[2] : enemy_id
        //[3] : enemy_type
        char cmd[4]{};

        //�o���ʒu�̔ԍ�
        DirectX::XMFLOAT3 emitter_point{};

        //[0] : ���[�_�[���ǂ���
        //[1] : �O���[�v�ԍ�
        //[2] : ���n�ԍ�
        char grope_data[3]{};
    };

#pragma endregion

#pragma region EnemyMoveData
    //-----EnemyData�Ŏg�p����z��̎��-----//
    enum EnemyDataArray
    {
        ObjectId,
        AiState,
        TargetId,
        Hitpoint,
    };

    //-----�G�̊�{�f�[�^-----//
    struct EnemyData
    {
        //-----�G�̔ԍ�-----//
        //==============
        //[0] : object_id
        //[1] : state(AI)
        //[2] : target_id
        //[3] : hitpoint
        char enemy_data[4];

        //-----�����̈ʒu-----//
        DirectX::XMFLOAT3 pos;
    };

    //-----�G�̊�{�f�[�^�������Ă���\����-----//
    struct EnemiesMoveData
    {
        //-----�ʐM�R�}���h-----//
        //=================
        //[0] : CommandList
        //[1] : UpdateCommand
        //[2] : DataKind(����G�̎��) :�����͎g�p���ĂȂ�
        //[3] : vector�^�̃T�C�Y
        char cmd[4]{};

        //-----�G�̃f�[�^-----//
        std::vector<EnemyData> enemy_data;
    };
#pragma endregion

#pragma region EnemyConditionData

    //-----�G�̏�Ԃ�enum-----//
    enum EnemyConditionEnum
    {
        Stun
    };

    //-----�G�̏�Ԃ̔z��̒��g-----//
    enum EnemyConditionArray
    {
        EnemyConditionObjectId = 2,
        EnemyCondition = 3,
    };

    //-----�G�̏󋵃f�[�^(�X�^���Ƃ�)-----//
    struct EnemyConditionData
    {
        //-----�f�[�^-----//
        //[0] : CommandList
        //[1] : UpdateCommand
        //[2] : �G�̔ԍ�
        //[3] : �ǂ̏�Ԃ�()
        char data[4]{};
    };
#pragma endregion

    //-----�G�̎��S�f�[�^-----//
    struct EnemyDieData
    {
        //-----�ʐM�R�}���h-----//
        char cmd[4]{};

        int object_id{ -1 };
    };

#pragma region EnemyDamageData
    //-----�G�̃_���[�W�f�[�^�̔z��̒��g-----//
    enum EnemyDamageCmdArray
    {
        DamageComEnemyId = 2,
        DamageComDamage = 3
    };

    //-----�G�̃_���[�W�f�[�^-----//
    struct EnemyDamageData
    {
        //=================
        //[0] : CommandList
        //[1] : UpdateCommand
        //[2] : �G�̔ԍ�
        //[3] : �^�����_���[�W
        char data[4]{};
    };

#pragma endregion

}
//-----�G�̃f�[�^�\���̂��S�ē����Ă���-----//
struct EnemyAllDataStruct
{
    //-----�G�̏o���f�[�^-----//
    std::vector<EnemySendData::EnemySpawnData> enemy_spawn_data;

    //-----�G�̊�{�f�[�^-----//
    std::vector<EnemySendData::EnemiesMoveData> enemy_move_data;

    //-----�G�̏�ԃf�[�^-----//
    std::vector<EnemySendData::EnemyConditionData> enemy_condition_data;

    //-----�G�̃_���[�W�f�[�^-----//
    std::vector<EnemySendData::EnemyDamageData> enemy_damage_data;

    //-----�G�̎��S�f�[�^-----//
    std::vector<EnemySendData::EnemyDieData> enemy_die_data;
};

namespace TransferEnemyControl
{
    //-----�f�[�^�z��̏ꏊ-----//
    enum DataArray
    {
        Result = 1
    };

    //-----����-----//
    enum Result
    {
        None,
        //-----����-----//
        Permit,
        //-----������-----//
        Prohibition
    };

    struct TransferEnemyResult
    {
        char data[2]{};
    };
}