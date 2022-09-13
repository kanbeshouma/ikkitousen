#pragma once
#include<vector>
#include<memory>
#include<DirectXMath.h>
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
    //マッチング終了かつtcp通信切断
    MachingEnd,
    //ログアウト
    Logout,
    //更新
    Update,
    //IPアドレス取得
    IpAddress,
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


////------マッチング終了に送るデータ-----//
struct MachingEndData
{
    //通信コマンド
    char cmd[4]{};
};
