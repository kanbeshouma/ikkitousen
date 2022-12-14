#pragma once
#include<memory>
#include<vector>
#include <mutex>
#include"CommunicationSystem.h"

class CorrespondenceManager
{
private:
    CorrespondenceManager();
    ~CorrespondenceManager() {}
public:
    static CorrespondenceManager& Instance();
public:
    //-----サーバー-----//

    //-------自分のIPアドレスを取得-------//
    //===========================
    void AcquisitionMyIpAddress();

    bool InitializeServer();

    ////------UDPの受信関数------//
    //===========================
    //戻り値   : 接続番号
    //第1引数 : 受信したデータの保存変数
    //第2引数 : 保存変数の大きさ
    int UdpReceive(char* data, int size);

    ////----------UDP:特定の相手に送信----------//
    //===========================
    //第1引数 : 何番目のクライアントかどうか
    //第2引数 : 渡すデータ
    //第3引数 : データ型のサイズ
    void UdpSend(int id, char* data, int size);

    ////----------UDP:接続者全員に送信----------//
    //===========================
    //第1引数 : 渡すデータ
    //第2引数 : データ型のサイズ
    void UdpSend(char* data, int size);

    ////------TCP接続確立関数------//
    //===========================
    void TcpAccept();

    ////------TCPの受信関数------//
    //===========================
    //戻り値   : 接続番号
    //第1引数 : 受信したデータの保存変数
    //第2引数 : 保存変数の大きさ
    int TcpHostReceive(char* data, int size);

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

    ////----------TCP終了送信----------//
    //==========================
    //戻り値   : 全員にtcp通信終了のデータを送信して切断が成功したか
    bool CloseTcpHost();

    ////----------TCPクライアント終了----------//
    //==========================
    //戻り値   : 切断完了したかどうか
    bool CloseTcpClient();

    ////----------TCPでログインすることを送信----------//
    //==========================
    void Login();

    ////-----クライアント側のソケットの初期化-----//
    bool InitializeClient();

    ////-----ログインデータの受信-----//
    bool LoginReceive();

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

public:
    //--------------------------セッター---------------------------//
    void SetOperationPrivateId(int id) { operation_private_id = id; }
    void SetOpponentPlayerId(std::vector<int> id) { opponent_player_id = id; }
    void SetOpponentPlayerId(int id) { opponent_player_id.at(id) = id; }
    void SetMultiPlay(bool multi) { is_multi = multi; }

    ////-----引数-----//
    //===========
    //第1引数 : 設定したい場所
    //第2引数 : 設定したい値
    void SetOpponentPlayerId(int id,int num) { opponent_player_id.at(id) = num; }
    void SetHostId(int id) { host_id = id; }
    char* GetUdpPort(int id) { return udp_port; }

    //--------------------------ゲッター---------------------------//
    int GetOperationPrivateId() { return operation_private_id;}
    int GetHostId() { return host_id; }
    std::vector<int> GetOpponentPlayerId() { return opponent_player_id; }
    std::mutex& GetMutex() { return mutex; }
    bool GetMultiPlay() { return is_multi; }

    //-----データを初期化する-----//
    void ResetData();
private:
    std::unique_ptr<CommunicationSystem> communication_system;

    //自分の番号を保存
    int operation_private_id{ -1 };

    //自分以外のプレイヤーの番号
    std::vector<int> opponent_player_id;

    //TCP用のポート番号
    char tcp_port[8];

    //ホストのID
    int host_id{ -1 };
public:
    //UDP用のポート番号
    char udp_port[8];

    //-----名前-----//
    char my_name[12];

    //-----自分以外の名前-----//
    std::string names[MAX_CLIENT];
private:
    std::mutex mutex;

    //-----マルチプレイかどうか-----//
    bool is_multi{ false };
};