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
        char data[512]{};
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
                CheckDataCommand(data[ComLocation::UpdateCom], data,id);
                break;
            default:
                break;
            }

        }
    }
    CoUninitialize();
}

void SceneMultiGameHost::CheckDataCommand(char com, char* data,int id)
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
    case UpdateCommand::PlayerActionCommand:
    {
        //-----プレイヤーのどのアクションかどうかをチェック-----//
        CheckPlayerActionCommand(data[ComLocation::DataKind],data);
        break;
    }
    //-----プレイヤーの体力のデータ-----//
    case UpdateCommand::PlayerHealthCommand:
    {
        //-----データをキャスト-----//
        PlayerHealthData* p_data = (PlayerHealthData*)data;
        //-----データを保存-----//
        receive_all_data.player_health_data.emplace_back(*p_data);
        break;
    }
    //-----チェイン攻撃の敵の番号データ-----//
    case UpdateCommand::ChainAttackLockOnEnemy:
    {
        //-----敵の番号データのサイズを取得-----//
        int e_data_size = data[ComLocation::DataKind];

        //-----データをずらす-----//
        data += 3;

        //-----データ分配列を確保する-----//
        std::vector<char> d_vec;

        d_vec.resize(e_data_size);

        DebugConsole::Instance().WriteDebugConsole("敵番号受信", TextColor::Pink);

        for (int i = 0; i < e_data_size; i++)
        {
            d_vec.at(i) = (char)*data;
            data += sizeof(char);
            std::string  t = std::to_string(d_vec.at(i));
            DebugConsole::Instance().WriteDebugConsole(t, TextColor::Pink);
        }

        //-----データ設定-----//
        chain_rock_on_enemy_id.insert(std::make_pair(id, d_vec));

        break;
    }
    //-----敵の基本データ-----//
    case UpdateCommand::EnemiesMoveCommand:
    {
        EnemySendData::EnemiesMoveData* e_data = new EnemySendData::EnemiesMoveData;

        //-----どの敵の種類のデータか取得-----//
        e_data->cmd[ComLocation::DataKind] = data[ComLocation::DataKind];

        //-----vector型のサイズを取得
        int size = data[ComLocation::Other];
        e_data->cmd[ComLocation::Other] = size;

        //----データをコマンド分ずらす-----//
        data += 4;

        e_data->enemy_data.resize(size);

        for (int i = 0; i < size; i++)
        {
            //-----データをキャストする-----//
            e_data->enemy_data.at(i) = *(EnemySendData::EnemyData*)data;

            //-----キャストした構造体分メモリをずらす-----//
            data += sizeof(EnemySendData::EnemyData);
        }

        //-----データを設定-----//
        receive_all_enemy_data.enemy_move_data.emplace_back(*e_data);

        break;
    }
    //-----敵の状態データ-----//
    case UpdateCommand::EnemyConditionCommand:
    {
        using namespace EnemySendData;
        EnemyConditionData* e_data = (EnemyConditionData*)data;

        receive_all_enemy_data.enemy_condition_data.emplace_back(*e_data);
        break;
    }
    //-----敵のダメージデータ-----//
    case UpdateCommand::EnemyDamageCommand:
    {
        using namespace EnemySendData;
        EnemyDamageData* e_data = (EnemyDamageData*)data;

        receive_all_enemy_data.enemy_damage_data.emplace_back(*e_data);
        break;
    }
    //-----敵の死亡データ-----//
    case UpdateCommand::EnemyDieCommand:
    {
        //-----データをキャスト-----//
        EnemySendData::EnemyDieData* d = (EnemySendData::EnemyDieData*)data;
        //-----データを保存----//
        receive_all_enemy_data.enemy_die_data.emplace_back(*d);
        break;
    }
    default:
        break;
    }
}


void SceneMultiGameHost::CheckPlayerActionCommand(char com, char* data)
{
    //-----データをキャスト-----//
    PlayerActionData* p_data = (PlayerActionData*)data;
    //-----データを保存-----//
    receive_all_data.player_action_data.emplace_back(*p_data);

    DebugConsole::Instance().WriteDebugConsole("入力情報を受信", TextColor::White);

}