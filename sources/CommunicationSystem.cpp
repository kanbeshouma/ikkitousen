#include "CommunicationSystem.h"
#include"Correspondence.h"
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include"EnemyStructuer.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include"json.hpp"
#include <filesystem>
#include <fstream>

CommunicationSystem::CommunicationSystem()
{
}

CommunicationSystem::~CommunicationSystem()
{
    WSACleanup();
}

void CommunicationSystem::AcquisitionMyIpAddress()
{
    //---自分の名前を検索---//
    char host_name[50];
    gethostname(host_name, 50);

    //------ソケット情報変数-------//
    addrinfo hints;
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};

    //----------情報を設定---------//
    //------追加のオプションを設定-----//
    hints.ai_flags = 0;
    //-------IPV4で接続--------//
    hints.ai_family = AF_INET;
    //-------UDPで通信-------//
    hints.ai_socktype = SOCK_DGRAM;
    //--ソケットアドレスのプロトコル設定--//
    hints.ai_protocol = 0;
    getaddrinfo(host_name, NULL, &hints, &addr_info);

    servent* port = getservbyname(host_name,NULL);

    addr = *((sockaddr_in*)addr_info->ai_addr);
    //自分のアドレスを保存
    std::string ip = std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b4);
    if (port)
    {
        std::string p = std::to_string(port->s_port);
        DebugConsole::Instance().WriteDebugConsole(p);
    }
    snprintf(SocketCommunicationManager::Instance().host_ip, 32, ip.c_str());
    snprintf(SocketCommunicationManager::Instance().my_ip, 32, ip.c_str());

}

bool CommunicationSystem::InitializeHost(char* tco_port, char* udp_port, int private_id)
{
    //----------UDP通信用の初期化----------//
    if (InitializeHostUdpSocket(udp_port, private_id) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("ホスト : UDPのソケットの作成に失敗しました", TextColor::Red);
        WSACleanup();
        return false;
    }

    //----------TCP通信用の初期化----------//
    if (InitializeHostTcp(tco_port,private_id) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("ホスト : TCPのソケットの作成に失敗しました", TextColor::Red);
        WSACleanup();
        return false;
    }
    return true;
}

