#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"SceneMulchGameClient.h"
#include"Correspondence.h"
#include"SocketCommunication.h"
#include"NetWorkInformationStucture.h"

void SceneMulchGameClient::ReceiveTcpData()
{
    CoInitializeEx(NULL, NULL);
    DebugConsole::Instance().WriteDebugConsole("ログインスレッド開始");
    for (;;)
    {
        if (end_tcp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("ログインスレッドを終了");
            break;
        }
        char data[256]{};
        int size = sizeof(data);

        if (CorrespondenceManager::Instance().TcpClientReceive(data, size) > 0)
        {
            //-----コマンドがログインの場合-----//
            switch (data[0])
            {
            case  CommandList::Login:
            {
                std::lock_guard<std::mutex> lock(mutex);

                SendClientLoginData* login_data = (SendClientLoginData*)data;
                //-----プレイヤーの追加フラグとIDを設定-----//
                register_player = true;
                register_player_id = login_data->new_client_id;

                //-----新しく追加したプレイヤーのID登録する-----//
                if (CorrespondenceManager::Instance().GetOpponentPlayerId().at(login_data->new_client_id) < 0)
                {
                    CorrespondenceManager::Instance().GetOpponentPlayerId().at(login_data->new_client_id) = login_data->new_client_id;
                    SocketCommunicationManager::Instance().game_udp_server_addr[login_data->new_client_id] = login_data->addr;
                }
                break;
            }
            case CommandList::Logout:
            {
                std::lock_guard<std::mutex> lock(mutex);

                LogoutData* logout_data = (LogoutData*)data;
                //-----ログアウトするプレイヤーのIDを保存-----//
                logout_id.emplace_back(logout_data->id);

                break;
            }
            default:
                break;
            }
        }
    }

    CoUninitialize();

}

