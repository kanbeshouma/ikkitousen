#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>
#include"game_pad.h"
#include"SocketCommunication.h"
#include"EnemyStructuer.h"
//---------------------------------------------------------------
//
//通信で必要なコマンド，構造体を宣言する
//
//---------------------------------------------------------------


//どのタイプのデータを送ったか
enum CommandList
{
    //ログイン
    Login = 1,
    //マッチング終了
    MachingEnd,
    //ログアウト
    Logout,
    //更新
    Update,
    //IPアドレス取得
    IpAddress,
};

enum UpdateCommand
{
    //-----プレイヤーの動きとターゲットに関するデータ-----//
    PlayerMoveCommand,

    //-----プレイヤーの位置データ-----//
    PlayerPositionCommand,

    //-----プレイヤーのアクションデータ-----//
    PlayerActionCommand,

    //-----プレイヤーの当たり判定(攻撃関連)のデータ-----//
    PlayerAttackResultCommand,

    //-----敵の出現データ-----//
    EnemySpawnCommand,

    //-----敵の基本データ-----//
    EnemiesMoveCommand,

    //-----敵の死亡データ-----//
    EnemyDieCommand,
};


//-----コマンドを入れる配列のデータの種類-----//
enum ComLocation
{
    //-----コマンドの0番目-----//
    ComList,
    //-----コマンドの1番目-----//
    UpdateCom,
    //-----コマンドの2番目(ここはプレイヤーや敵によって入るデータは変わってくる)-----//
    DataKind,
    //-----コマンドの3番目(ここは2番目以外でも設定したい場合に使う)-----//
    Other
};


////-----ログイン情報------//
struct LoginData
{
    //通信コマンド
    char cmd[4]{};
    //自分のID(何番目に保存されているか)
    int operation_private_id{ -1 };
    //この配列は接続しているプレイヤー(自分は含めない)のIDが入る
    int opponent_player_id[MAX_CLIENT];
    //今接続している(自分も含める)クライアントのアドレスを取得
    sockaddr_in game_udp_server_addr[MAX_CLIENT];

    //名前
    std::string name[MAX_CLIENT];
    //-----色-----//
    int p_color[MAX_CLIENT];

    //ホストの番号
    int host_id{ -1 };
};


////-----ホストに送るログインデータ-----//
struct SendHostLoginData
{
    //通信コマンド
    char cmd[4]{};
    ////クライアントの受信用のポート番号
    char port[8] = { "7000" };
    //-----名前-----//
    std::string name;
    //-----プレイヤーの色-----//
    int player_color{};
};


////-----今繋がっているクライアントに送るログインデータ-----//
struct SendClientLoginData
{
    //通信コマンド
    char cmd[4]{};
    //接続してきたクライアント番号
    int new_client_id{ -1 };
    //今接続している(自分も含める)クライアントのアドレスを取得
    sockaddr_in addr;
    //-----名前-----//
    std::string name{};
    //-----プレイヤーの色-----//
    int player_color{};
};

////-----ログアウト時に送るデータ-----//
struct LogoutData
{
    //通信コマンド
    char cmd[4]{};

    //自分のID
    int id{ -1 };
};

////------マッチング終了に送るデータ-----//
struct MachingEndData
{
    //通信コマンド
    char cmd[4]{};
};

//-----プレイヤーが送る動きとターゲットのデータ-----//
struct PlayerMoveData
{
    //通信コマンド
    char cmd[4]{};

    //-----プレイヤーの番号-----//
    int player_id{ -1 };

    //-----入力方向-----//
    DirectX::XMFLOAT3 move_vec{};

    //-----ロックオンしている敵の番号-----//
    int lock_on_enemy_id{ -1 };

    //-----ロックオンしてるかどうか-----//
    bool lock_on{ false };
};

//-----プレイヤーの位置データ-----//
struct PlayerPositionData
{
    //通信コマンド
    char cmd[4]{};

    //-----プレイヤーの番号-----//
    int player_id{ -1 };

    //-----位置-----//
    DirectX::XMFLOAT3 position{};

    //-----入力方向-----//
    DirectX::XMFLOAT3 move_vec{};

};

//-----プレイヤーのアクションデータ(ボタン入力と位置、回転など)-----//
struct PlayerActionData
{
    //通信コマンド
    char cmd[4]{};

    //-----プレイヤーの番号-----//
    int player_id{ -1 };

    //-----位置-----//
    DirectX::XMFLOAT3 position{};

    //-----チャージポイント-----//
    DirectX::XMFLOAT3 charge_point{};

    //-----入力値-----//
    DirectX::XMFLOAT3 move_vec{};

    //-----速力-----//
    DirectX::XMFLOAT3 velocity{};

    //-----ボタンの入力-----//
    GamePadButton new_button_state;
};

//-----プレイヤーの当たり判定(攻撃関連)のデータ-----//
struct PlayerAttackResultData
{
    //通信コマンド
    char cmd[4]{};

    //-----プレイヤーの番号-----//
    int player_id{ -1 };

    //-----コンボカウント-----//
    float combo_count{};

    //-----ブロックされたかどうか-----//
    bool block{ false };
};

//-----プレイヤーのデータ構造体が全て入っている-----//
struct PlayerAllDataStruct
{
    //-----プレイヤーの動きデータ-----//
    std::vector<PlayerMoveData> player_move_data;

    //-----プレイヤーの位置データ-----//
    std::vector<PlayerPositionData> player_position_data;

    //-----プレイヤーのアクションデータ-----//
    std::vector<PlayerActionData> player_action_data;

    //-----プレイヤーの攻撃当たり判定データ(結果が入る)-----//
    std::vector<PlayerAttackResultData> player_attack_result_data;

};


namespace EnemySendData
{
    //-----EnemyDataで使用する配列の種類-----//
    enum EnemyDataArray
    {
        ObjectId,
        AiState,
        TargetId,
        Hitpoint,
    };


    //-----敵のスポーンデータ-----//
    struct EnemySpawnData
    {
        //通信コマンド
        char cmd[4]{};

        //-----敵の番号-----//
        int enemy_id{ -1 };

        //出現タイミングを記録
        float spawn_timer{};

        //出現位置の番号
        DirectX::XMFLOAT3 emitter_point{};

        //敵の種類
        EnemyType type{};
    };

    //-----敵の基本データ-----//
    struct EnemyData
    {
        //-----敵の番号-----//
        //==============
        //[0] : object_id
        //[1] : state(AI)
        //[2] : target_id
        //[3] : hitpoint
        char enemy_data[4];

        //-----自分の位置-----//
        DirectX::XMFLOAT3 pos;
    };

    //-----敵の基本データが入っている構造体-----//
    struct EnemiesMoveData
    {
        //-----通信コマンド-----//
        //=================
        //[0] : CommandList
        //[1] : UpdateCommand
        //[2] : DataKind(送る敵の種類)
        //[3] : vector型のサイズ
        char cmd[4]{};

        //-----敵のデータ-----//
        std::vector<EnemyData> enemy_data;
    };

    //-----敵の死亡データ-----//
    struct EnemyDieData
    {
        //-----通信コマンド-----//
        char cmd[4]{};

        int object_id{ -1 };
    };
}
//-----敵のデータ構造体が全て入っている-----//
struct EnemyAllDataStruct
{
    //-----敵の出現データ-----//
    std::vector<EnemySendData::EnemySpawnData> enemy_spawn_data;

    //-----敵の基本データ-----//
    std::vector<EnemySendData::EnemiesMoveData> enemy_move_data;

    //-----敵の死亡データ-----//
    std::vector<EnemySendData::EnemyDieData> enemy_die_data;
};
