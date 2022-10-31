#pragma once
#include<map>
#include<tuple>
#include"BaseEnemy.h"

//-----敵のリーダーの情報を管理しているクラス-----//

class MasterEnemyDataAdmin
{
public:
    MasterEnemyDataAdmin() {}
    ~MasterEnemyDataAdmin();

public:
    //-----リーダー情報-----//
    struct MasterData
    {
        DirectX::XMFLOAT3 position{};
        int target_id{ -1 };
        int ai_state{ -1 };
    };
private:
     using MasterDataMap = std::map<int, MasterData>;
    ////-----リーダー情報のデータ-----//
    //========================
    //key : グループ番号 : data : そのグループのリーダーデータ
     MasterDataMap master_data_map;

public:
    ////-----マスターデータの設定-----//
    //========================
    //第一引数 : グループ番号
    //第二引数 : 自分の位置
    //第三引数 : ターゲットしているプレイヤーの番号
    //第四引数 : 自分のAI
    void SetMasterData(int group, DirectX::XMFLOAT3 pos, int target_id, int ai_state);

    ////-----マスターデータの取得-----//
    //=========================
    //戻り値 : std::tuple<データがあったかどうか,マスターデータ>(false : データ無し true : データ有り)
    //第一引数 : グループ番号
    std::tuple<bool, MasterData> GetMasterData(int group);
};