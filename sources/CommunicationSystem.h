#pragma once
#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include"SocketCommunication.h"
#include"DebugConsole.h"
#include"EnemyStructuer.h"
//-----------------------------------------------------------------------------------------------------------
//
//              データを送受信するための機能の初期化及び
//                                動きを管理するクラス
//
//-----------------------------------------------------------------------------------------------------------

class CommunicationSystem
{
public:
    CommunicationSystem();
    ~CommunicationSystem();
public:
    //------------------------初期化関係----------------------------//

    ////-----自分のIPアドレスを取得-------//
    void AcquisitionMyIpAddress();

#pragma region HostInitialize
    ////------ホストの場合の初期化------//
//===========================
//戻り値   : 成功したかどうか
//第1引数 : TCPポート番号
//第2引数 : UDPポート番号
//第3引数 : 自分の番号
    bool InitializeHost(char* tco_port, char* udp_port, int private_id);
private:
    ////------ホストの場合の受信用UDPソケットの初期化------//
    //===========================
    //戻り値   : 成功したかどうか
    //第1引数 : ポート番号
    //第2引数 : 自分の番号
    bool InitializeHostUdpSocket(char* port, int private_id);

    ////------ホストの場合のTCPの初期化------//
    //===========================
    //戻り値   : 成功したかどうか
    //第1引数 : ポート番号
    //第2引数 : 自分の番号
    bool InitializeHostTcp(char* port, int private_id);

#pragma endregion

public:

#pragma region ClientInitialize
    ////------クライアントの場合の初期化------//
//===========================
//戻り値   : 成功したかどうか
//第1引数 : ポート番号
    bool InitializeClient(char* tco_port, char* udp_port);
private:
    ////------クライアントの場合のTCPの初期化------//
    //===========================
    //戻り値   : 成功したかどうか
    //第1引数 : ポート番号
    bool InitializeClientTcp(char* port);

    ////------クライアントの場合の受信用UDPソケットの初期化------//
    //===========================
    //戻り値   : 成功したかどうか
    //第1引数 : ポート番号
    bool InitializeClientUdpSocket(char* port);

#pragma endregion

public:
    /// <summary>
    ///HTTPリクエスト
    /// </summary>
    int HttpRequest();
public:
    //--------------------------ログイン関係------------------------//

#pragma region Login
                    ////-----ログイン用の受信(TCP通信)-----//
    //==========================
    //戻り値   : 受信したデータのサイズ
    //第1引数 : 受信したデータを入れる変数
    //第2引数 : データを入れる変数のサイズ
    int LoginReceive(char* data, int size);

    ////-----ログイン用の送信(TCP通信)-----//
    //==========================
    //第1引数 : 送信するデータ
    //第2引数 : データのサイズ
    void LoginSend(char* data, int size);

#pragma endregion

public:
    //------------------------------送受信-----------------------------//
#pragma region TCP
    ////----------TCP通信接続確立----------//
    //==========================
    void TcpAccept(char* port);

    ////----------TCPホスト受信----------//
    //==========================
    //戻り値   : 受信した相手の番号を返す
    //第1引数 : 受信したデータを入れる変数
    //第2引数 : データを入れる変数のサイズ
    //第3引数 : 自分の番号
    int TcpHostReceive(char* data, int size,int operation_private_id);

    ////----------TCPクライアント受信----------//
    //==========================
    //戻り値   : 受信したデータのサイズを返す
    //第1引数 : 受信したデータを入れる変数
    //第2引数 : データを入れる変数のサイズ
    int TcpClientReceive(char* data, int size);

    ////----------TCP:特定の相手に送信----------//
    //==========================
    //第1引数 : 送りたい相手の番号
    //第2引数 : 送りたいデータ
    //第3引数 : 送るデータの型のサイズ
    void TcpSend(int id, char* data, int size);

    ////----------TCP:ホストに送信----------//
    //==========================
    //第2引数 : 送りたいデータ
    //第3引数 : 送るデータの型のサイズ
    void TcpSend(char* data, int size);

