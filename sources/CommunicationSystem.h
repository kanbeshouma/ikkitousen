#pragma once
#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include"SocketCommunication.h"
#include"DebugConsole.h"
#include"EnemyStructuer.h"
//-----------------------------------------------------------------------------------------------------------
//
//              �f�[�^�𑗎�M���邽�߂̋@�\�̏������y��
//                                �������Ǘ�����N���X
//
//-----------------------------------------------------------------------------------------------------------

class CommunicationSystem
{
public:
    CommunicationSystem();
    ~CommunicationSystem();
public:
    //------------------------�������֌W----------------------------//

    ////-----������IP�A�h���X���擾-------//
    void AcquisitionMyIpAddress();

#pragma region HostInitialize
    ////------�z�X�g�̏ꍇ�̏�����------//
//===========================
//�߂�l   : �����������ǂ���
//��1���� : TCP�|�[�g�ԍ�
//��2���� : UDP�|�[�g�ԍ�
//��3���� : �����̔ԍ�
    bool InitializeHost(char* tco_port, char* udp_port, int private_id);
private:
    ////------�z�X�g�̏ꍇ�̎�M�pUDP�\�P�b�g�̏�����------//
    //===========================
    //�߂�l   : �����������ǂ���
    //��1���� : �|�[�g�ԍ�
    //��2���� : �����̔ԍ�
    bool InitializeHostUdpSocket(char* port, int private_id);

    ////------�z�X�g�̏ꍇ��TCP�̏�����------//
    //===========================
    //�߂�l   : �����������ǂ���
    //��1���� : �|�[�g�ԍ�
    //��2���� : �����̔ԍ�
    bool InitializeHostTcp(char* port, int private_id);

#pragma endregion

public:

#pragma region ClientInitialize
    ////------�N���C�A���g�̏ꍇ�̏�����------//
//===========================
//�߂�l   : �����������ǂ���
//��1���� : �|�[�g�ԍ�
    bool InitializeClient(char* tco_port, char* udp_port);
private:
    ////------�N���C�A���g�̏ꍇ��TCP�̏�����------//
    //===========================
    //�߂�l   : �����������ǂ���
    //��1���� : �|�[�g�ԍ�
    bool InitializeClientTcp(char* port);

    ////------�N���C�A���g�̏ꍇ�̎�M�pUDP�\�P�b�g�̏�����------//
    //===========================
    //�߂�l   : �����������ǂ���
    //��1���� : �|�[�g�ԍ�
    bool InitializeClientUdpSocket(char* port);

#pragma endregion

public:
    /// <summary>
    ///HTTP���N�G�X�g
    /// </summary>
    int HttpRequest();
public:
    //--------------------------���O�C���֌W------------------------//

#pragma region Login
                    ////-----���O�C���p�̎�M(TCP�ʐM)-----//
    //==========================
    //�߂�l   : ��M�����f�[�^�̃T�C�Y
    //��1���� : ��M�����f�[�^������ϐ�
    //��2���� : �f�[�^������ϐ��̃T�C�Y
    int LoginReceive(char* data, int size);

    ////-----���O�C���p�̑��M(TCP�ʐM)-----//
    //==========================
    //��1���� : ���M����f�[�^
    //��2���� : �f�[�^�̃T�C�Y
    void LoginSend(char* data, int size);

#pragma endregion

public:
    //------------------------------����M-----------------------------//
#pragma region TCP
    ////----------TCP�ʐM�ڑ��m��----------//
    //==========================
    void TcpAccept(char* port);

    ////----------TCP�z�X�g��M----------//
    //==========================
    //�߂�l   : ��M��������̔ԍ���Ԃ�
    //��1���� : ��M�����f�[�^������ϐ�
    //��2���� : �f�[�^������ϐ��̃T�C�Y
    //��3���� : �����̔ԍ�
    int TcpHostReceive(char* data, int size,int operation_private_id);

    ////----------TCP�N���C�A���g��M----------//
    //==========================
    //�߂�l   : ��M�����f�[�^�̃T�C�Y��Ԃ�
    //��1���� : ��M�����f�[�^������ϐ�
    //��2���� : �f�[�^������ϐ��̃T�C�Y
    int TcpClientReceive(char* data, int size);

    ////----------TCP:����̑���ɑ��M----------//
    //==========================
    //��1���� : ���肽������̔ԍ�
    //��2���� : ���肽���f�[�^
    //��3���� : ����f�[�^�̌^�̃T�C�Y
    void TcpSend(int id, char* data, int size);

