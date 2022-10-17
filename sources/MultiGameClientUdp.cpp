#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "SceneMultiGameClient.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"

void SceneMultiGameClient::ReceiveUdpData()
{
    CoInitializeEx(NULL, NULL);
    DebugConsole::Instance().WriteDebugConsole("UDPスレッド開始");
    for (;;)
    {
        if (end_udp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("UDPスレッドを終了");
            break;
        }
        char data[256]{};
        int size = sizeof(data);

        //-----データを受信-----//
        int id = CorrespondenceManager::Instance().UdpReceive(data, size);

        if (id >= 0)
        {
            //-----コマンドの確認-----//
            switch (data[ComLocation::ComList])
            {
            case CommandList::Update:
                //-----データの種類の確認-----//
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
    //-----プレイヤーの動きデータ-----//
    case UpdateCommand::PlayerMoveCommand:
    {
        //-----データをキャスト-----//
        PlayerMoveData* p_data = (PlayerMoveData*)data;

        //-----データを保存-----//
        receive_all_player_data.player_move_data.emplace_back(*p_data);
        break;
    }
    //-----プレイヤーの位置データ-----//
    case UpdateCommand::PlayerPositionCommand:
    {
        //-----データをキャスト-----//
        PlayerPositionData* p_data = (PlayerPositionData*)data;
        //-----データを保存-----//
        receive_all_player_data.player_position_data.emplace_back(*p_data);
        break;
    }
    //-----プレイヤーのアクションデータ-----//
    case UpdateCommand::PlayerActionCommand:
    {
        //-----プレイヤーのどのアクションかどうかをチェック-----//
        CheckPlayerActionCommand(data[ComLocation::DataKind], data);
        break;
    }
    //-----敵の出現データ-----//
    case UpdateCommand::EnemySpawnCommand:
    {
        //-----データをキャスト-----//
        EnemySendData::EnemySpawnData* s = (EnemySendData::EnemySpawnData*)data;
        //-----データを保存-----//
        receive_all_enemy_data.enemy_spawn_data.emplace_back(*s);

        break;
    }
    default:
        break;
    }
}


void SceneMultiGameClient::CheckPlayerActionCommand(char com, char* data)
{
    switch (com)
    {
    case PlayerActionKind::AvoidanceData:
    {
        //-----データをキャスト-----//
        PlayerActionData* p_data = (PlayerActionData*)data;
        //-----データを保存-----//
        receive_all_player_data.player_avoidance_data.emplace_back(*p_data);

        DebugConsole::Instance().WriteDebugConsole("入力情報を受信", TextColor::White);
        break;
    }
    default:
        break;
    }

}