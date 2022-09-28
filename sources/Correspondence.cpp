#include"Correspondence.h"
#include"NetWorkInformationStucture.h"
#include"DebugConsole.h"
CorrespondenceManager::CorrespondenceManager()
{
    WSADATA was_data;
    if (WSAStartup(MAKEWORD(2, 2), &was_data) != 0)
    {
        //初期化に失敗
        WSAGetLastError();
    }

    communication_system = std::make_unique<CommunicationSystem>();
    //配列をリサイズする
    opponent_player_id.resize(MAX_CLIENT);
    for (int i = 0; i < opponent_player_id.size(); i++)
    {
        opponent_player_id.at(i) = -1;
    }
    //それぞれのポート番号を設定
    snprintf(udp_port, 8, "50008");
    snprintf(tcp_port, 8, "50110");

}

CorrespondenceManager& CorrespondenceManager::Instance()
{
    static CorrespondenceManager i;
    return i;
}

void CorrespondenceManager::AcquisitionMyIpAddress()
{
    //server->AcquisitionMyIpAddress();
    communication_system->AcquisitionMyIpAddress();
}

bool CorrespondenceManager::InitializeServer()
{
    //----------今あるソケットの情報などを一旦リセットする----------//
    SocketCommunicationManager::Instance().ClearData();
    //----------ソケット情報の初期化----------//
    return communication_system->InitializeHost(tcp_port,udp_port, operation_private_id);
}

int CorrespondenceManager::UdpReceive(char* data, int size)
{
    //----------接続して来たプレイヤーのIDを返す----------//
    int id = communication_system->UdpReceive(data, size);
    //-----もしコマンドが-1なら何もしない-----//
    if (data[0] == -1)
    {
        DebugConsole::Instance().WriteDebugConsole("コマンドが-1でした", TextColor::Blue);
        return -1;
    }
    return id;
}

void CorrespondenceManager::TcpSend(int id, char* data, int size)
{
    communication_system->TcpSend(id, data, size);
}

bool CorrespondenceManager::CloseTcpHost()
{
    MachingEndData data;
    data.cmd[0] = CommandList::MachingEnd;
    return communication_system->CloseTcpHost((char*)&data,sizeof(MachingEndData), operation_private_id);
}

bool CorrespondenceManager::CloseTcpClient()
{
    return communication_system->CloseTcpClient();
}

void CorrespondenceManager::Login()
{
    SendHostLoginData login;
    login.cmd[0] = CommandList::Login;
    snprintf(login.port, 8, CorrespondenceManager::Instance().udp_port);
    int size = sizeof(SendHostLoginData);
    communication_system->LoginSend((char*)&login, size);
}

bool CorrespondenceManager::InitializeClient()
{
    //----------今あるソケットの情報などを一旦リセットする----------//
    SocketCommunicationManager::Instance().ClearData();
    //----------ソケットの初期化----------//
    return communication_system->InitializeClient(tcp_port,udp_port);
}

void CorrespondenceManager::UdpSend(char* data, int size)
{
    communication_system->UdpSend(data, size);
}

void CorrespondenceManager::TcpAccept()
{
   communication_system->TcpAccept(udp_port);
}

int CorrespondenceManager::TcpHostReceive(char* data, int size)
{
    int id = communication_system->TcpHostReceive(data, size,operation_private_id);
    //-----もしコマンドが-1なら何もしない-----//
    if (data[0] == -1) return -1;
    return id;

}

int CorrespondenceManager::TcpClientReceive(char* data, int size)
{
    //----------クライアント側の受信をする----------//
   int receive = communication_system->TcpClientReceive(data, size);
   if (data[0] == -1) return -1;
   return receive;
}

void CorrespondenceManager::UdpSend(int id, char* data, int size)
{
    //server->UdpSend(id,data,size);
    communication_system->UdpSend(id, data, size);
}

bool CorrespondenceManager::LoginReceive()
{
    //-----受信データ-----//
    char data[256];
    ZeroMemory(data, sizeof(data));
    //----データを受信----//
    //int size = client->Receive(SignalType::UDP, data, sizeof(data));
    int size = communication_system->LoginReceive(data, sizeof(data));
    //データのサイズが0以下なら何も受信していないからreturn
    if (size <= 0) return false;
    //データのコマンドが-1なら何もされていないからreturn
    if (data[0] == -1) return false;
    //先頭バイトによる分岐
    switch (data[0])
    {
        //-------ログイン--------//
    case CommandList::Login:
    {
        LoginData* login = (LoginData*)data;
        //------自分の番号を保存-----//
        operation_private_id = login->operation_private_id;

        //-----ホストの番号を保存-----//
        host_id = login->host_id;

        //-----自分以外のプレイヤーの番号を保存----//
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            opponent_player_id.at(i) = login->opponent_player_id[i];
            //Ipアドレスを保存
            SocketCommunicationManager::Instance().game_udp_server_addr[i] = login->game_udp_server_addr[i];
            std::string ip = std::to_string(opponent_player_id.at(i)) + "番目 :" +  std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4);
            DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);
        }

        return true;
        break;
    }
    default:
        return false;
        break;
    }
    return false;
}

void CorrespondenceManager::LogoutClient(int client_id)
{
    communication_system->LogoutClient(client_id);
}

sockaddr_in CorrespondenceManager::SetSendPortAddress(sockaddr_in addr, char* port)
{
    return communication_system->SetSendPortAddress(addr, port);
}

void CorrespondenceManager::ResetData()
{
    operation_private_id = -1;
    //配列をリサイズする
    for (int i = 0; i < opponent_player_id.size(); i++)
    {
        opponent_player_id.at(i) = -1;
    }
}
