#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include "SceneMultiGameHost.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"

void SceneMultiGameHost::ReceiveTcpData()
{
    CoInitializeEx(NULL,NULL);
    DebugConsole::Instance().WriteDebugConsole("TCP�X���b�h�J�n");
    for (;;)
    {
        if (end_tcp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("TCP�X���b�h���I��");
            break;
        }
        char data[256]{};
        int size = sizeof(data);
        //-----�ʐM����Ƃ̐ڑ����m������(�Q�[�����J�n���Ă��Ȃ��Ƃ�����)-----//
        if(is_start_game == false)CorrespondenceManager::Instance().TcpAccept();

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
				//-----���̃N���C�A���g�����M�����f�[�^-----//
			case CommandList::SelectNextStage:
			{
				std::lock_guard<std::mutex> lock(mutex);
				//-----�f�[�^��ݒ�-----//
				client_select_stage.emplace_back(static_cast<WaveManager::STAGE_IDENTIFIER>(data[1]));

				break;
			}
			case CommandList::TransferEnemyControlRequest:
			{
				std::lock_guard<std::mutex> lock(mutex);
				//-----����true�Ȃ瑼�̐ڑ��҂������N�G�X�g���Ă��邩�璆�~������-----//
				if (transfer_enemy_host_request)
				{
					char data[2]{};

					data[ComLocation::ComList] = CommandList::TransferEnemyControlResult;
					data[TransferEnemyControl::DataArray::Result] = TransferEnemyControl::Result::Prohibition;

					CorrespondenceManager::Instance().TcpSend(data, sizeof(data));
				}

				DebugConsole::Instance().WriteDebugConsole("���N�G�X�g���󂯂܂���", TextColor::Green);
				transfer_enemy_host_request = true;
				transfer_enemy_request_id = client_id;
				break;
			}
			case CommandList::ReturnEnemyControl:
			{
				std::lock_guard<std::mutex> lock(mutex);
				//-----�A���Ă������Ƃ�m�点��-----//
				return_enemy_control = true;
				break;
			}
			case CommandList::SelectTryingAgain:
			{
				std::lock_guard<std::mutex> lock(mutex);
				select_trying_again.emplace_back(1);
				break;
			}
			//-----�C�x���g���I��������-----//
			case CommandList::WatchEndEvent:
			{
				std::lock_guard<std::mutex> lock(mutex);
				end_event.emplace_back(1);
				break;
			}
			default:
				break;
			}

		}
	}

    CoUninitialize();
}

void SceneMultiGameHost::Login(int client_id, char* data)
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
	login.cmd[static_cast<int>(LoginDataCmd::OperationPrivateId)] = client_id;

	//----------�����̔ԍ���ۑ�����----------//
	login.cmd[static_cast<int>(LoginDataCmd::HostId)] = CorrespondenceManager::Instance().GetOperationPrivateId();


	//------------�ʐM����ȊO�̃v���C���[�̔ԍ���ۑ�----------------//
	std::string txt{};
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----���܂łɐڑ����ė����v���C���[�̔ԍ���z��ɕۑ�-----//
		login.opponent_player_id[i] = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		txt = std::to_string(i) + "�Ԗڂ�" + std::to_string(CorrespondenceManager::Instance().GetOpponentPlayerId().at(i)) + "��ݒ�";
		DebugConsole::Instance().WriteDebugConsole(txt, TextColor::Green);

		//-----���O�ݒ�-----//
		//login.name[i] = CorrespondenceManager::Instance().names[i];

		std::memcpy(login.name[i], CorrespondenceManager::Instance().names[i].c_str(), sizeof(CorrespondenceManager::NAME_LENGTH));

		login.p_color[i] = CorrespondenceManager::Instance().player_colors[i];

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
	//-----���O��ۑ�-----//
	CorrespondenceManager::Instance().names[client_id] = receive_data->name;
	CorrespondenceManager::Instance().player_colors[client_id] = receive_data->cmd[static_cast<int>(SendHostLoginDataCmd::PlayerColor)];

	//-----------�z�X�g�̊Ǘ�����ID�̒��ɍ��ڑ����ė����v���C���[�̔ԍ���ۑ�-------------//
	CorrespondenceManager::Instance().SetOpponentPlayerId(client_id);


	//-----------�ʐM����ȊO�̃v���C���[�Ɏ���������-------------//
	//��{�I�Ɏ����̑��삵�Ă���v���C���[�ԍ��Ԗڂɒl������
	login.opponent_player_id[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().GetOperationPrivateId();

	std::memcpy(login.name[CorrespondenceManager::Instance().GetOperationPrivateId()], CorrespondenceManager::Instance().my_name, sizeof(CorrespondenceManager::Instance().my_name));

	//login.name[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().my_name;
	login.p_color[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().my_player_color;

	//--------�V�������O�C�����ė�������Ƀf�[�^�𑗐M---------//
	CorrespondenceManager::Instance().TcpSend(client_id, (char*)&login, sizeof(LoginData));

	//-----���Ȃ����Ă���N���C�A���g�ɐV�������O�C�����Ă�������̃f�[�^�𑗂�-----//


	//-----�f�[�^��ݒ�-----//
	SendClientLoginData client_send{};
	client_send.cmd[0] = CommandList::Login;
	client_send.cmd[static_cast<int>(SendClientLoginDataCmd::NewClientId)] = client_id;
	client_send.addr = create;
	client_send.name = receive_data->name;
	client_send.cmd[static_cast<int>(SendClientLoginDataCmd::PlayerColor)] = receive_data->cmd[static_cast<int>(SendHostLoginDataCmd::PlayerColor)];

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----���ڑ����Ă��鑊���ID���擾-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----ID��0���������ڑ����Ă����҂Ɠ����Ȃ�Ƃ΂�-----//
		if (id < 0 || id == client_id) continue;

		//--------�V�������O�C�����ė�������Ƀf�[�^�𑗐M---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&client_send, sizeof(SendClientLoginData));

	}

	//-----�v���C���[�̒ǉ��t���O��ID��ݒ�-----//
	register_player = true;
	register_player_id = client_id;
	register_player_color = receive_data->cmd[static_cast<int>(SendHostLoginDataCmd::PlayerColor)];

	DebugConsole::Instance().WriteDebugConsole("�v���C���[�����O�C�����Ă��܂���", TextColor::Green);

}

void SceneMultiGameHost::Logout(char* data)
{
	//-------------------------------�r�����������----------------------------//
	//------mutex�����b�N-------//
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);

	DebugConsole::Instance().WriteDebugConsole("�z�X�g : ���O�A�E�g�f�[�^����M",TextColor::Yellow);

	LogoutData* logout_data = (LogoutData*)data;

	//-----���O�A�E�g����v���C���[��ID��ۑ�-----//
	logout_id.emplace_back(logout_data->id);

	//-----����f�[�^���쐬-----//
	LogoutData d;
	d.cmd[0] = CommandList::Logout;
	d.id = logout_data->id;

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----���ڑ����Ă��鑊���ID���擾-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----ID��0�����Ȃ�Ƃ΂�-----//
		if (id < 0) continue;

		//--------���O�A�E�g�f�[�^�𑗐M---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&d, sizeof(LogoutData));

	}

}
