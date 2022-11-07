#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include "SceneMultiGameHost.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"


void SceneMultiGameHost::ReceiveUdpData()
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

void SceneMultiGameHost::CheckDataCommand(char com, char* data)
{
    std::lock_guard<std::mutex> lock(mutex);

    switch (com)
    {
        //-----�v���C���[�̃��C���f�[�^-----//
    case UpdateCommand::PlayerMoveCommand:
    {
        //-----�f�[�^���L���X�g-----//
        PlayerMoveData* p_data = (PlayerMoveData*)data;
        //-----�f�[�^��ۑ�-----//
        receive_all_data.player_move_data.emplace_back(*p_data);
        break;
    }
    case UpdateCommand::PlayerPositionCommand:
    {
        //-----�f�[�^���L���X�g-----//
        PlayerPositionData* p_data = (PlayerPositionData*)data;
        //-----�f�[�^��ۑ�-----//
        receive_all_data.player_position_data.emplace_back(*p_data);
        break;
    }
    case UpdateCommand::PlayerActionCommand:
    {
        //-----�v���C���[�̂ǂ̃A�N�V�������ǂ������`�F�b�N-----//
        CheckPlayerActionCommand(data[ComLocation::DataKind],data);
        break;
    }
    //-----�v���C���[�̗̑͂̃f�[�^-----//
    case UpdateCommand::PlayerHealthCommand:
    {
        //-----�f�[�^���L���X�g-----//
        PlayerHealthData* p_data = (PlayerHealthData*)data;
        //-----�f�[�^��ۑ�-----//
        receive_all_data.player_health_data.emplace_back(*p_data);
        break;
    }
    //-----�G�̏�ԃf�[�^-----//
    case UpdateCommand::EnemyConditionCommand:
    {
        using namespace EnemySendData;
        EnemyConditionData* e_data = (EnemyConditionData*)data;

        receive_all_enemy_data.enemy_condition_data.emplace_back(*e_data);
        break;
    }
    //-----�G�̃_���[�W�f�[�^-----//
    case UpdateCommand::EnemyDamageCommand:
    {
        using namespace EnemySendData;
        EnemyDamageData* e_data = (EnemyDamageData*)data;

        receive_all_enemy_data.enemy_damage_data.emplace_back(*e_data);
        break;
    }
    default:
        break;
    }
}


void SceneMultiGameHost::CheckPlayerActionCommand(char com, char* data)
{
    //-----�f�[�^���L���X�g-----//
    PlayerActionData* p_data = (PlayerActionData*)data;
    //-----�f�[�^��ۑ�-----//
    receive_all_data.player_action_data.emplace_back(*p_data);

    DebugConsole::Instance().WriteDebugConsole("���͏�����M", TextColor::White);

}