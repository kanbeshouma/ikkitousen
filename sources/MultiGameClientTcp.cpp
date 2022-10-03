#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"SceneMultiGameClient.h"
#include"Correspondence.h"
#include"SocketCommunication.h"
#include"NetWorkInformationStucture.h"

void SceneMultiGameClient::ReceiveTcpData()
{
    CoInitializeEx(NULL, NULL);
    DebugConsole::Instance().WriteDebugConsole("TCP�X���b�h�J�n");
    for (;;)
    {
        if (end_tcp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("TCP�X���b�h���I��");
            break;
        }
        char data[256]{};
        int size = sizeof(data);

        if (CorrespondenceManager::Instance().TcpClientReceive(data, size) > 0)
        {
            //-----�R�}���h�����O�C���̏ꍇ-----//
            switch (data[0])
            {
            case  CommandList::Login:
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
                break;
            }
            case CommandList::Logout:
            {
                DebugConsole::Instance().WriteDebugConsole("�N���C�A���g : ���O�A�E�g�f�[�^����M");
                std::lock_guard<std::mutex> lock(mutex);


                LogoutData* logout_data = (LogoutData*)data;
                //-----����ID�������Ȃ�X���b�h�I���t���O�𗧂Ă�-----//
                if (logout_data->id == CorrespondenceManager::Instance().GetOperationPrivateId())
                {
                    end_tcp_thread = true;
                }
                //------����ȊO�Ȃ玩���ȊO�����O�A�E�g���邩�烍�O�A�E�g�f�[�^������-----//
                else
                {
                    //-----���O�A�E�g����v���C���[��ID��ۑ�-----//
                    logout_id.emplace_back(logout_data->id);
                }
                break;
            }
            default:
                std::string text = "�R�}���h :" + std::to_string(data[0]);
                DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                break;
            }
        }
    }

    CoUninitialize();

}

