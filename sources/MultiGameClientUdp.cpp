#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include "SceneMultiGameClient.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"

void SceneMultiGameClient::ReceiveUdpData()
{
    CoInitializeEx(NULL, NULL);
    DebugConsole::Instance().WriteDebugConsole("UDP�X���b�h�J�n");
    for (;;)
    {
        if (end_udp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("UDP�X���b�h���I��");
            break;
        }
        char data[512]{};
        int size = sizeof(data);

        //-----�f�[�^����M-----//
        int id = CorrespondenceManager::Instance().UdpReceive(data, size);

        if (id >= 0)
        {
            //-----�R�}���h�̊m�F-----//
            switch (data[ComLocation::ComList])
            {
            case CommandList::Update:
                //-----�f�[�^�̎�ނ̊m�F-----//
                CheckDataCommand(data[ComLocation::UpdateCom], data);
                break;
            default:
                break;
            }

        }
    }
    CoUninitialize();
}


void SceneMultiGameClient::CheckDataCommand(char com, char* data)
{
    std::lock_guard<std::mutex> lock(mutex);

    switch (com)
    {
    //-----�v���C���[�̓����f�[�^-----//
    case UpdateCommand::PlayerMoveCommand:
    {
        //-----�f�[�^���L���X�g-----//
        PlayerMoveData* p_data = (PlayerMoveData*)data;

        //-----�f�[�^��ۑ�-----//
        receive_all_player_data.player_move_data.emplace_back(*p_data);
        break;
    }
    //-----�v���C���[�̈ʒu�f�[�^-----//
    case UpdateCommand::PlayerPositionCommand:
    {
        //-----�f�[�^���L���X�g-----//
        PlayerPositionData* p_data = (PlayerPositionData*)data;
        //-----�f�[�^��ۑ�-----//
        receive_all_player_data.player_position_data.emplace_back(*p_data);
        break;
    }
    //-----�v���C���[�̃A�N�V�����f�[�^-----//
    case UpdateCommand::PlayerActionCommand:
    {
        //-----�v���C���[�̂ǂ̃A�N�V�������ǂ������`�F�b�N-----//
        CheckPlayerActionCommand(data[ComLocation::DataKind], data);
        break;
    }
    //-----�v���C���[�̍U������-----//
    case UpdateCommand::PlayerAttackResultCommand:
    {
        //-----�f�[�^���L���X�g-----//
        PlayerAttackResultData* p_data = (PlayerAttackResultData*)data;
        //-----�f�[�^��ۑ�-----//
        receive_all_player_data.player_attack_result_data.emplace_back(*p_data);
        break;
    }
    //-----�G�̏o���f�[�^-----//
    case UpdateCommand::EnemySpawnCommand:
    {
        //-----�f�[�^���L���X�g-----//
        EnemySendData::EnemySpawnData* s = (EnemySendData::EnemySpawnData*)data;
        //-----�f�[�^��ۑ�-----//
        receive_all_enemy_data.enemy_spawn_data.emplace_back(*s);

        break;
    }
    //-----�G�̊�{�f�[�^-----//
    case UpdateCommand::EnemiesMoveCommand:
    {
        EnemySendData::EnemiesMoveData* e_data = new EnemySendData::EnemiesMoveData;

        //-----�ǂ̓G�̎�ނ̃f�[�^���擾-----//
        e_data->cmd[ComLocation::DataKind] = data[ComLocation::DataKind];

        //-----vector�^�̃T�C�Y���擾
        int size = data[ComLocation::Other];
        e_data->cmd[ComLocation::Other] = size;

        //----�f�[�^���R�}���h�����炷-----//
        data += 4;

        e_data->enemy_data.resize(size);

        for (int i = 0; i < size; i++)
        {
            //-----�f�[�^���L���X�g����-----//
            e_data->enemy_data.at(i) = *(EnemySendData::EnemyData*)data;

            //-----�L���X�g�����\���̕������������炷-----//
            data += sizeof(EnemySendData::EnemyData);
        }

        //-----�f�[�^��ݒ�-----//
        receive_all_enemy_data.enemy_move_data.emplace_back(*e_data);

        break;
    }

    default:
        break;
    }
}


void SceneMultiGameClient::CheckPlayerActionCommand(char com, char* data)
{
    //-----�f�[�^���L���X�g-----//
    PlayerActionData* p_data = (PlayerActionData*)data;
    //-----�f�[�^��ۑ�-----//
    receive_all_player_data.player_action_data.emplace_back(*p_data);

    DebugConsole::Instance().WriteDebugConsole("���͏�����M", TextColor::White);

}