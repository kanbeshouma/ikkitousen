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
        //-----プレイヤーのメインデータ-----//
    case UpdateCommand::PlayerMainCommand:
    {
        //-----データをキャスト-----//
        PlayerMainData* p_data = (PlayerMainData*)data;

        //-----データを保存-----//
        receive_all_data.player_main_data.emplace_back(*p_data);
        break;
    }
    default:
        break;
    }
}