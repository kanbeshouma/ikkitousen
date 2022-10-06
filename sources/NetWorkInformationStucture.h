#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>
#include"game_pad.h"
#include"SocketCommunication.h"
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

};


//-----コマンドを入れる配列のデータの種類-----//
enum ComLocation
{
    //-----コマンドの0番目-----//
    ComList,
    //-----コマンドの1番目-----//
    UpdateCom,

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
};

//-----プレイヤーのアクションデータ(ボタン入力)-----//
struct PlayerActionData
{
    //通信コマンド
    char cmd[4]{};

    //-----プレイヤーの番号-----//
    int player_id{ -1 };

    //-----ボタンの入力-----//
    GamePadButton new_button_state;
};


struct AllDataStruct
{
    //-----プレイヤーの動きデータ-----//
    std::vector<PlayerMoveData> player_move_data;

    //-----プレイヤーの位置データ-----//
    std::vector<PlayerPositionData> player_position_data;

    //-----プレイヤーのアクションデータ-----//
    std::vector<PlayerActionData> player_action_data;

};