    ////----------TCP:�z�X�g�ɑ��M----------//
    //==========================
    //��2���� : ���肽���f�[�^
    //��3���� : ����f�[�^�̌^�̃T�C�Y
    void TcpSend(char* data, int size);

    ////----------TCP:�N���C�A���g�ɑ��M-----------//
    //==========================
    //��2���� : ���肽���f�[�^
    //��3���� : ����f�[�^�̌^�̃T�C�Y
    void TcpSendAllClient(char* data, int size);

    ////----------TCP�z�X�g�I�����M----------//
    //==========================
    //�߂�l   : �S����tcp�ʐM�I���̃f�[�^�𑗐M���Đؒf������������
    //��1���� : ����f�[�^
    //��2���� : ����f�[�^�̃T�C�Y
    //��3���� : �����̔ԍ�
    bool CloseTcpHost(char* data, int size,int operation_private_id);

    ////----------TCP�N���C�A���g�I��----------//
    //==========================
    //�߂�l   : �ؒf�����������ǂ���
    bool CloseTcpClient();
#pragma endregion

#pragma region UDP
    ////----------UDP��M----------//
//==========================
//��1���� : ���肽������̔ԍ�
//��1���� : ��M�����f�[�^������ϐ�
//��2���� : �f�[�^������ϐ��̃T�C�Y
    int UdpReceive(char* data, int size);

    ////----------UDP:����̑���ɑ��M----------//
    //==========================
    //��1���� : ���肽������̔ԍ�
    //��2���� : ���肽���f�[�^
    //��3���� : ����f�[�^�̌^�̃T�C�Y
    void UdpSend(int id, char* data, int size);

    ////----------UDP:�ڑ��ґS���ɑ��M----------//
    //==========================
    //��1���� : ���肽���f�[�^
    //��2���� : ����f�[�^�̌^�̃T�C�Y
    void UdpSend(char* data, int size);

#pragma endregion

#pragma region MultiCast
    ////------�}���`�L���X�g���M�̏ꍇ�̏�����------//
    //===========================
    //�߂�l   : �����������ǂ���
    bool InitializeMultiCastSend();

    ////------�}���`�L���X�g��M�̏ꍇ�̏�����------//
    //===========================
    //�߂�l   : �����������ǂ���
    bool InitializeMultiCastReceive();

    ////----------�}���`�L���X�g���M----------//
    //===========================
    //��1���� : �n���f�[�^
    //��2���� : �f�[�^�^�̃T�C�Y
    void MultiCastSend(char* data, int size);

    ////------�}���`�L���X�g�̎�M�֐�------//
    //===========================
    //��1���� : ��M�����f�[�^�̕ۑ��ϐ�
    //��2���� : �ۑ��ϐ��̑傫��
    bool MultiCastReceive(char* data, int size);
#pragma endregion


    ////----------���O�A�E�g�����N���C�A���g�̍폜----------//
    //==========================
    //��1���� : ���O�A�E�g����N���C�A���g�̔ԍ�
    void LogoutClient(int client_id);

    ////-------���O�C�����ė�������̃A�h���X���g�p���đ����̃|�[�g�ԍ��ɍ��킹���A�h���X���擾����-----//
    //===========================
    //�߂�l   : �|�[�g�ԍ�����v���Ă���A�h���X
    //��1���� : ��M�����A�h���X
    //��2���� : �����̃|�[�g�ԍ�
    sockaddr_in SetSendPortAddress(sockaddr_in addr, char* port);

private:
    ////-------���O�C������z�X�g�̃A�h���X���擾-----//
    //===========================
    //�߂�l   : �A�h���X���擾�ł������ǂ���
    //��1���� : �|�[�g�ԍ�;
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
    //<�w�b�_��������菜�������ǂ���>//
    bool header_check = false;
    //<���������擾���Ă��̕�����菜�������ǂ���>//
    bool char_num_check = false;
    //<������������>//
    int char_count{};
    //���v�T�C�Y
    int all_size = 0;
    //Web�T�[�o�[����󂯎�����G�̃p�����[�^�f�[�^
    std::vector<WebEnemy::WebEnemyParamPack> web_enemy_data;
public:
    //<Web�T�[�o�[����擾�����f�[�^�̃Q�b�^�[>//
    std::vector<WebEnemy::WebEnemyParamPack> GetWebEnemyParamPack() { return web_enemy_data; }
};