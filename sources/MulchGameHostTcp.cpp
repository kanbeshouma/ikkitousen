#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"SceneMulchGameHost.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"
void SceneMulchGameHost::ReceiveTcpData()
{
    CoInitializeEx(NULL,NULL);
    DebugConsole::Instance().WriteDebugConsole("���O�C���X���b�h�J�n");
    for (;;)
    {
        if (end_tcp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("���O�C���X���b�h���I��");
            break;
        }
        char data[256]{};
        int size = sizeof(data);
        //-----�ʐM����Ƃ̐ڑ����m������-----//
        CorrespondenceManager::Instance().TcpAccept();

        //-----�f�[�^����M����-----//
        int client_id = CorrespondenceManager::Instance().TcpHostReceive(data, size);

		//-----�f�[�^����M�����烍�O�C�������쐬���Ă���𑗐M-----//
		if (client_id >= 0)
		{
			switch (data[0])
			{
			case CommandList::Login:
				//-----���O�C������-----//
				Login(client_id, data);
				break;
			case CommandList::Logout:
				//-----���O�A�E�g����-----//
				Logout(data);
				break;
			default:
				break;
			}

		}
	}

    CoUninitialize();
}

void SceneMulchGameHost::Login(int client_id, char* data)
{
	//----------���ɓo�^����Ă���ꍇ�͎~�߂�----------//
	if (CorrespondenceManager::Instance().GetOpponentPlayerId().at(client_id) != -1) return;

	//-------------------------------�r�����������----------------------------//
	//------mutex�����b�N-------//
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);

	SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

	//-----��M�f�[�^-----//
	SendHostLoginData* receive_data = (SendHostLoginData*)data;

	LoginData login;
	login.cmd[0] = CommandList::Login;

	//-------�ʐM����̔ԍ���ۑ�---------//
	login.operation_private_id = client_id;

	//----------�����̔ԍ���ۑ�����----------//
	login.host_id = CorrespondenceManager::Instance().GetOperationPrivateId();


	//------------�ʐM����ȊO�̃v���C���[�̔ԍ���ۑ�----------------//
	std::string txt{};
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----���܂łɐڑ����ė����v���C���[�̔ԍ���z��ɕۑ�-----//
		login.opponent_player_id[i] = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		txt = std::to_string(i) + "�Ԗڂ�" + std::to_string(CorrespondenceManager::Instance().GetOpponentPlayerId().at(i)) + "��ݒ�";
		DebugConsole::Instance().WriteDebugConsole(txt, TextColor::Green);

		//Ip�A�h���X���ۑ�
		login.game_udp_server_addr[i] = instance.game_udp_server_addr[i];
		std::string ip = std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4);
		DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);

	}
	sockaddr_in create;


	//-----��M����IP�A�h���X�ƃ|�[�g�ԍ��ő��M�p�A�h���X���쐬����-----//
	create = CorrespondenceManager::Instance().SetSendPortAddress(instance.game_udp_server_addr[client_id], receive_data->port);
	if (create.sin_addr.S_un.S_addr == 0)
	{
		DebugConsole::Instance().WriteDebugConsole("���M�p�A�h���X���ݒ�ł��Ă��܂���", TextColor::Green);
	}
	instance.game_udp_server_addr[client_id] = create;

	//-----------�z�X�g�̊Ǘ�����ID�̒��ɍ��ڑ����ė����v���C���[�̔ԍ���ۑ�-------------//
	CorrespondenceManager::Instance().SetOpponentPlayerId(client_id);


	//-----------�ʐM����ȊO�̃v���C���[�Ɏ���������-------------//
	//��{�I�Ɏ����̑��삵�Ă���v���C���[�ԍ��Ԗڂɒl������
	login.opponent_player_id[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().GetOperationPrivateId();


	//--------�V�������O�C�����ė�������Ƀf�[�^�𑗐M---------//
	CorrespondenceManager::Instance().TcpSend(client_id, (char*)&login, sizeof(LoginData));

	//-----���Ȃ����Ă���N���C�A���g�ɐV�������O�C�����Ă�������̃f�[�^�𑗂�-----//

	//-----�f�[�^��ݒ�-----//
	SendClientLoginData client_send{};
	client_send.cmd[0] = CommandList::Login;
	client_send.new_client_id = client_id;
	client_send.addr = create;

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----���ڑ����Ă��鑊���ID���擾-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----ID��0���������ڑ����Ă����҂Ɠ����Ȃ�Ƃ΂�-----//
		if (id < 0 || id == client_id) continue;

		//--------�V�������O�C�����ė�������Ƀf�[�^�𑗐M---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&client_send, sizeof(LoginData));

	}

	//-----�v���C���[�̒ǉ��t���O��ID��ݒ�-----//
	register_player = true;
	register_player_id = client_id;


	DebugConsole::Instance().WriteDebugConsole("�v���C���[�����O�C�����Ă��܂���", TextColor::Green);

}

void SceneMulchGameHost::Logout(char* data)
{
	//-------------------------------�r�����������----------------------------//
	//------mutex�����b�N-------//
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);

	LogoutData* logout_data = (LogoutData*)data;

	//-----���O�A�E�g����v���C���[��ID��ۑ�-----//
	logout_id.emplace_back(logout_data->id);

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----���ڑ����Ă��鑊���ID���擾-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----ID��0���������ڑ����Ă����҂Ɠ����Ȃ�Ƃ΂�-----//
		if (id < 0 || id == logout_data->id) continue;

		//--------�V�������O�C�����ė�������Ƀf�[�^�𑗐M---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&data, sizeof(LogoutData));

	}

}
