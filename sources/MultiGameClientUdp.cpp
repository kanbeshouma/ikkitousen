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
        char data[256]{};
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
        //-----�v���C���[�̃��C���f�[�^-----//
    case UpdateCommand::PlayerMoveCommand:
    {
        //-----�f�[�^���L���X�g-----//
        PlayerMoveData* p_data = (PlayerMoveData*)data;

        //-----�f�[�^��ۑ�-----//
        receive_all_data.player_main_data.emplace_back(*p_data);
        break;
    }
    default:
        break;
    }
}