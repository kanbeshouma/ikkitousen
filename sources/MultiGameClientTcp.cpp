#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"SceneMultiGameClient.h"
#include"Correspondence.h"
#include"SocketCommunication.h"
#include"NetWorkInformationStucture.h"
#include"EnemyManager.h"

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
        char data[512]{};
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
                register_player_id = login_data->cmd[static_cast<int>(SendClientLoginDataCmd::NewClientId)];
                register_player_color = login_data->cmd[static_cast<int>(SendClientLoginDataCmd::PlayerColor)];
                CorrespondenceManager::Instance().names[register_player_id] = login_data->name;

                //-----新しく追加したプレイヤーのID登録する-----//
                if (CorrespondenceManager::Instance().GetOpponentPlayerId().at(login_data->cmd[static_cast<int>(SendClientLoginDataCmd::NewClientId)]) < 0)
                {
                    CorrespondenceManager::Instance().GetOpponentPlayerId().at(login_data->cmd[static_cast<int>(SendClientLoginDataCmd::NewClientId)]) = login_data->cmd[static_cast<int>(SendClientLoginDataCmd::NewClientId)];
                    std::tuple<bool, sockaddr_in> d(false, login_data->addr);
                    SocketCommunicationManager::Instance().game_udp_server_addr[login_data->cmd[static_cast<int>(SendClientLoginDataCmd::NewClientId)]] = d;
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
            case CommandList::StartSendData:
            {
                std::lock_guard<std::mutex> lock(SocketCommunicationManager::Instance().GetMutex());
                auto& [check, ad] = SocketCommunicationManager::Instance().game_udp_server_addr[data[1]];
                check = true;

                break;
            }
            //-----敵の出現-----//
            case CommandList::EnemySpawnCommand:
            {
                std::lock_guard<std::mutex> lock(mutex);

#if 0
                //-----データをキャスト-----//
                EnemySendData::EnemySpawnData* s = (EnemySendData::EnemySpawnData*)data;
                //-----データを保存-----//
                receive_all_enemy_data.enemy_spawn_data.emplace_back(*s);

#endif // 0
                SetEnemySpawnData(data);

                DebugConsole::Instance().WriteDebugConsole("敵の出現",TextColor::Green);
                break;
            }
            //-----敵のホスト権の譲渡も結果-----//
            case CommandList::TransferEnemyControlResult:
            {
                std::lock_guard<std::mutex> lock(mutex);
                //------データを設定-----//
                memcpy_s(transfer_enemy_result.data, sizeof(transfer_enemy_result.data), data, sizeof(transfer_enemy_result.data));

                transfer_enemy_host_result = true;

                DebugConsole::Instance().WriteDebugConsole("譲渡結果のデータを受け取りました");
                break;
            }
            //-----ステージ番号を取得-----//
            case CommandList::CurrentStageNum:
            {
                std::lock_guard<std::mutex> lock(mutex);
                current_stage = static_cast<WaveManager::STAGE_IDENTIFIER>(data[1]);
                receive_stage_num = true;
                break;
            }
            //-----最終決定したステージの番号-----//
            case CommandList::EndResultNextStage:
            {
                receive_end_result_next_stage = true;
                //-----データを設定----//
                result_next_stage = static_cast<WaveManager::STAGE_IDENTIFIER>(data[1]);
                break;
            }
            //-----ステージクリア-----//
            case CommandList::StageClear:
            {
                std::lock_guard<std::mutex> lock(mutex);
                stage_situation = StageSituation::StageClearFlg;
                break;
            }
            //-----ステージリトライ-----//
            case CommandList::GameRetry:
            {
                std::lock_guard<std::mutex> lock(mutex);
                game_retry = true;
                break;
            }
            case CommandList::ReturnTitle:
            {
                std::lock_guard<std::mutex> lock(mutex);
                stage_situation = StageSituation::GameOverFlg;
                break;
            }
            //-----イベント終了-----//
            case CommandList::EndEvent:
            {
                std::lock_guard<std::mutex> lock(mutex);
                //-----どのイベントが終わったかを取得-----//
                end_event_data = true;
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

void SceneMultiGameClient::SetEnemySpawnData(char* data)
{
    using namespace EnemySendData;

    //配列のサイズを取得
    int size = data[static_cast<int>(SendEnemySpawnData::SpawnNum)];

    //----データをコマンド分ずらす-----//
    data += EnemyManager::SendSpawnEnemyDataComSize;

    for (int i = 0; i < size; i++)
    {
        receive_all_enemy_data.enemy_spawn_data.emplace_back(*(EnemySpawnData*)data);
        //-----キャストした構造体分メモリをずらす-----//
        data += sizeof(EnemySpawnData);

    }
    ////-----データをキャスト-----//
    //EnemySendData::EnemySpawnData* s = (EnemySendData::EnemySpawnData*)data;
    ////-----データを保存-----//
    //receive_all_enemy_data.enemy_spawn_data.emplace_back(*s);

}