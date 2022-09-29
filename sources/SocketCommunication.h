#pragma once
#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>
#pragma comment(lib, "Ws2_32.lib")
#define MAX_CLIENT 4
//==========================================================
//
//                ����̒ʐM�ɕK�v�ȏ��������Ă���Ǘ��N���X
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
    //-----UDP�ʐM��M�p�̃\�P�b�g-----//
    SOCKET udp_sock{ INVALID_SOCKET };

    //-----TCP�ʐM�̃\�P�b�g-----//
    SOCKET tcp_sock{ INVALID_SOCKET };

    //-----���O�C���Ŏg���N���C�A���g�̃\�P�b�g-----//
    SOCKET login_client_sock[MAX_CLIENT];

    //-----���O�C���Ŏg��TCP�ʐM�̃T�[�o�[�A�h���X-----//
    sockaddr_in login_tcp_server_addr;

    //-----�Q�[�����Ŏg��UDP�̃T�[�o�[�A�h���X(�N���C�A���g�̐�������)-----//
    sockaddr_in game_udp_server_addr[MAX_CLIENT];

    //-----UDP�ǂݍ��݊Ď��p-----//
    fd_set udp_fds;

    //-----TCP�ǂݍ��݊Ď��p-----//
    fd_set tcp_fds;

    //-----TCP�N���C�A���g�̃\�P�b�g�̓ǂݍ��݊Ď��p-----//
    fd_set client_tcp_fds;

    //-----TCP�N���C�A���g��fd�̐ڑ������J�E���g-----//
    int client_tcp_fds_count{ 0 };

    //�z�X�g��IP�A�h���X
    char host_ip[32]{ "127.0.0.1" };
    char my_ip[32]{ "127.0.0.1" };

public:
    std::mutex& GetMutex() { return mutex; }

public:
    void ClearData();
private:
    std::mutex mutex;

};