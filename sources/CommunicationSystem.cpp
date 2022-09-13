#include "CommunicationSystem.h"
#include"Correspondence.h"
CommunicationSystem::CommunicationSystem()
{
}

CommunicationSystem::~CommunicationSystem()
{
    WSACleanup();
}

void CommunicationSystem::AcquisitionMyIpAddress()
{
    //---�����̖��O������---//
    char host_name[50];
    gethostname(host_name, 50);
    //------�\�P�b�g���ϐ�-------//
    addrinfo hints;
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};
    //----------����ݒ�---------//
    //------�ǉ��̃I�v�V������ݒ�-----//
    hints.ai_flags = 0;
    //-------IPV4�Őڑ�--------//
    hints.ai_family = AF_INET;
    //-------UDP�ŒʐM-------//
    hints.ai_socktype = SOCK_DGRAM;
    //--�\�P�b�g�A�h���X�̃v���g�R���ݒ�--//
    hints.ai_protocol = 0;
    getaddrinfo(host_name, NULL, &hints, &addr_info);
    servent* port = getservbyname(host_name,NULL);
    addr = *((sockaddr_in*)addr_info->ai_addr);
    //�����̃A�h���X��ۑ�
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
    //----------UDP�ʐM�p�̏�����----------//
    if (InitializeHostUdpSocket(udp_port, private_id) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("�z�X�g : UDP�̃\�P�b�g�̍쐬�Ɏ��s���܂���", TextColor::Red);
        return false;
    }

    //----------TCP�ʐM�p�̏�����----------//
    if (InitializeHostTcp(tco_port,private_id) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("�z�X�g : TCP�̃\�P�b�g�̍쐬�Ɏ��s���܂���", TextColor::Red);
        return false;
    }
    return true;
}

