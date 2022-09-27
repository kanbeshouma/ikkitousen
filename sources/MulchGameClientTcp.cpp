#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"SceneMulchGameClient.h"
#include"Correspondence.h"
#include"SocketCommunication.h"
#include"NetWorkInformationStucture.h"

void SceneMulchGameClient::ReceiveLoginData()
{
    CoInitializeEx(NULL, NULL);
    DebugConsole::Instance().WriteDebugConsole("���O�C���X���b�h�J�n");
    for (;;)
    {
        if (end_login_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("���O�C���X���b�h���I��");
            break;
        }
        char data[256]{};
        int size = sizeof(data);

        if (CorrespondenceManager::Instance().TcpClientReceive(data, size) > 0)
        {
            //-----�R�}���h�����O�C���̏ꍇ
            if (data[0] == CommandList::Login)
            {
                std::lock_guard<std::mutex> lock(mutex);

                SendClientLoginData* login_data = (SendClientLoginData*)data;

                //-----�v���C���[�̒ǉ��t���O��ID��ݒ�-----//
                register_player = true;
                register_player_id = login_data->new_client_id;

                //-----�V�����ǉ������v���C���[��ID�o�^����-----//
                if (CorrespondenceManager::Instance().GetOpponentPlayerId().at(login_data->new_client_id) < 0)
                {
                    CorrespondenceManager::Instance().GetOpponentPlayerId().at(login_data->new_client_id) = login_data->new_client_id;
                    SocketCommunicationManager::Instance().game_udp_server_addr[login_data->new_client_id] = login_data->addr;
                }
            }
        }
    }

    CoUninitialize();

}

