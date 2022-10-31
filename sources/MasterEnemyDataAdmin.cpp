#include "MasterEnemyDataAdmin.h"

MasterEnemyDataAdmin::~MasterEnemyDataAdmin()
{
}

void MasterEnemyDataAdmin::SetMasterData(int group, DirectX::XMFLOAT3 pos, int target_id, int ai_state)
{
    MasterDataMap::iterator it;
    //-----マップの中に引数と同じキー値があるかどうかを検索-----//
    it = master_data_map.find(group);

    //-----見つからなかった場合-----//
    if (it == master_data_map.end())
    {
        //-----マスターデータを設定して登録-----//
        MasterData data;

        data.position = pos;
        data.target_id = target_id;
        data.ai_state = ai_state;

        master_data_map.insert(std::make_pair(group, data));
    }
    //-----見つかった場合-----//
    else
    {
        //-----データを更新-----//
        it->second.position = pos;
        it->second.target_id = target_id;
        it->second.ai_state = ai_state;
    }
}

std::tuple<bool, MasterEnemyDataAdmin::MasterData> MasterEnemyDataAdmin::GetMasterData(int group)
{
    MasterDataMap::iterator it;
    //-----マップの中に引数と同じキー値があるかどうかを検索-----//
    it = master_data_map.find(group);
    MasterData data{};

    //-----見つからなかった場合-----//
    if (it == master_data_map.end())
    {
        //-----falseを設定して、なかったダミー情報を返す-----//
        return std::tuple<bool, MasterData>(false,data);
    }
    //-----見つかった場合-----//
    else
    {
        //-----trueを設定して、情報を返す-----//
        return std::tuple<bool, MasterData>(true,it->second);
    }

}

void MasterEnemyDataAdmin::ResetMasterData()
{
    master_data_map.clear();
}