bool CommunicationSystem::InitializeHostUdpSocket(char* port, int private_id)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //--------�\�P�b�g���ϐ�--------//
    addrinfo hints;
    //-----���������A�h���X��ۑ�����-----//
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};
    //------------����ݒ�-------------//
    //----�ǉ��̃I�v�V������ݒ�----//
    hints.ai_flags = 0;
    //----------IPV4�Őڑ�----------//
    hints.ai_family = AF_INET;
    //-----------UDP�ŒʐM-------------//
    hints.ai_socktype = SOCK_DGRAM;
    //------�\�P�b�g�A�h���X�̃v���g�R���ݒ�-----//
    hints.ai_protocol = 0;
    //----------�C���^�[�l�b�g�A�h���X���擾----------//
    if (getaddrinfo(SocketCommunicationManager::Instance().my_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("UDP �z�X�g: �C���^�[�l�b�g�A�h���X�̎擾�Ɏ��s���܂���", TextColor::Red);
        return false;
    }
    //----------�A�h���X����������-----------//
    addr = *((sockaddr_in*)addr_info->ai_addr);

    //----------�����̃A�h���X��ۑ�----------//
    instance.game_udp_server_addr[private_id] = addr;
    std::string ip = std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b4) + "�|�[�g�ԍ�" + std::to_string(addr.sin_port);
    DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);

    //----------�\�P�b�g����----------//
    instance.udp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (instance.udp_sock == INVALID_SOCKET)
    {
        //----------�\�P�b�g�̐����Ɏ��s----------//
        DebugConsole::Instance().WriteDebugConsole("UDP �z�X�g: �\�P�b�g�̐����Ɏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //----------�\�P�b�g�̎󂯓�������֘A�t����----------//
    if (bind(instance.udp_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        //----------�\�P�b�g�̎󂯓�����̊֘A�t�����s----------//
        DebugConsole::Instance().WriteDebugConsole("UDP : �\�P�b�g�̎󂯓�����̊֘A�t���Ŏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    DebugConsole::Instance().WriteDebugConsole("UDP �z�X�g: ��M�p�\�P�b�g�̐����ɐ������܂���", TextColor::Blue);
    // sock��fds�ɃZ�b�g
    FD_SET(instance.udp_sock, &instance.udp_fds);

    return true;
}

bool CommunicationSystem::InitializeHostTcp(char* port, int private_id)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //�\�P�b�g���ϐ�
    addrinfo hints;
    ZeroMemory(&hints, sizeof(addrinfo));
    //���������A�h���X��ۑ�����
    addrinfo* addr_info{ NULL };
    sockaddr_in addr{};
    //����ݒ�
    //�ǉ��̃I�v�V������ݒ�
    hints.ai_flags = 0;
    //IPV6�Őڑ�
    hints.ai_family = AF_INET;
    //UDP�ŒʐM
    hints.ai_socktype = SOCK_STREAM;
    //�\�P�b�g�A�h���X�̃v���g�R���ݒ�
    hints.ai_protocol = 0;
    ////�C���^�[�l�b�g�A�h���X���擾
    if (getaddrinfo(SocketCommunicationManager::Instance().host_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP : �C���^�[�l�b�g�A�h���X�̎擾�Ɏ��s���܂���", TextColor::Red);
        return false;
    }
    DebugConsole::Instance().WriteDebugConsole(port);

    addr = *((sockaddr_in*)addr_info->ai_addr);
    //�\�P�b�g����
    instance.tcp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (instance.tcp_sock == INVALID_SOCKET)
    {
        //�\�P�b�g�̐����Ɏ��s
        DebugConsole::Instance().WriteDebugConsole("TCP : �z�X�g  �\�P�b�g�̐����Ɏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //�\�P�b�g�̎󂯓�������֘A�t����
    if (bind(instance.tcp_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        //�\�P�b�g�̎󂯓�����̊֘A�t�����s
        DebugConsole::Instance().WriteDebugConsole("TCP : �z�X�g  �\�P�b�g�̎󂯓�����̊֘A�t���Ŏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //��u���b�L���O(�m���u���b�L���O)�ɐݒ�
    u_long i_modes{ 1 };//0���ƃu���b�L���O
    if (ioctlsocket(instance.tcp_sock, FIONBIO, &i_modes) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP : �z�X�g  ��u���b�L���O�ݒ�Ɏ��s���܂���", TextColor::Red);
        WSAGetLastError();
        return false;
    }
    // sock��fds�ɃZ�b�g
    FD_SET(instance.tcp_sock, &instance.tcp_fds);

    //���O�C���p�̃\�P�b�g�ɑ������
    instance.login_client_sock[private_id]= instance.tcp_sock;

    //-----�T�[�o�[�̑҂��󂯂��J�n-----//
    //-----�������͑҂��󂯐�(-1���Ă�͎̂�������������)-----//
    listen(instance.tcp_sock, MAX_CLIENT - 1);

    return true;
}

bool CommunicationSystem::InitializeClient(char* tcp_port, char* udp_port)
{

    //-----UDP�ʐM�p�\�P�b�g���쐬-----//
    if (InitializeClientUdpSocket(udp_port) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("�N���C�A���g : UDP��M�p�\�P�b�g�̍쐬�Ɏ��s���܂���", TextColor::Red);
        return false;
    }
    //----------TCP�ʐM�p�̏�����----------//
    if (InitializeClientTcp(tcp_port) == false)
    {
        DebugConsole::Instance().WriteDebugConsole("�N���C�A���g : TCP�̃\�P�b�g�̍쐬�Ɏ��s���܂���", TextColor::Red);
        return false;
    }
    return true;
}

bool CommunicationSystem::InitializeClientTcp(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //-------���O�C����̃A�h���X���擾-------//
    if (AcquisitionLoginAddress(port) == false) return false;
    //-------�����̃A�h���X���擾--------//
    char host_name[50];
    gethostname(host_name, 50);
    //-------�\�P�b�g���ϐ�-------//
    addrinfo hints;
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    //----------����ݒ�----------//
    //-----�ǉ��̃I�v�V������ݒ�-----//
    hints.ai_flags = 0;
    //------IPV4�Őڑ�------//
    hints.ai_family = AF_INET;
    //---------UDP�ŒʐM----------//
    hints.ai_socktype = SOCK_STREAM;
    //-----------�\�P�b�g�A�h���X�̃v���g�R���ݒ�-------------//
    hints.ai_protocol = 0;
    if (getaddrinfo(host_name, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP �N���C�A���g : �C���^�[�l�b�g�A�h���X�̎擾�Ɏ��s���܂���", TextColor::Red);
        return false;
    }
    DebugConsole::Instance().WriteDebugConsole(port);

    //--------------�\�P�b�g�𐶐�--------------//
    instance.tcp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (instance.tcp_sock == INVALID_SOCKET)
    {
        //�\�P�b�g�̐����Ɏ��s
        DebugConsole::Instance().WriteDebugConsole("TCP �N���C�A���g: �\�P�b�g�̐����Ɏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    //----------�T�[�o�[�֐ڑ�----------//
    if (connect(instance.tcp_sock, (sockaddr*)&instance.login_tcp_server_addr, sizeof(instance.login_tcp_server_addr)) != 0)
    {
        //----------�T�[�o�[�ւ̐ڑ��Ɏ��s----------//
        DebugConsole::Instance().WriteDebugConsole("TCP �N���C�A���g: �T�[�o�[�Ƃ̐ڑ��Ɏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    //-----sock��fds�ɃZ�b�g-----//
    FD_SET(instance.tcp_sock, &instance.tcp_fds);

    return true;
}

bool CommunicationSystem::InitializeClientUdpSocket(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //-------�\�P�b�g���ϐ�-------//
    addrinfo hints;
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};
    //----------����ݒ�----------//
    //-----�ǉ��̃I�v�V������ݒ�-----//
    hints.ai_flags = 0;

    //------IPV4�Őڑ�------//
    hints.ai_family = AF_INET;

    //---------UDP�ŒʐM----------//
    hints.ai_socktype = SOCK_DGRAM;

    //-----------�\�P�b�g�A�h���X�̃v���g�R���ݒ�-------------//
    hints.ai_protocol = 0;

    if (getaddrinfo(SocketCommunicationManager::Instance().my_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("UDP �N���C�A���g : �C���^�[�l�b�g�A�h���X�̎擾�Ɏ��s���܂���", TextColor::Red);
        return false;
    }

    //----------�A�h���X����������-----------//
    //--(�N���C�A���g�̓��O�C���f�[�^�������^�C�~���O�ŃA�h���X�z��ɓ���邩�炱���ł͕ۑ����Ȃ�)--//
    addr = *((sockaddr_in*)addr_info->ai_addr);
    std::string ip =std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + std::to_string(addr.sin_addr.S_un.S_un_b.s_b4) + "�|�[�g�ԍ�" + std::to_string(addr.sin_port);
    DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Blue);
    DebugConsole::Instance().WriteDebugConsole(port,TextColor::Blue);

    //--------------�\�P�b�g�𐶐�--------------//
    instance.udp_sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);

    if (instance.udp_sock == INVALID_SOCKET)
    {
        //�\�P�b�g�̐����Ɏ��s
        DebugConsole::Instance().WriteDebugConsole("UDP �N���C�A���g: �\�P�b�g�̐����Ɏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }

    //�\�P�b�g�̎󂯓�������֘A�t����
    if (bind(instance.udp_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        //�\�P�b�g�̎󂯓�����̊֘A�t�����s
        DebugConsole::Instance().WriteDebugConsole("UDP �N���C�A���g: �\�P�b�g�̎󂯓�����̊֘A�t���Ŏ��s���܂���", TextColor::Red);
        int error = WSAGetLastError();
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return false;
    }
    DebugConsole::Instance().WriteDebugConsole("UDP �N���C�A���g: ��M�p�\�P�b�g�̐����ɐ������܂���", TextColor::Blue);
    // sock��fds�ɃZ�b�g
    FD_SET(instance.udp_sock, &instance.udp_fds);

    return true;
}

int CommunicationSystem::LoginReceive(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    fd_set client_fd{};
    timeval tv;
    //----------�b-----------//
    tv.tv_sec = 0;
    //----------�~���b----------//
    tv.tv_usec = 0;
    //----------�ǂݍ��݊Ď��ϐ����R�s�[���ĊĎ�����----------//
    memcpy(&client_fd, &instance.tcp_fds, sizeof(fd_set));

    int  judg = select(static_cast<int>(instance.tcp_sock + 1), &client_fd, NULL, NULL, &tv);
    if (judg < 0) return -1;

    int recv_size = recv(instance.tcp_sock, data, size, 0);
    //�G���[�̎���SOCKET_ERROR������
    if (recv_size == SOCKET_ERROR)
    {
        //�R���\�[����ʂɏo��
        if (tcp_error_num != WSAGetLastError())
        {
            tcp_error_num = WSAGetLastError();
            DebugConsole::Instance().WriteDebugConsole("���O�C�� : receive failed", TextColor::Red);
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
    //----------�f�[�^���T�[�o�[(�z�X�g)�ɑ��M����----------//
    int send_size = send(instance.tcp_sock, data, size,0);
    //�G���[�̎���SOCKET_ERROR������
    if (send_size == SOCKET_ERROR)
    {
        //�R���\�[����ʂɏo��
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
        std::string text =  "�T�[�o�[(�z�X�g)��" + std::to_string(send_size) + "�o�C�g���M���܂���";

        DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
    }
}

void CommunicationSystem::TcpAccept(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //-----�ǂݍ��݊Ď��ϐ�-----//
    fd_set fd_work;
    timeval tv;
    //-----�b-----//
    tv.tv_sec = 0;
    //-----�~���b-----//
    tv.tv_usec = 0;
    //-----fd_work�ɃR�s�[����-----//
    memcpy(&fd_work, &instance.tcp_fds, sizeof(fd_set));
    //-----fds�ɐݒ肳�ꂽ�\�P�b�g���ǂݍ��݉\�ɂȂ�܂ő҂�-----//
    int n = select(static_cast<int>(instance.tcp_sock + 1), &fd_work, NULL, NULL, &tv);
    //----------�^�C���A�E�g�̏ꍇselect��0��Ԃ�----------//
    if (n <= 0) return;
    int len = sizeof(sockaddr_in);
    sockaddr_in addr;
    SOCKET sock{ INVALID_SOCKET };
    //----------�ڑ���ҋ@����----------//
    sock = accept(instance.tcp_sock, (sockaddr*)&addr, &len);
    if (sock == INVALID_SOCKET)
    {
        if (tcp_error_num != WSAGetLastError())
        {
            tcp_error_num = WSAGetLastError();
            std::string text = "TCP : ���O�C�� : error number:" + std::to_string(tcp_error_num);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
            return;
        }
    }
    else
    {
        //----------�N���C�A���g����----------//
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            //----------�A�h���X����������ԂȂ�Ƃ΂�----------//
            if ((instance.game_udp_server_addr[i].sin_addr.S_un.S_addr == 0 &&
                instance.login_client_sock[i] == INVALID_SOCKET) ||
                i == CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
            //------�ۑ����Ă���A�h���X�Ǝ�M�����A�h���X�������Ȃ牽�Ԗڂ���Ԃ�------//
            if (addr.sin_addr.S_un.S_addr == instance.game_udp_server_addr[i].sin_addr.S_un.S_addr)
            {
                DebugConsole::Instance().WriteDebugConsole("�ۑ����Ă���A�h���X�ł�", TextColor::White);
                return;
            }
        }
        //----------�N���C�A���g�󂯓���----------//
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            //----------�A�h���X����������ԂȂ�Ƃ΂�----------//
            if (instance.game_udp_server_addr[i].sin_addr.S_un.S_addr != 0 &&
                instance.login_client_sock[i] != INVALID_SOCKET) continue;

            instance.game_udp_server_addr[i] = addr;
            instance.login_client_sock[i] = sock;
            //-----sock��fds�ɃZ�b�g-----//
            FD_SET(instance.login_client_sock[i], &instance.client_tcp_fds);
            instance.client_tcp_fds_count++;
            DebugConsole::Instance().WriteDebugConsole("���O�C�����Ă��܂���", TextColor::White);
            return;
        }
    }
}

int CommunicationSystem::TcpHostReceive(char* data, int size, int operation_private_id)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    fd_set client_fd{};
    timeval tv;
    //----------�b-----------//
    tv.tv_sec = 0;
    //----------�~���b----------//
    tv.tv_usec = 0;
    //----------�f�[�^����M����----------//
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        //-----�����\�P�b�g�ɉ��������Ă��Ȃ��ꍇ�������̔ԍ��Ɠ����ԍ��Ȃ玟�ɂ���-----//
        if (instance.login_client_sock[i] == INVALID_SOCKET || i == operation_private_id) continue;
        //----------�ǂݍ��݊Ď��ϐ����R�s�[���ĊĎ�����----------//
        memcpy(&client_fd, &instance.client_tcp_fds, sizeof(fd_set));
        int  judg = select(static_cast<int>(instance.login_client_sock[i] + instance.client_tcp_fds_count), &client_fd, NULL, NULL, &tv);
        if (judg >= 1)
        {
            //----------�f�[�^����M����---------//
            int recv_size = recv(instance.login_client_sock[i], data, size, 0);
            if (recv_size == SOCKET_ERROR)
            {
                //�R���\�[����ʂɏo��
                if (tcp_error_num != WSAGetLastError())
                {
                    tcp_error_num = WSAGetLastError();
                    DebugConsole::Instance().WriteDebugConsole("���O�C�� : receive failed", TextColor::Red);
                    std::string text = "error number:" + std::to_string(tcp_error_num);
                    DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                    continue;
                }
            }
            else
            {
                DebugConsole::Instance().WriteDebugConsole("���O�C���f�[�^����M���܂���", TextColor::White);
                return i;
            }
        }
    }
    //----------�����܂ŗ����牽����M���Ă��Ȃ�����-1��Ԃ��Ă���----------//
    return -1;
}

int CommunicationSystem::TcpClientReceive(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    fd_set client_fd{};
    timeval tv;
    //----------�b-----------//
    tv.tv_sec = 0;
    //----------�~���b----------//
    tv.tv_usec = 0;
    //----------�f�[�^����M����----------//
    //----------�ǂݍ��݊Ď��ϐ����R�s�[���ĊĎ�����----------//
    memcpy(&client_fd, &instance.tcp_fds, sizeof(fd_set));
    int  judg = select(static_cast<int>(instance.tcp_sock + 1), &client_fd, NULL, NULL, &tv);
    if (judg >= 1)
    {
        //----------�f�[�^����M����---------//
        int recv_size = recv(instance.tcp_sock, data, size, 0);
        if (recv_size == SOCKET_ERROR)
        {
            //�R���\�[����ʂɏo��
            if (tcp_error_num != WSAGetLastError())
            {
                tcp_error_num = WSAGetLastError();
                DebugConsole::Instance().WriteDebugConsole("���O�C�� : receive failed", TextColor::Red);
                std::string text = "error number:" + std::to_string(tcp_error_num);
                DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                return -1;
            }
        }
        else
        {
            DebugConsole::Instance().WriteDebugConsole("���O�C���f�[�^����M���܂���", TextColor::White);
            return recv_size;
        }
    }
    //----------�����܂ŗ����牽����M���Ă��Ȃ�����-1��Ԃ��Ă���----------//
    return -1;
}

void CommunicationSystem::TcpSend(int id, char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //�f�[�^���N���C�A���g�ɑ���
    int send_size = send(instance.login_client_sock[id],data,size,0);
    //�G���[�̎���SOCKET_ERROR������
    if (send_size == SOCKET_ERROR)
    {
        //�R���\�[����ʂɏo��
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
        std::string text = std::to_string(id) + "�Ԗڂ�TCP�ʐM��" + std::to_string(send_size) + "�o�C�g���M���܂���";

        DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
    }
}

bool CommunicationSystem::CloseTcpHost(char* data, int size,int operation_private_id)
{

    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (instance.login_client_sock[i] == INVALID_SOCKET || i == operation_private_id) continue;
        //�f�[�^���N���C�A���g�ɑ���
        int send_size = send(instance.login_client_sock[i], data, size, 0);
        //�G���[�̎���SOCKET_ERROR������
        if (send_size == SOCKET_ERROR)
        {
            //�R���\�[����ʂɏo��
            if (tcp_error_num != WSAGetLastError())
            {
                tcp_error_num = WSAGetLastError();
                DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
                std::string text = "error number:" + std::to_string(tcp_error_num);
                DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
                //���M�Ɏ��s������false���������Ă���
                return false;
            }
        }
        else
        {
            std::string text = std::to_string(i) + "�Ԗڂ�TCP�ʐM��" + std::to_string(send_size) + "�o�C�g���M���܂���";
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
        }
    }

    if (closesocket(instance.tcp_sock) != 0)
    {
        tcp_error_num = WSAGetLastError();
        DebugConsole::Instance().WriteDebugConsole("TCP�ؒf�Ɏ��s���܂���", TextColor::Red);
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
        DebugConsole::Instance().WriteDebugConsole("TCP�ؒf�Ɏ��s���܂���", TextColor::Red);
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
    //----------�b-----------//
    tv.tv_sec = 0;
    //----------�~���b----------//
    tv.tv_usec = 0;
    //----------�ǂݍ��݊Ď��ϐ�----------//
    fd_set fd_work;
    //----------fd_work�ɃR�s�[����----------//
    memcpy(&fd_work, &instance.udp_fds, sizeof(fd_set));
    //-----fds�ɐݒ肳�ꂽ�\�P�b�g���ǂݍ��݉\�ɂȂ�܂ő҂�-----//
    int n = select(0, &fd_work, NULL, NULL, &tv);
    //----------�^�C���A�E�g�̏ꍇselect��0��Ԃ�----------//
    if (n <= 0)
    {
        //DebugConsole::Instance().WriteDebugConsole("�^�C���A�E�g", TextColor::Blue);
        return -1;
    }
    sockaddr_in addr;
    int addr_length{ sizeof(sockaddr_in) };
    //----------�f�[�^�̎�M----------//
    int recv_size = recvfrom(instance.udp_sock, data, size, 0, (sockaddr*)&addr, &addr_length);
    //-----�G���[�̎���SOCKET_ERROR������-----//
    if (recv_size == SOCKET_ERROR)
    {
        //----------�R���\�[����ʂɏo��----------//
        int error = WSAGetLastError();
        DebugConsole::Instance().WriteDebugConsole("receive failed", TextColor::Red);
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        return -1;
    }
    //----------�N���C�A���g����----------//
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        //----------�A�h���X����������Ԃ������̔ԍ��Ɠ�����������----------//
        if (instance.game_udp_server_addr[i].sin_addr.S_un.S_addr == 0 || i == CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //------�ۑ����Ă���A�h���X�Ǝ�M�����A�h���X�������Ȃ牽�Ԗڂ���Ԃ�------//
        if (addr.sin_addr.S_un.S_addr == instance.game_udp_server_addr[i].sin_addr.S_un.S_addr)
        {
            std::string ip = std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4) + "��M�|�[�g�ԍ�" + std::to_string(addr.sin_port) + "   :  �ۑ��|�[�g" + std::to_string(instance.game_udp_server_addr[i].sin_port);
            std::string text =ip + "�A�h���X" +  std::to_string(i) + "�Ԗڂ̃A�h���X�Ŏ�M���܂���";
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Blue);

            return i;
        }
    }
    //----------�����܂ŗ�����ő�ڑ����ɒB���Ă���̂�-1��Ԃ��Ă���----------//
    return -1;

}

void CommunicationSystem::UdpSend(int id, char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

    //�f�[�^�𑗂�(�����Ŏg���\�P�b�g�͂��ꂼ��̃|�[�g�ԍ��ɍ��킹�č�����\�P�b�g)
    int send_size = sendto(instance.udp_sock, data, size, 0, (sockaddr*)&instance.game_udp_server_addr[id], sizeof(instance.game_udp_server_addr[id]));

    //�G���[�̎���SOCKET_ERROR������
    if (send_size == SOCKET_ERROR)
    {
        //�R���\�[����ʂɏo��
        int error = WSAGetLastError();
        DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
        std::string text = "error number:" + std::to_string(error);
        DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
    }
    else
    {
        std::string text = std::to_string(id) + "�Ԗڂ�" + std::to_string(send_size) + "�o�C�g���M���܂���";

        DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
    }
}

void CommunicationSystem::UdpSend(char* data, int size)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        //-----�����A�h���X���ۑ�����Ă��Ȃ����������̔ԍ��Ɠ����Ȃ��΂�-----//
        if (instance.game_udp_server_addr[i].sin_addr.S_un.S_addr == 0 ||
            i == CorrespondenceManager::Instance().GetOperationPrivateId()) continue;

        //-----�f�[�^�𑗐M����-----//
        int send_size = sendto(instance.udp_sock, data, size, 0, (sockaddr*)&instance.game_udp_server_addr[i], sizeof(instance.game_udp_server_addr[i]));

        //�G���[�̎���SOCKET_ERROR������
        if (send_size == SOCKET_ERROR)
        {
            //�R���\�[����ʂɏo��
            int error = WSAGetLastError();
            DebugConsole::Instance().WriteDebugConsole("send failed", TextColor::Red);
            std::string text = "error number:" + std::to_string(error);
            DebugConsole::Instance().WriteDebugConsole(text, TextColor::Red);
        }
        else
        {
            std::string ip = std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4) + "�|�[�g�ԍ�" + std::to_string(instance.game_udp_server_addr[i].sin_port);
            std::string text = ip + "�̃A�h���X :" +std::to_string(i) + "�Ԗڂ�" + std::to_string(send_size) + "�o�C�g���M���܂���";

            DebugConsole::Instance().WriteDebugConsole(text, TextColor::White);
        }
    }
}

void CommunicationSystem::LogoutClient(int client_id)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //----------�A�h���X�̏�����----------//
    if (instance.game_udp_server_addr[client_id].sin_addr.S_un.S_addr != 0)
    {
        std::string text = std::to_string(client_id) + "�Ԗڂ̃N���C�A���g�����O�A�E�g���܂���";
        DebugConsole::Instance().WriteDebugConsole(text,TextColor::Red);
        instance.game_udp_server_addr[client_id].sin_addr.S_un.S_addr = 0;
    }

}

bool CommunicationSystem::AcquisitionLoginAddress(char* port)
{
    SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
    //---------�\�P�b�g���ϐ�--------//
    addrinfo hints;
    //-------���������A�h���X��ۑ�����-------//
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    sockaddr_in addr{};
    //---------����ݒ�---------//
    //----------�ǉ��̃I�v�V������ݒ�---------//
    hints.ai_flags = 0;
    //-----IPV4�Őڑ�-----//
    hints.ai_family = AF_INET;
    //------UDP�ŒʐM-------//
    hints.ai_socktype = SOCK_DGRAM;
    //---�\�P�b�g�A�h���X�̃v���g�R���ݒ�---//
    hints.ai_protocol = 0;
    //-------�C���^�[�l�b�g�A�h���X���擾-------//
    if (getaddrinfo(SocketCommunicationManager::Instance().host_ip, port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("TCP �N���C�A���g: ���O�C����̃A�h���X�擾�Ɏ��s���܂����B", TextColor::Red);
        return false;
    }
    instance.login_tcp_server_addr = *((sockaddr_in*)addr_info->ai_addr);
    std::string ip = std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b1) + std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b2) + std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b3) + std::to_string(instance.login_tcp_server_addr.sin_addr.S_un.S_un_b.s_b4);
    DebugConsole::Instance().WriteDebugConsole("���O�C����̃A�h���X�擾", TextColor::Green);
    DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);
    return true;
}

sockaddr_in CommunicationSystem::SetSendPortAddress(sockaddr_in addr, char* port)
{
    sockaddr_in create;
    //---------�\�P�b�g���ϐ�--------//
    addrinfo hints;
    //-------���������A�h���X��ۑ�����-------//
    addrinfo* addr_info{ NULL };
    ZeroMemory(&hints, sizeof(addrinfo));
    //---------����ݒ�---------//
    //----------�ǉ��̃I�v�V������ݒ�---------//
    hints.ai_flags = 0;
    //-----IPV4�Őڑ�-----//
    hints.ai_family = AF_INET;
    //------UDP�ŒʐM-------//
    hints.ai_socktype = SOCK_DGRAM;
    //---�\�P�b�g�A�h���X�̃v���g�R���ݒ�---//
    hints.ai_protocol = 0;
    //-------�C���^�[�l�b�g�A�h���X���擾-------//
    std::string ip = std::to_string(addr.sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(addr.sin_addr.S_un.S_un_b.s_b4);

    if (getaddrinfo(ip.c_str(),port, &hints, &addr_info) != 0)
    {
        DebugConsole::Instance().WriteDebugConsole("UDP �z�X�g: ���O�C����̃A�h���X�擾�Ɏ��s���܂����B", TextColor::Red);
    }
    create = *((sockaddr_in*)addr_info->ai_addr);

    return create;
}
