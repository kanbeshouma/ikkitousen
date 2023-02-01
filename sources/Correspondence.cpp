#include"Correspondence.h"
#include"NetWorkInformationStucture.h"
#include"DebugConsole.h"
CorrespondenceManager::CorrespondenceManager()
{

    communication_system = std::make_unique<CommunicationSystem>();
    //配列をリサイズする
    opponent_player_id.resize(MAX_CLIENT);
    for (int i = 0; i < opponent_player_id.size(); i++)
    {
        opponent_player_id.at(i) = -1;
        names[i] = "";
        player_colors[i] = 0;
    }
    //それぞれのポート番号を設定
    snprintf(udp_port, 8, "50008");
    snprintf(tcp_port, 8, "50110");
    snprintf(my_name, 12, "Player");

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

bool CorrespondenceManager::HttpRequest()
{
    if (communication_system->HttpRequest() > 0)
    {
        DebugConsole::Instance().WriteDebugConsole("HTTPリクエスト成功",TextColor::Green);
        return true;
    }
    else
    {
        DebugConsole::Instance().WriteDebugConsole("HTTPリクエスト失敗",TextColor::Red);
        return false;
    }
    return false;
}

bool CorrespondenceManager::InitializeServer()
{
    //----------今あるソケットの情報などを一旦リセットする----------//
    SocketCommunicationManager::Instance().ClearData();
    //----------ソケット情報の初期化----------//
    return communication_system->InitializeHost(tcp_port,udp_port, operation_private_id);
}

#if 0
bool CorrespondenceManager::InitializeMultiCastSend()
{
    if (communication_system->InitializeMultiCastSend())
    {
        DebugConsole::Instance().WriteDebugConsole("マルチキャスト送信初期化成功", TextColor::Green);
        return true;
    }
    else
    {
        DebugConsole::Instance().WriteDebugConsole("マルチキャスト送信初期化失敗", TextColor::Red);
        return false;
    }

    return false;
}

bool CorrespondenceManager::InitializeMultiCastReceive()
{
    if (communication_system->InitializeMultiCastReceive())
    {
        DebugConsole::Instance().WriteDebugConsole("マルチキャスト受信初期化成功", TextColor::Green);
        return true;
    }
    else
    {
        DebugConsole::Instance().WriteDebugConsole("マルチキャスト受信初期化失敗", TextColor::Red);
        return false;
    }
    return false;
}

#endif // 0

void CorrespondenceManager::MultiCastSend(char* data, int size)
{
    communication_system->MultiCastSend(data, size);
}

void CorrespondenceManager::MultiCastReceive(char* data, int size)
{
    communication_system->MultiCastReceive(data, size);
}

int CorrespondenceManager::UdpReceive(char* data, int size)
{
    //-----マルチプレイでなかったら処理を止める-----//
    if (is_multi == false) return -1;

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
    if (is_multi == false) return;

    communication_system->TcpSend(id, data, size);
}

void CorrespondenceManager::TcpSend(char* data, int size)
{
    if (is_multi == false) return;

    communication_system->TcpSend(data, size);
}

void CorrespondenceManager::TcpSendAllClient(char* data, int size)
{
    if (is_multi == false) return;

    communication_system->TcpSendAllClient(data, size);
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
    login.cmd[ComLocation::ComList] = CommandList::Login;

    login.cmd[static_cast<int>(SendHostLoginDataCmd::PlayerColor)] = CorrespondenceManager::Instance().my_player_color;

    snprintf(login.port, 8, CorrespondenceManager::Instance().udp_port);

    std::memcpy(login.name, CorrespondenceManager::Instance().my_name,sizeof(login.name));
    //login.name = CorrespondenceManager::Instance().my_name;
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
    if (is_multi == false) return;

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
   return receive;
}

void CorrespondenceManager::UdpSend(int id, char* data, int size)
{
    if (is_multi == false) return;

    //server->UdpSend(id,data,size);
    communication_system->UdpSend(id, data, size);
}

bool CorrespondenceManager::LoginReceive()
{
    //-----受信データ-----//
    char data[512];
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
        operation_private_id = login->cmd[static_cast<int>(LoginDataCmd::OperationPrivateId)];

        //-----ホストの番号を保存-----//
        host_id = login->cmd[static_cast<int>(LoginDataCmd::HostId)];

        //-----自分以外のプレイヤーの番号を保存----//
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            opponent_player_id.at(i) = login->opponent_player_id[i];
            //Ipアドレスを保存
            SocketCommunicationManager::Instance().game_udp_server_addr[i] = login->game_udp_server_addr[i];
            std::string ip = std::to_string(opponent_player_id.at(i)) + "番目 :" +  std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4);
            DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);

            //-----色の設定-----//
            player_colors[i] = login->p_color[i];

            //-----名前の登録はIDがちゃんとした数字のものだけ-----//
            if (login->opponent_player_id[i] < 0) continue;
            DebugConsole::Instance().WriteDebugConsole(login->name[i], TextColor::Green);
            names[i] = login->name[i];

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