    ////----------TCP:クライアントに送信-----------//
    //==========================
    //第2引数 : 送りたいデータ
    //第3引数 : 送るデータの型のサイズ
    void TcpSendAllClient(char* data, int size);

    ////----------TCPホスト終了送信----------//
    //==========================
    //戻り値   : 全員にtcp通信終了のデータを送信して切断が成功したか
    //第1引数 : 送るデータ
    //第2引数 : 送るデータのサイズ
    //第3引数 : 自分の番号
    bool CloseTcpHost(char* data, int size,int operation_private_id);

    ////----------TCPクライアント終了----------//
    //==========================
    //戻り値   : 切断完了したかどうか
    bool CloseTcpClient();
#pragma endregion

#pragma region UDP
    ////----------UDP受信----------//
//==========================
//第1引数 : 送りたい相手の番号
//第1引数 : 受信したデータを入れる変数
//第2引数 : データを入れる変数のサイズ
    int UdpReceive(char* data, int size);

    ////----------UDP:特定の相手に送信----------//
    //==========================
    //第1引数 : 送りたい相手の番号
    //第2引数 : 送りたいデータ
    //第3引数 : 送るデータの型のサイズ
    void UdpSend(int id, char* data, int size);

    ////----------UDP:接続者全員に送信----------//
    //==========================
    //第1引数 : 送りたいデータ
    //第2引数 : 送るデータの型のサイズ
    void UdpSend(char* data, int size);

#pragma endregion

#pragma region MultiCast
    ////------マルチキャスト送信の場合の初期化------//
    //===========================
    //戻り値   : 成功したかどうか
    bool InitializeMultiCastSend();

    ////------マルチキャスト受信の場合の初期化------//
    //===========================
    //戻り値   : 成功したかどうか
    bool InitializeMultiCastReceive();

    ////----------マルチキャスト送信----------//
    //===========================
    //第1引数 : 渡すデータ
    //第2引数 : データ型のサイズ
    void MultiCastSend(char* data, int size);

    ////------マルチキャストの受信関数------//
    //===========================
    //第1引数 : 受信したデータの保存変数
    //第2引数 : 保存変数の大きさ
    bool MultiCastReceive(char* data, int size);
#pragma endregion


    ////----------ログアウトしたクライアントの削除----------//
    //==========================
    //第1引数 : ログアウトするクライアントの番号
    void LogoutClient(int client_id);

    ////-------ログインして来た相手のアドレスを使用して送り先のポート番号に合わせたアドレスを取得する-----//
    //===========================
    //戻り値   : ポート番号が一致しているアドレス
    //第1引数 : 受信したアドレス
    //第2引数 : 送り先のポート番号
    sockaddr_in SetSendPortAddress(sockaddr_in addr, char* port);

private:
    ////-------ログインするホストのアドレスを取得-----//
    //===========================
    //戻り値   : アドレスを取得できたかどうか
    //第1引数 : ポート番号;
    bool AcquisitionLoginAddress(char* port);

private:
    int tcp_error_num{ -1 };
    int udp_error_num{ -1 };
    const char* hostname = "localhost";
    const char* path = "/ikkiwebsite/ConversionJson/Registration";
    const char* http_port = "80";
    //const char* hostname = "localhost";
    //const char* path = "/ConversionJson/Registration";
    //const char* http_port = "5001";
    //<ヘッダ部分を取り除いたかどうか>//
    bool header_check = false;
    //<文字数を取得してその分を取り除いたかどうか>//
    bool char_num_check = false;
    //<文字数を入れる>//
    int char_count{};
    //合計サイズ
    int all_size = 0;
    //Webサーバーから受け取った敵のパラメータデータ
    std::vector<WebEnemy::WebEnemyParamPack> web_enemy_data;
public:
    //<Webサーバーから取得したデータのゲッター>//
    std::vector<WebEnemy::WebEnemyParamPack> GetWebEnemyParamPack() { return web_enemy_data; }
};