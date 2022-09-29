#pragma once
#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>
#pragma comment(lib, "Ws2_32.lib")
#define MAX_CLIENT 4
//==========================================================
//
//                今回の通信に必要な情報を持っている管理クラス
//
//==========================================================
class SocketCommunicationManager
{
private:
    SocketCommunicationManager();
    ~SocketCommunicationManager();
public:
    static SocketCommunicationManager& Instance()
    {
        static SocketCommunicationManager i;
        return i;
    }
public:
    //-----UDP通信受信用のソケット-----//
    SOCKET udp_sock{ INVALID_SOCKET };

    //-----TCP通信のソケット-----//
    SOCKET tcp_sock{ INVALID_SOCKET };

    //-----ログインで使うクライアントのソケット-----//
    SOCKET login_client_sock[MAX_CLIENT];

    //-----ログインで使うTCP通信のサーバーアドレス-----//
    sockaddr_in login_tcp_server_addr;

    //-----ゲーム側で使うUDPのサーバーアドレス(クライアントの数分いる)-----//
    sockaddr_in game_udp_server_addr[MAX_CLIENT];

    //-----UDP読み込み監視用-----//
    fd_set udp_fds;

    //-----TCP読み込み監視用-----//
    fd_set tcp_fds;

    //-----TCPクライアントのソケットの読み込み監視用-----//
    fd_set client_tcp_fds;

    //-----TCPクライアントのfdの接続数をカウント-----//
    int client_tcp_fds_count{ 0 };

    //ホストのIPアドレス
    char host_ip[32]{ "127.0.0.1" };
    char my_ip[32]{ "127.0.0.1" };

public:
    std::mutex& GetMutex() { return mutex; }

public:
    void ClearData();
private:
    std::mutex mutex;

};