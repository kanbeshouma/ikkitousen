#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"SceneMultiGameClient.h"
#include"Correspondence.h"
#include"SocketCommunication.h"
#include"NetWorkInformationStucture.h"

void SceneMultiGameClient::ReceiveTcpData()
{
    CoInitializeEx(NULL, NULL);
    DebugConsole::Instance().WriteDebugConsole("TCPスレッド開始");
    for (;;)
    {
        if (end_tcp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("TCPスレッドを終了");
            break;
        }
        char data[256]{};
        int size = sizeof(data);

        if (CorrespondenceManager::Instance().TcpClientReceive(data, size) > 0)
        {
            //-----コマンドがログインの場合-----//
            switch (data[ComLocation::ComList])
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
                DebugConsole::Instance().WriteDebugConsole("クライアント : ログアウトデータを受信");
                std::lock_guard<std::mutex> lock(mutex);


                LogoutData* logout_data = (LogoutData*)data;
                //-----もしIDが自分ならスレッド終了フラグを立てる-----//
                if (logout_data->id == CorrespondenceManager::Instance().GetOperationPrivateId())
                {
                    end_tcp_thread = true;
                }
                //------それ以外なら自分以外がログアウトするからログアウトデータを入れる-----//
                else
                {
                    //-----ログアウトするプレイヤーのIDを保存-----//
                    logout_id.emplace_back(logout_data->id);
                }
                break;
            }
            default:
                std::string text = "コマンド :" + std::to_string(data[ComLocation::ComList]);
                DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                break;
            }
        }
    }

    CoUninitialize();

}

