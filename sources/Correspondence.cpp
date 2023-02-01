#include"Correspondence.h"
#include"NetWorkInformationStucture.h"
#include"DebugConsole.h"
CorrespondenceManager::CorrespondenceManager()
{

    communication_system = std::make_unique<CommunicationSystem>();
    //�z������T�C�Y����
    opponent_player_id.resize(MAX_CLIENT);
    for (int i = 0; i < opponent_player_id.size(); i++)
    {
        opponent_player_id.at(i) = -1;
        names[i] = "";
        player_colors[i] = 0;
    }
    //���ꂼ��̃|�[�g�ԍ���ݒ�
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
        DebugConsole::Instance().WriteDebugConsole("HTTP���N�G�X�g����",TextColor::Green);
        return true;
    }
    else
    {
        DebugConsole::Instance().WriteDebugConsole("HTTP���N�G�X�g���s",TextColor::Red);
        return false;
    }
    return false;
}

bool CorrespondenceManager::InitializeServer()
{
    //----------������\�P�b�g�̏��Ȃǂ���U���Z�b�g����----------//
    SocketCommunicationManager::Instance().ClearData();
    //----------�\�P�b�g���̏�����----------//
    return communication_system->InitializeHost(tcp_port,udp_port, operation_private_id);
}

#if 0
bool CorrespondenceManager::InitializeMultiCastSend()
{
    if (communication_system->InitializeMultiCastSend())
    {
        DebugConsole::Instance().WriteDebugConsole("�}���`�L���X�g���M����������", TextColor::Green);
        return true;
    }
    else
    {
        DebugConsole::Instance().WriteDebugConsole("�}���`�L���X�g���M���������s", TextColor::Red);
        return false;
    }

    return false;
}

bool CorrespondenceManager::InitializeMultiCastReceive()
{
    if (communication_system->InitializeMultiCastReceive())
    {
        DebugConsole::Instance().WriteDebugConsole("�}���`�L���X�g��M����������", TextColor::Green);
        return true;
    }
    else
    {
        DebugConsole::Instance().WriteDebugConsole("�}���`�L���X�g��M���������s", TextColor::Red);
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
    //-----�}���`�v���C�łȂ������珈�����~�߂�-----//
    if (is_multi == false) return -1;

    //----------�ڑ����ė����v���C���[��ID��Ԃ�----------//
    int id = communication_system->UdpReceive(data, size);
    //-----�����R�}���h��-1�Ȃ牽�����Ȃ�-----//
    if (data[0] == -1)
    {
        DebugConsole::Instance().WriteDebugConsole("�R�}���h��-1�ł���", TextColor::Blue);
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
    //----------������\�P�b�g�̏��Ȃǂ���U���Z�b�g����----------//
    SocketCommunicationManager::Instance().ClearData();
    //----------�\�P�b�g�̏�����----------//
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
    //-----�����R�}���h��-1�Ȃ牽�����Ȃ�-----//
    if (data[0] == -1) return -1;
    return id;

}

int CorrespondenceManager::TcpClientReceive(char* data, int size)
{
    //----------�N���C�A���g���̎�M������----------//
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
    //-----��M�f�[�^-----//
    char data[512];
    ZeroMemory(data, sizeof(data));
    //----�f�[�^����M----//
    //int size = client->Receive(SignalType::UDP, data, sizeof(data));
    int size = communication_system->LoginReceive(data, sizeof(data));
    //�f�[�^�̃T�C�Y��0�ȉ��Ȃ牽����M���Ă��Ȃ�����return
    if (size <= 0) return false;
    //�f�[�^�̃R�}���h��-1�Ȃ牽������Ă��Ȃ�����return
    if (data[0] == -1) return false;
    //�擪�o�C�g�ɂ�镪��
    switch (data[0])
    {
        //-------���O�C��--------//
    case CommandList::Login:
    {
        LoginData* login = (LoginData*)data;
        //------�����̔ԍ���ۑ�-----//
        operation_private_id = login->cmd[static_cast<int>(LoginDataCmd::OperationPrivateId)];

        //-----�z�X�g�̔ԍ���ۑ�-----//
        host_id = login->cmd[static_cast<int>(LoginDataCmd::HostId)];

        //-----�����ȊO�̃v���C���[�̔ԍ���ۑ�----//
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            opponent_player_id.at(i) = login->opponent_player_id[i];
            //Ip�A�h���X��ۑ�
            SocketCommunicationManager::Instance().game_udp_server_addr[i] = login->game_udp_server_addr[i];
            std::string ip = std::to_string(opponent_player_id.at(i)) + "�Ԗ� :" +  std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(login->game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4);
            DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);

            //-----�F�̐ݒ�-----//
            player_colors[i] = login->p_color[i];

            //-----���O�̓o�^��ID�������Ƃ��������̂��̂���-----//
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
    //�z������T�C�Y����
    for (int i = 0; i < opponent_player_id.size(); i++)
    {
        opponent_player_id.at(i) = -1;
    }
}
