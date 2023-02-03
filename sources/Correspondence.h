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
    //-----�T�[�o�[-----//

    //-------������IP�A�h���X���擾-------//
    //===========================
    void AcquisitionMyIpAddress();

    /// <summary>
    ///HTTP���N�G�X�g
    /// </summary>
    bool HttpRequest();

    bool InitializeServer();

#if 0
    //<�}���`�L���X�g������>//
////------�}���`�L���X�g���M�̏ꍇ�̏�����------//
//===========================
//�߂�l   : �����������ǂ���
    bool InitializeMultiCastSend();

    ////------�}���`�L���X�g��M�̏ꍇ�̏�����------//
    //===========================
    //�߂�l   : �����������ǂ���
    bool InitializeMultiCastReceive();

#endif // 0

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

#pragma region UDP
    ////------UDP�̎�M�֐�------//
//===========================
//�߂�l   : �ڑ��ԍ�
//��1���� : ��M�����f�[�^�̕ۑ��ϐ�
//��2���� : �ۑ��ϐ��̑傫��
    int UdpReceive(char* data, int size);

    ////----------UDP:����̑���ɑ��M----------//
    //===========================
    //��1���� : ���Ԗڂ̃N���C�A���g���ǂ���
    //��2���� : �n���f�[�^
    //��3���� : �f�[�^�^�̃T�C�Y
    void UdpSend(int id, char* data, int size);

    ////----------UDP:�ڑ��ґS���ɑ��M----------//
    //===========================
    //��1���� : �n���f�[�^
    //��2���� : �f�[�^�^�̃T�C�Y
    void UdpSend(char* data, int size);

#pragma endregion


#pragma region TCP
    ////------TCP�ڑ��m���֐�------//
    //===========================
    void TcpAccept();

    ////------TCP�̎�M�֐�------//
    //===========================
    //�߂�l   : �ڑ��ԍ�
    //��1���� : ��M�����f�[�^�̕ۑ��ϐ�
    //��2���� : �ۑ��ϐ��̑傫��
    int TcpHostReceive(char* data, int size);

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

    ////----------TCP�I�����M----------//
    //==========================
    //�߂�l   : �S����tcp�ʐM�I���̃f�[�^�𑗐M���Đؒf������������
    bool CloseTcpHost();

    ////----------TCP�N���C�A���g�I��----------//
    //==========================
    //�߂�l   : �ؒf�����������ǂ���
    bool CloseTcpClient();
#pragma endregion


#pragma region Login
    ////----------TCP�Ń��O�C�����邱�Ƃ𑗐M----------//
//==========================
    void Login();

    ////-----�N���C�A���g���̃\�P�b�g�̏�����-----//
    bool InitializeClient();

    ////-----���O�C���f�[�^�̎�M-----//
    bool LoginReceive();

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

public:
    //--------------------------�Z�b�^�[---------------------------//
    void SetOperationPrivateId(int id) { operation_private_id = id; }
    void SetOpponentPlayerId(std::vector<int> id) { opponent_player_id = id; }
    void SetOpponentPlayerId(int id) { opponent_player_id.at(id) = id; }
    void SetMultiPlay(bool multi) { is_multi = multi; }

    ////-----����-----//
    //===========
    //��1���� : �ݒ肵�����ꏊ
    //��2���� : �ݒ肵�����l
    void SetOpponentPlayerId(int id,int num) { opponent_player_id.at(id) = num; }
    void SetHostId(int id) { host_id = id; }
    char* GetUdpPort(int id) { return udp_port; }

    //--------------------------�Q�b�^�[---------------------------//
    int GetOperationPrivateId() { return operation_private_id;}
    int GetHostId() { return host_id; }
    std::vector<int> GetOpponentPlayerId() { return opponent_player_id; }
    std::mutex& GetMutex() { return mutex; }
    bool GetMultiPlay() { return is_multi; }

    //-----�f�[�^������������-----//
    void ResetData();
private:
    std::unique_ptr<CommunicationSystem> communication_system;

    //�����̔ԍ���ۑ�
    int operation_private_id{ -1 };

    //�����ȊO�̃v���C���[�̔ԍ�
    std::vector<int> opponent_player_id;

    //TCP�p�̃|�[�g�ԍ�
    char tcp_port[8];

    //�z�X�g��ID
    int host_id{ -1 };
public:
    //UDP�p�̃|�[�g�ԍ�
    char udp_port[8];

    //-----���O-----//
    char my_name[8];

    static constexpr int NAME_LENGTH = 8;

    //-----�v���C���[�̐F-----//
    int my_player_color{ 0 };

    //-----�����ȊO�̖��O-----//
    std::string names[MAX_CLIENT];

    //-----�����ȊO�̐F-----//
    int player_colors[MAX_CLIENT];

private:
    std::mutex mutex;

    //-----�}���`�v���C���ǂ���-----//
    bool is_multi{ false };

    //-----�z�X�g���ǂ���-----//
    bool is_host{ false };
public:
    bool GetHost() { return is_host; }
    void SetHost(bool arg) { is_host = arg; }
private:
    //-----�ڑ��Ґ�(�������܂߂�)-----//
    int connected_persons{ 0 };
public:
    //-----�ڑ��Ґ�����-----//
    void AddConnectedPersons() { connected_persons++; }
    //------�ڑ��Ґ�����-----//
    void SubConnectedPersons() { connected_persons--; }

    int GetConnectedPersons() { return connected_persons; }

    void ClearConnectedPersons() { connected_persons = 0; }

    //<Web�T�[�o�[����擾�����f�[�^�̃Q�b�^�[>//
    std::vector<WebEnemy::WebEnemyParamPack> GetWebEnemyParamPack() { return communication_system->GetWebEnemyParamPack(); }
};