#include "SocketCommunication.h"
#include"DebugConsole.h"
SocketCommunicationManager::SocketCommunicationManager()
{
    //������
    for (int i = 0; i < MAX_CLIENT; ++i)
    {
        sockaddr_in add_d{};
        add_d.sin_addr.S_un.S_addr = 0;
        //<�^�v���Ńf�[�^��ۑ�>//
        std::tuple<bool, sockaddr_in> d(false, add_d);
        game_udp_server_addr[i] = d;
        login_client_sock[i] = INVALID_SOCKET;
        //send_udp_sock[i] = INVALID_SOCKET;
    }
    http_sock = INVALID_SOCKET;
    multicast_sock = INVALID_SOCKET;
    login_tcp_server_addr.sin_addr.S_un.S_addr = 0;
    multicast_addr.sin_addr.S_un.S_addr = 0;
    FD_ZERO(&client_tcp_fds);
    FD_ZERO(&udp_fds);
    FD_ZERO(&tcp_fds);
    FD_ZERO(&multicast_fds);
    client_tcp_fds_count = 0;
}
SocketCommunicationManager::~SocketCommunicationManager()
{
    if (udp_sock != INVALID_SOCKET) closesocket(udp_sock);
    if (tcp_sock != INVALID_SOCKET) closesocket(tcp_sock);
    if (http_sock != INVALID_SOCKET) closesocket(http_sock);
    if (multicast_sock != INVALID_SOCKET) closesocket(multicast_sock);
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        sockaddr_in add_d{};
        add_d.sin_addr.S_un.S_addr = 0;
        //<�^�v���Ńf�[�^��ۑ�>//
        std::tuple<bool, sockaddr_in> d(false, add_d);
        game_udp_server_addr[i] = d;
        if (login_client_sock[i] != INVALID_SOCKET) closesocket(login_client_sock[i]);
    }

    login_tcp_server_addr.sin_addr.S_un.S_addr = 0;
    multicast_addr.sin_addr.S_un.S_addr = 0;
    FD_ZERO(&client_tcp_fds);
    FD_ZERO(&udp_fds);
    FD_ZERO(&tcp_fds);
    FD_ZERO(&multicast_fds);
    client_tcp_fds_count = 0;
}

void SocketCommunicationManager::ClearData()
{
    DebugConsole::Instance().WriteDebugConsole("�\�P�b�g�Ȃǂ̏���������");

    WSACleanup();

    if (udp_sock != INVALID_SOCKET)
    {
        closesocket(udp_sock);
        udp_sock = INVALID_SOCKET;
    }
    if (tcp_sock != INVALID_SOCKET)
    {
        closesocket(tcp_sock);
        tcp_sock = INVALID_SOCKET;
    }
    if (http_sock != INVALID_SOCKET)
    {
        closesocket(http_sock);
        http_sock = INVALID_SOCKET;
    }
    if (multicast_sock != INVALID_SOCKET)
    {
        closesocket(multicast_sock);
        multicast_sock = INVALID_SOCKET;
    }
    for (int i = 0; i < MAX_CLIENT; ++i)
    {
        sockaddr_in add_d{};
        add_d.sin_addr.S_un.S_addr = 0;
        //<�^�v���Ńf�[�^��ۑ�>//
        std::tuple<bool, sockaddr_in> d(false, add_d);
        game_udp_server_addr[i] = d;
        if (login_client_sock[i] != INVALID_SOCKET)
        {
            closesocket(login_client_sock[i]);
            login_client_sock[i] = INVALID_SOCKET;
        }
    }
    login_tcp_server_addr.sin_addr.S_un.S_addr = 0;
    multicast_addr.sin_addr.S_un.S_addr = 0;
    FD_ZERO(&client_tcp_fds);
    FD_ZERO(&udp_fds);
    FD_ZERO(&tcp_fds);
    FD_ZERO(&multicast_fds);
    client_tcp_fds_count = 0;

    WSADATA was_data;
    if (WSAStartup(MAKEWORD(2, 2), &was_data) != 0)
    {
        //�������Ɏ��s
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);

    }
}
