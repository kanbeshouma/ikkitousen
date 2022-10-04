#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "SceneMultiGameHost.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"


void SceneMultiGameHost::ReceiveUdpData()
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

void SceneMultiGameHost::CheckDataCommand(char com, char* data)
{
    std::lock_guard<std::mutex> lock(mutex);

    switch (com)
    {
        //-----プレイヤーのメインデータ-----//
    case UpdateCommand::PlayerMoveCommand:
    {
        //-----データをキャスト-----//
        PlayerMoveData* p_data = (PlayerMoveData*)data;
        //-----データを保存-----//
        receive_all_data.player_move_data.emplace_back(*p_data);
        break;
    }
    case UpdateCommand::PlayerPositionCommand:
    {
        //-----データをキャスト-----//
        PlayerPositionData* p_data = (PlayerPositionData*)data;
        //-----データを保存-----//
        receive_all_data.player_position_data.emplace_back(*p_data);
        break;
    }
    default:
        break;
    }
}