bool CommunicationSystem::InitializeHostUdpSocket(char* port, int private_id)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    //--------ソケット情報変数--------//
    addrinfo hints;
    //-----所得したアドレスを保存する-----//
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};
    //------------情報を設定-------------//
    //----追加のオプションを設定----//
    hints.ai_flags = 0;
    //----------IPV4で接続----------//
    hints.ai_family = AF_INET;
    //-----------UDPで通信-------------//
    hints.ai_socktype = SOCK_DGRAM;
    //------ソケットアドレスのプロトコル設定-----//
    hints.ai_protocol = 0;
    //----------インターネットアドレスを取得----------//
    if (getaddrinfo(SocketCommunicationManager::Instance().my_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("UDP ホスト: インターネットアドレスの取得に失敗しました", TextColor::Red);
        return false;
    }
    //----------アドレス情報を代入する-----------//
    addr = *((sockaddr_in*)addr_info->ai_addr);

    //----------自分のアドレスを保存----------//
    instance.game_udp_server_addr[private_id] = addr;
    std::string ip = std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b4) + "ポート番号" + std::to_string(addr.sin_port);
    DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);

    //----------ソケット生成----------//
    instance.udp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (instance.udp_sock == INVALID_SOCKET)
    {
        //----------ソケットの生成に失敗----------//
        DebugConsole::Instance().WriteDebugConsole("UDP ホスト: ソケットの生成に失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //----------ソケットの受け入れ情報を関連付ける----------//
    if (bind(instance.udp_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        //----------ソケットの受け入れ情報の関連付け失敗----------//
        DebugConsole::Instance().WriteDebugConsole("UDP : ソケットの受け入れ情報の関連付けで失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    DebugConsole::Instance().WriteDebugConsole("UDP ホスト: 受信用ソケットの生成に成功しました", TextColor::Blue);
    // sockをfdsにセット
    FD_SET(instance.udp_sock, &instance.udp_fds);

    return true;
}

bool CommunicationSystem::InitializeHostTcp(char* port, int private_id)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //ソケット情報変数
    addrinfo hints;
    ZeroMemory(&hints, sizeof(addrinfo));
    //所得したアドレスを保存する
    addrinfo* addr_info{ NULL };
    sockaddr_in addr{};
    //情報を設定
    //追加のオプションを設定
    hints.ai_flags = 0;
    //IPV6で接続
    hints.ai_family = AF_INET;
    //UDPで通信
    hints.ai_socktype = SOCK_STREAM;
    //ソケットアドレスのプロトコル設定
    hints.ai_protocol = 0;
    ////インターネットアドレスを取得
    if (getaddrinfo(SocketCommunicationManager::Instance().host_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP : インターネットアドレスの取得に失敗しました", TextColor::Red);
        return false;
    }
    DebugConsole::Instance().WriteDebugConsole(port);

    addr = *((sockaddr_in*)addr_info->ai_addr);
    //ソケット生成
    instance.tcp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (instance.tcp_sock == INVALID_SOCKET)
    {
        //ソケットの生成に失敗
        DebugConsole::Instance().WriteDebugConsole("TCP : ホスト  ソケットの生成に失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //ソケットの受け入れ情報を関連付ける
    if (bind(instance.tcp_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        //ソケットの受け入れ情報の関連付け失敗
        DebugConsole::Instance().WriteDebugConsole("TCP : ホスト  ソケットの受け入れ情報の関連付けで失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //非ブロッキング(ノンブロッキング)に設定
    u_long i_modes{ 1 };//0だとブロッキング
    if (ioctlsocket(instance.tcp_sock, FIONBIO, &i_modes) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP : ホスト  非ブロッキング設定に失敗しました", TextColor::Red);
        WSAGetLastError();
        return false;
    }
    // sockをfdsにセット
    FD_SET(instance.tcp_sock, &instance.tcp_fds);

    //ログイン用のソケットに代入する
    instance.login_client_sock[private_id]= instance.tcp_sock;

    //-----サーバーの待ち受けを開始-----//
    //-----第二引数は待ち受け数(-1してるのは自分を除くから)-----//
    listen(instance.tcp_sock, MAX_CLIENT - 1);

    return true;
}

bool CommunicationSystem::InitializeClient(char* tcp_port, char* udp_port)
{

    //-----UDP通信用ソケットを作成-----//
    if (InitializeClientUdpSocket(udp_port) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("クライアント : UDP受信用ソケットの作成に失敗しました", TextColor::Red);
        WSACleanup();
        return false;
    }
    //----------TCP通信用の初期化----------//
    if (InitializeClientTcp(tcp_port) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("クライアント : TCPのソケットの作成に失敗しました", TextColor::Red);
        WSACleanup();
        return false;
    }
    return true;
}

bool CommunicationSystem::InitializeClientTcp(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //-------ログイン先のアドレスを取得-------//
    if (AcquisitionLoginAddress(port) == false) return false;
    //-------自分のアドレスを取得--------//
    char host_name[50];
    gethostname(host_name, 50);
    //-------ソケット情報変数-------//
    addrinfo hints;
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    //----------情報を設定----------//
    //-----追加のオプションを設定-----//
    hints.ai_flags = 0;
    //------IPV4で接続------//
    hints.ai_family = AF_INET;
    //---------UDPで通信----------//
    hints.ai_socktype = SOCK_STREAM;
    //-----------ソケットアドレスのプロトコル設定-------------//
    hints.ai_protocol = 0;
    if (getaddrinfo(host_name, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP クライアント : インターネットアドレスの取得に失敗しました", TextColor::Red);
        return false;
    }
    DebugConsole::Instance().WriteDebugConsole(port);

    //--------------ソケットを生成--------------//
    instance.tcp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (instance.tcp_sock == INVALID_SOCKET)
    {
        //ソケットの生成に失敗
        DebugConsole::Instance().WriteDebugConsole("TCP クライアント: ソケットの生成に失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    //----------サーバーへ接続----------//
    if (connect(instance.tcp_sock, (sockaddr*)&instance.login_tcp_server_addr, sizeof(instance.login_tcp_server_addr)) != 0)
    {
        //----------サーバーへの接続に失敗----------//
        DebugConsole::Instance().WriteDebugConsole("TCP クライアント: サーバーとの接続に失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //-----sockをfdsにセット-----//
    FD_SET(instance.tcp_sock, &instance.tcp_fds);

    return true;
}

bool CommunicationSystem::InitializeClientUdpSocket(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //-------ソケット情報変数-------//
    addrinfo hints;
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};
    //----------情報を設定----------//
    //-----追加のオプションを設定-----//
    hints.ai_flags = 0;

    //------IPV4で接続------//
    hints.ai_family = AF_INET;

    //---------UDPで通信----------//
    hints.ai_socktype = SOCK_DGRAM;

    //-----------ソケットアドレスのプロトコル設定-------------//
    hints.ai_protocol = 0;

    if (getaddrinfo(SocketCommunicationManager::Instance().my_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("UDP クライアント : インターネットアドレスの取得に失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    //----------アドレス情報を代入する-----------//
    //--(クライアントはログインデータを貰ったタイミングでアドレス配列に入れるからここでは保存しない)--//
    addr = *((sockaddr_in*)addr_info->ai_addr);
    std::string ip =std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b4) + "ポート番号" + std::to_string(addr.sin_port);
    DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Blue);
    DebugConsole::Instance().WriteDebugConsole(port,TextColor::Blue);

    //--------------ソケットを生成--------------//
    instance.udp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);

    if (instance.udp_sock == INVALID_SOCKET)
    {
        //ソケットの生成に失敗
        DebugConsole::Instance().WriteDebugConsole("UDP クライアント: ソケットの生成に失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    //ソケットの受け入れ情報を関連付ける
    if (bind(instance.udp_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        //ソケットの受け入れ情報の関連付け失敗
        DebugConsole::Instance().WriteDebugConsole("UDP クライアント: ソケットの受け入れ情報の関連付けで失敗しました", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    DebugConsole::Instance().WriteDebugConsole("UDP クライアント: 受信用ソケットの生成に成功しました", TextColor::Blue);
    // sockをfdsにセット
    FD_SET(instance.udp_sock, &instance.udp_fds);

    return true;
}

int CommunicationSystem::HttpRequest()
{
    //TO_INT(test, 5);
    //TO_INT("test", 1);


    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    //<OpenSSL変数>//
    SSL* ssl;
    SSL_CTX* ctx;

    addrinfo hints;
    addrinfo* addrInfo = nullptr;
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr;
    // 取得する情報を設定
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    //<HTTPサーバーのアドレスを取得>//
    int err = getaddrinfo(hostname, http_port, &hints, &addrInfo);
    if (err != 0) {
        DebugConsole::Instance().WriteDebugConsole("ドメインからアドレス取得に失敗しました", TextColor::Red);
        return -1;
    }
    addr = *((sockaddr_in*)addrInfo->ai_addr);

    //<ソケットの生成>//
    instance.http_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (instance.http_sock < 0) {
        DebugConsole::Instance().WriteDebugConsole("ソケットの生成に失敗しました。", TextColor::Red);
        return -1;
    }
    //<サーバーに接続>//
    if (connect(instance.http_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        DebugConsole::Instance().WriteDebugConsole("connectに失敗しました。", TextColor::Red);
        return -1;
    }

    //<エラーが数値で管理されているからそれの詳細が分かるようにする>//
    SSL_load_error_strings();
    //<SSLの初期化をする>//
    //これを実行することにより、暗号化方式やメッセージダイジェスト関数などが登録される
    SSL_library_init();
    //<SSL_CTX構造体の生成>//
    //TLSv1,SSLv2,SSLv3の種類がある
    //SSLv23_client_method<-いずれかを使う場合はこれでまとめて設定できる
    ctx = SSL_CTX_new(SSLv23_client_method());

    //==============================================//
    //
    //SSL_CTXはSSLにおける暗号化や認証方法などを管理する
    //
    //==============================================//
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        DebugConsole::Instance().WriteDebugConsole("ssl_ctxの生成に失敗しました。", TextColor::Red);
        return -1;
    }

    //<SSL構造体の生成>//
    //SSL_CTX構造体にセットしたプロトコルや暗号化方式を元に
    //コネクションを管理するSSL構造体を生成する
    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);
        DebugConsole::Instance().WriteDebugConsole("sslの生成に失敗しました。", TextColor::Red);
        return -1;
    }

    //==============================================//
    //
    //SSLはサーバーのコネクション管理
    //
    //==============================================//
    //<コネクション済みのソケットとSSL構造体を結びつける>//
    int ret;

    ret = SSL_set_fd(ssl, static_cast<int>(instance.http_sock));
    if (ret == 0) {
        ERR_print_errors_fp(stderr);
        DebugConsole::Instance().WriteDebugConsole("socketとsslの関連付けに失敗しました", TextColor::Red);
        return -1;
    }

    /* PRNG 初期化 */
    RAND_poll();
    while (RAND_status() == 0) {
        unsigned short rand_ret = rand() % 65536;
        RAND_seed(&rand_ret, sizeof(rand_ret));
    }

    /* SSL で接続 */
    //<自動的にサーバとハンドシェイクが行われる>//
    //使用するプロトコル(SSLv2 / SSLv3 / TLSv1)
    //使用する暗号化方式・鍵交換方式・ハッシュ方式
    //サーバ証明書の取得
    //使用する共通鍵
    //これらが決定される
    ret = SSL_connect(ssl);
    if (ret != 1) {
        ERR_print_errors_fp(stderr);
        DebugConsole::Instance().WriteDebugConsole("サーバ接続に失敗しました。", TextColor::Red);
        return -1;

    }
    /* リクエスト送信 */
    char request[256];
    sprintf_s(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, hostname);
    ret = SSL_write(ssl, request, static_cast<int>(strlen(request)));
    if (ret < 1) {
        ERR_print_errors_fp(stderr);
        DebugConsole::Instance().WriteDebugConsole("送信に失敗しました。", TextColor::Red);
        return -1;
    }

    //<ヘッダ部分を取り除いたかどうか>//
    header_check = false;
    //<文字数を取得してその分を取り除いたかどうか>//
    char_num_check = false;
    //<文字数を入れる>//
    char_count = {};
    //分割して受信した場合結合用の配列
    char ret_source[1500] = { "" };
    //合計サイズ
    all_size = 0;
    std::string json_data{};

     //分割して受信した場合結合用の配列
    while (1) {
        char buf[255] = {};
        int read_size;
        read_size = SSL_read(ssl, buf, sizeof(buf) - 1);

            //<ヘッダ部分を抜き取る>//
            if (header_check == false)
            {
                char body[1024] = { "" };
                int idx = 0;
                while (1)
                {
                    //文字列を確認し改行コードが連続している箇所を検索
                    if (memcmp((buf + idx), "\r\n\r\n", 4) == 0)
                    {
                        //改行コードが連続している所から最後までを取得
                        memcpy(body, &buf[idx + 4], strlen(&buf[idx + 4]));
                        //<ヘッダ部分を取り除いたデータを保存>//
                        std::memcpy(ret_source + all_size, body, strlen(body));
                        all_size += static_cast<int>(strlen(body));
                        //<ヘッダ部分を取り除いた>//
                        header_check = true;
                        break;
                    }
                    idx++;
                }
            }
            //<データの文字数を抜き取る>//
            else
            {
                if (read_size > 0) {
                    std::memcpy(ret_source + all_size, buf, read_size);
                    all_size += read_size;
                }
            }

            //<文字数を取得する>//
            if(char_num_check == false)
            {
                int idx = 0;
                while (1)
                {
                    //文字列を確認し改行コードが連続している箇所を検索
                    if (memcmp((ret_source + idx), "\r\n", 2) == 0)
                    {
                        //<文字数を受け取る>//
                        char num[10] = {};
                        //<文字数が書いてある部分を取得>//
                        memcpy(num, &ret_source, idx);
                        //<16進数に変換>//
                        std::string number = "0x";
                        number += num;
                        //<10進数に変換>//
                        char_count = static_cast<int>(std::stod(number.c_str()));

                        //改行コードが連続している所から最後までを取得
                        memcpy(ret_source, &ret_source[idx + 2], strlen(&ret_source[idx + 2]));
                        char_num_check = true;
                        all_size -= 5;
                        break;
                    }
                    idx++;
                 }
            }

            //<データの文字数よりも大きかったら後ろの部分をきる取る>//
            if (all_size >= char_count)
            {
                json_data = ret_source;
                json_data.erase(json_data.find("\r\n"));
                break;
            }

#if 0
                            //<一番最初で文字数を取得>//
        if (read_size > 0) {
            std::cout << buf;
        }
        else if (read_size <= 0) {
            /* FIN 受信 */
            break;
        }
        else {
            ERR_print_errors_fp(stderr);
            exit(1);
        }

#endif // 0
    }
    using namespace nlohmann;

    auto j = nlohmann::json::parse(json_data);

    std::vector<WebEnemy::WebEnemyParamPack> web_enemy_data;

    for (json::iterator it = j.begin(); it != j.end(); ++it)
    {
        auto j_data = it.value();
        WebEnemy::WebEnemyParamPack e_data = j_data.get<WebEnemy::WebEnemyParamPack>();
        web_enemy_data.emplace_back(e_data);
    }
    web_enemy_data;

    ret = SSL_shutdown(ssl);
    if (ret < 0) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }
    else if (ret == 0)
    {
        ret = SSL_shutdown(ssl);
        if (ret < 0)
        {
            int e = SSL_get_error(ssl, ret);//ここのエラーコードが1だった
            std::string msg = std::to_string(e);
            DebugConsole::Instance().WriteDebugConsole(msg,TextColor::Red);
        }
    }

    //<SSLの終了処理>//

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    closesocket(instance.http_sock);
    WSACleanup();
    return 1;

}

int CommunicationSystem::LoginReceive(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    fd_set client_fd{};
    timeval tv;
    //----------秒-----------//
    tv.tv_sec = 0;
    //----------ミリ秒----------//
    tv.tv_usec = 0;
    //----------読み込み監視変数をコピーして監視する----------//
    memcpy(&client_fd, &instance.tcp_fds, sizeof(fd_set));

    int  judg = select(static_cast<int>(instance.tcp_sock + 1), &client_fd, NULL, NULL, &tv);
    if (judg <= 0) return -1;

    int recv_size = recv(instance.tcp_sock, data, size, 0);
    //エラーの時はSOCKET_ERRORが入る
    if (recv_size == SOCKET_ERROR)
    {
        //コンソール画面に出力
        if (tcp_error_num != WSAGetLastError())
        {
            tcp_error_num = WSAGetLastError();
            DebugConsole::Instance().WriteDebugConsole("ログイン : receive failed", TextColor::Red);
            std::string text = "error number:" + std::to_string(tcp_error_num);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        }
        return -1;
    }
    return recv_size;
}

void CommunicationSystem::LoginSend(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //----------データをサーバー(ホスト)に送信する----------//
    int send_size = send(instance.tcp_sock, data, size,0);
    //エラーの時はSOCKET_ERRORが入る
    if (send_size == SOCKET_ERROR)
    {
        //コンソール画面に出力
        if (tcp_error_num != WSAGetLastError())
        {
            tcp_error_num = WSAGetLastError();
            DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
            std::string text = "error number:" + std::to_string(tcp_error_num);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        }
    }
    else
    {
        std::string text =  "サーバー(ホスト)に" + std::to_string(send_size) + "バイト送信しました";

        DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
    }
}

void CommunicationSystem::TcpAccept(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //-----読み込み監視変数-----//
    fd_set fd_work;
    timeval tv;
    //-----秒-----//
    tv.tv_sec = 0;

    //-----ミリ秒-----//
    tv.tv_usec = 0;

    //-----fd_workにコピーする-----//
    memcpy(&fd_work, &instance.tcp_fds, sizeof(fd_set));

    //-----fdsに設定されたソケットが読み込み可能になるまで待つ-----//
    int n = select(static_cast<int>(instance.tcp_sock + 1), &fd_work, NULL, NULL, &tv);

    //----------タイムアウトの場合selectは0を返す----------//
    if (n <= 0) return;
    int len = sizeof(sockaddr_in);
    sockaddr_in addr;
    SOCKET sock{ INVALID_SOCKET };

    //----------接続を待機する----------//
    sock = accept(instance.tcp_sock, (sockaddr*)&addr, &len);
    if (sock == INVALID_SOCKET)
    {
        if (tcp_error_num != WSAGetLastError())
        {
            tcp_error_num = WSAGetLastError();
            std::string text = "TCP : ログイン : error number:" + std::to_string(tcp_error_num);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
            return;
        }
    }
    else
    {
#if 0
        //----------クライアント検索----------//
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            //----------アドレスが初期化状態ならとばす----------//
            if ((instance.game_udp_server_addr[i].sin_addr.S_un.S_addr == 0 &&
                instance.login_client_sock[i] == INVALID_SOCKET) ||
                i == CorrespondenceManager::Instance().GetOperationPrivateId()) continue;

            //------保存してあるアドレスと受信したアドレスが同じなら何番目かを返す------//
            if (addr.sin_addr.S_un.S_addr == instance.game_udp_server_addr[i].sin_addr.S_un.S_addr)
            {
                DebugConsole::Instance().WriteDebugConsole("保存してあるアドレスです", TextColor::White);
                return;
            }
        }

#endif // 0
        //----------クライアント受け入れ----------//
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            //----------アドレスが初期化状態ならとばす----------//
            if (instance.game_udp_server_addr[i].sin_addr.S_un.S_addr != 0 &&
                instance.login_client_sock[i] != INVALID_SOCKET) continue;

            instance.game_udp_server_addr[i] = addr;
            instance.login_client_sock[i] = sock;
            //-----sockをfdsにセット-----//
            FD_SET(instance.login_client_sock[i], &instance.client_tcp_fds);
            instance.client_tcp_fds_count++;
            DebugConsole::Instance().WriteDebugConsole("ログインしてきました", TextColor::White);
            return;
        }
    }
}

int CommunicationSystem::TcpHostReceive(char* data, int size, int operation_private_id)
{

    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    std::lock_guard<std::mutex> lock(instance.GetMutex());

    fd_set client_fd{};
    timeval tv;
    //----------秒-----------//
    tv.tv_sec = 0;
    //----------ミリ秒----------//
    tv.tv_usec = 0;
    //----------データを受信する----------//
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        //-----もしソケットに何も入っていない場合か自分の番号と同じ番号なら次にいく-----//
        if (instance.login_client_sock[i] == INVALID_SOCKET || i == operation_private_id) continue;
        //----------読み込み監視変数をコピーして監視する----------//
        memcpy(&client_fd, &instance.client_tcp_fds, sizeof(fd_set));
        int  judg = select(static_cast<int>(instance.login_client_sock[i] + instance.client_tcp_fds_count), &client_fd, NULL, NULL, &tv);
        if (judg >= 1)
        {
            //----------データを受信する---------//
            int recv_size = recv(instance.login_client_sock[i], data, size, 0);
            if (recv_size == SOCKET_ERROR)
            {
                //コンソール画面に出力
                if (tcp_error_num != WSAGetLastError())
                {
                    tcp_error_num = WSAGetLastError();
                    DebugConsole::Instance().WriteDebugConsole("TCP : receive failed", TextColor::Red);
                    std::string text = "error number:" + std::to_string(tcp_error_num);
                    DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                    continue;
                }
            }
            else
            {
                DebugConsole::Instance().WriteDebugConsole("TCP : データを受信しました", TextColor::White);
                return i;
            }
        }
    }
    //----------ここまで来たら何も受信していないから-1を返しておく----------//
    return -1;
}

int CommunicationSystem::TcpClientReceive(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    fd_set client_fd{};
    timeval tv;
    //----------秒-----------//
    tv.tv_sec = 0;
    //----------ミリ秒----------//
    tv.tv_usec = 0;
    //----------データを受信する----------//
    //----------読み込み監視変数をコピーして監視する----------//
    memcpy(&client_fd, &instance.tcp_fds, sizeof(fd_set));
    int  judg = select(static_cast<int>(instance.tcp_sock + 1), &client_fd, NULL, NULL, &tv);
    if (judg >= 1)
    {
        //----------データを受信する---------//
        int recv_size = recv(instance.tcp_sock, data, size, 0);
        if (recv_size == SOCKET_ERROR)
        {
            //コンソール画面に出力
            if (tcp_error_num != WSAGetLastError())
            {
                tcp_error_num = WSAGetLastError();
                DebugConsole::Instance().WriteDebugConsole("TCP : receive failed", TextColor::Red);
                std::string text = "error number:" + std::to_string(tcp_error_num);
                DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                return -1;
            }
        }
        else
        {
            std::string text =std::to_string(recv_size) + "バイトTCPで受信しました";
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
            return recv_size;
        }
    }
    //----------ここまで来たら何も受信していないから-1を返しておく----------//
    return -1;
}

void CommunicationSystem::TcpSend(int id, char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //データをクライアントに送る
    int send_size = send(instance.login_client_sock[id],data,size,0);
    //エラーの時はSOCKET_ERRORが入る
    if (send_size == SOCKET_ERROR)
    {
        //コンソール画面に出力
        if (tcp_error_num != WSAGetLastError())
        {
            tcp_error_num = WSAGetLastError();
            DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
            std::string text = "error number:" + std::to_string(tcp_error_num);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        }
    }
    else
    {
        std::string text = std::to_string(id) + "番目にTCP通信で" + "コマンド :"+std::to_string(data[0]) +" で"+std::to_string(send_size) + "バイト送信しました";

        DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
    }
}

void CommunicationSystem::TcpSend(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //----------データをサーバー(ホスト)に送信する----------//
    int send_size = send(instance.tcp_sock, data, size, 0);
    //エラーの時はSOCKET_ERRORが入る
    if (send_size == SOCKET_ERROR)
    {
        //コンソール画面に出力
        if (tcp_error_num != WSAGetLastError())
        {
            tcp_error_num = WSAGetLastError();
            DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
            std::string text = "error number:" + std::to_string(tcp_error_num);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        }
    }
    else
    {
        std::string text = "サーバー(ホスト)に" + std::to_string(send_size) + "バイト送信しました";

        DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
    }
}

void CommunicationSystem::TcpSendAllClient(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        //-----ソケットに何も設定されていない時か自分の番号と同じならとばす-----//
        if (instance.login_client_sock[i] == INVALID_SOCKET || i == CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //データをクライアントに送る
        int send_size = send(instance.login_client_sock[i], data, size, 0);
        //エラーの時はSOCKET_ERRORが入る
        if (send_size == SOCKET_ERROR)
        {
            //コンソール画面に出力
            if (tcp_error_num != WSAGetLastError())
            {
                tcp_error_num = WSAGetLastError();
                DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
                std::string text = "error number:" + std::to_string(tcp_error_num);
                DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
            }
        }
        else
        {
            std::string text = std::to_string(i) + "番目にTCP通信で" + "コマンド :" + std::to_string(data[0]) + " で" + std::to_string(send_size) + "バイト送信しました";

            DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
        }
    }
}

bool CommunicationSystem::CloseTcpHost(char* data, int size,int operation_private_id)
{

    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (instance.login_client_sock[i] == INVALID_SOCKET || i == operation_private_id) continue;
        //データをクライアントに送る
        int send_size = send(instance.login_client_sock[i], data, size, 0);
        //エラーの時はSOCKET_ERRORが入る
        if (send_size == SOCKET_ERROR)
        {
            //コンソール画面に出力
            if (tcp_error_num != WSAGetLastError())
            {
                tcp_error_num = WSAGetLastError();
                DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
                std::string text = "error number:" + std::to_string(tcp_error_num);
                DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                //送信に失敗したらfalseをかえしておく
                return false;
            }
        }
        else
        {
            std::string text = std::to_string(i) + "番目にTCP通信で" + std::to_string(send_size) + "バイト送信しました";
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
        }
    }

    if (closesocket(instance.tcp_sock) != 0)
    {
        tcp_error_num = WSAGetLastError();
        DebugConsole::Instance().WriteDebugConsole("TCP切断に失敗しました", TextColor::Red);
        std::string text = "error number:" + std::to_string(tcp_error_num);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    return true;
}

bool CommunicationSystem::CloseTcpClient()
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    if (closesocket(instance.tcp_sock) != 0)
    {
        tcp_error_num = WSAGetLastError();
        DebugConsole::Instance().WriteDebugConsole("TCP切断に失敗しました", TextColor::Red);
        std::string text = "error number:" + std::to_string(tcp_error_num);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    return true;
}

int CommunicationSystem::UdpReceive(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    timeval tv;
    //----------秒-----------//
    tv.tv_sec = 0;
    //----------ミリ秒----------//
    tv.tv_usec = 0;
    //----------読み込み監視変数----------//
    fd_set fd_work;
    //----------fd_workにコピーする----------//
    memcpy(&fd_work, &instance.udp_fds, sizeof(fd_set));
    //-----fdsに設定されたソケットが読み込み可能になるまで待つ-----//
    int n = select(0, &fd_work, NULL, NULL, &tv);
    //----------タイムアウトの場合selectは0を返す----------//
    if (n <= 0)
    {
        //DebugConsole::Instance().WriteDebugConsole("タイムアウト", TextColor::Blue);
        return -1;
    }
    sockaddr_in addr;
    int addr_length{ sizeof(sockaddr_in) };
    //----------データの受信----------//
    int recv_size = recvfrom(instance.udp_sock, data, size, 0, (sockaddr*)&addr, &addr_length);
    //-----エラーの時はSOCKET_ERRORが入る-----//
    if (recv_size == SOCKET_ERROR)
    {
        //----------コンソール画面に出力----------//
        int error = WSAGetLastError();
        DebugConsole::Instance().WriteDebugConsole("receive failed", TextColor::Red);
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return -1;
    }
    //----------クライアント検索----------//
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        //----------アドレスが初期化状態か自分の番号と同じだったら----------//
        if (instance.game_udp_server_addr[i].sin_addr.S_un.S_addr == 0 || i == CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //------保存してあるアドレスと受信したアドレスが同じなら何番目かを返す------//
        if (addr.sin_addr.S_un.S_addr == instance.game_udp_server_addr[i].sin_addr.S_un.S_addr)
        {
            std::string ip = std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4) + "受信ポート番号" + std::to_string(addr.sin_port) + "   :  保存ポート" + std::to_string(instance.game_udp_server_addr[i].sin_port);
            std::string text =ip + "アドレス" +  std::to_string(i) + "番目のアドレスで受信しました";
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Blue);

            return i;
        }
    }
    //----------ここまで来たら最大接続数に達しているので-1を返しておく----------//
    return -1;

}

void CommunicationSystem::UdpSend(int id, char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    //データを送る(ここで使うソケットはそれぞれのポート番号に合わせて作ったソケット)
    int send_size = sendto(instance.udp_sock, data, size, 0, (sockaddr*)&instance.game_udp_server_addr[id], sizeof(instance.game_udp_server_addr[id]));

    //エラーの時はSOCKET_ERRORが入る
    if (send_size == SOCKET_ERROR)
    {
        //コンソール画面に出力
        int error = WSAGetLastError();
        DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
    }
    else
    {
        std::string text = std::to_string(id) + "番目に" + std::to_string(send_size) + "バイト送信しました";

        DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
    }
}

void CommunicationSystem::UdpSend(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        //-----もしアドレスが保存されていない時か自分の番号と同じなら飛ばす-----//
        if (instance.game_udp_server_addr[i].sin_addr.S_un.S_addr == 0 ||
            i == CorrespondenceManager::Instance().GetOperationPrivateId()) continue;

        //-----データを送信する-----//
        int send_size = sendto(instance.udp_sock, data, size, 0, (sockaddr*)&instance.game_udp_server_addr[i], sizeof(instance.game_udp_server_addr[i]));

        //エラーの時はSOCKET_ERRORが入る
        if (send_size == SOCKET_ERROR)
        {
            //コンソール画面に出力
            int error = WSAGetLastError();
            DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
            std::string text = "error number:" + std::to_string(error);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        }
        else
        {
            char da[2]{};
            memcpy_s(da,sizeof(da),data,sizeof(da));

            std::string ip = std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4) + "ポート番号" + std::to_string(instance.game_udp_server_addr[i].sin_port);
            std::string text = ip + "のアドレス :" +std::to_string(i) + "番目にコマンド : "+ std::to_string(da[1])+"で" +std::to_string(send_size) + "バイト送信しました";

            DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
        }
    }
}

void CommunicationSystem::LogoutClient(int client_id)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //----------アドレスの初期化----------//
    if (instance.game_udp_server_addr[client_id].sin_addr.S_un.S_addr != 0)
    {
        std::string text = std::to_string(client_id) + "番目のクライアントがログアウトしました";
        DebugConsole::Instance().WriteDebugConsole(text,TextColor::Red);
        instance.game_udp_server_addr[client_id].sin_addr.S_un.S_addr = 0;
    }

}

bool CommunicationSystem::AcquisitionLoginAddress(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //---------ソケット情報変数--------//
    addrinfo hints;
    //-------所得したアドレスを保存する-------//
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};
    //---------情報を設定---------//
    //----------追加のオプションを設定---------//
    hints.ai_flags = 0;
    //-----IPV4で接続-----//
    hints.ai_family = AF_INET;
    //------UDPで通信-------//
    hints.ai_socktype = SOCK_DGRAM;
    //---ソケットアドレスのプロトコル設定---//
    hints.ai_protocol = 0;
    //-------インターネットアドレスを取得-------//
    if (getaddrinfo(SocketCommunicationManager::Instance().host_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP クライアント: ログイン先のアドレス取得に失敗しました。", TextColor::Red);
        return false;
    }
    instance.login_tcp_server_addr = *((sockaddr_in*)addr_info->ai_addr);
    std::string ip = std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b1) + std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b2) + std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b3) + std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b4);
    DebugConsole::Instance().WriteDebugConsole("ログイン先のアドレス取得", TextColor::Green);
    DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);
    return true;
}

sockaddr_in CommunicationSystem::SetSendPortAddress(sockaddr_in addr, char* port)
{
    sockaddr_in create;
    //---------ソケット情報変数--------//
    addrinfo hints;
    //-------所得したアドレスを保存する-------//
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    //---------情報を設定---------//
    //----------追加のオプションを設定---------//
    hints.ai_flags = 0;
    //-----IPV4で接続-----//
    hints.ai_family = AF_INET;
    //------UDPで通信-------//
    hints.ai_socktype = SOCK_DGRAM;
    //---ソケットアドレスのプロトコル設定---//
    hints.ai_protocol = 0;
    //-------インターネットアドレスを取得-------//
    std::string ip = std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b4);

    if (getaddrinfo(ip.c_str(),port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("UDP ホスト: ログイン先のアドレス取得に失敗しました。", TextColor::Red);
    }
    create = *((sockaddr_in*)addr_info->ai_addr);

    return create;
}
