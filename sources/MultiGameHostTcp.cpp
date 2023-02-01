#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "SceneMultiGameHost.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"

void SceneMultiGameHost::ReceiveTcpData()
{
    CoInitializeEx(NULL,NULL);
    DebugConsole::Instance().WriteDebugConsole("TCPスレッド開始");
    for (;;)
    {
        if (end_tcp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("TCPスレッドを終了");
            break;
        }
        char data[256]{};
        int size = sizeof(data);
        //-----通信相手との接続を確立する(ゲームが開始していないときだけ)-----//
        if(is_start_game == false)CorrespondenceManager::Instance().TcpAccept();

        //-----データを受信する-----//
        int client_id = CorrespondenceManager::Instance().TcpHostReceive(data, size);

		//-----データを受信したらログイン情報を作成してそれを送信-----//
		if (client_id >= 0)
		{
			switch (data[0])
			{
			case CommandList::Login:
				//-----ログイン処理-----//
				Login(client_id, data);
				break;
			case CommandList::Logout:
				//-----ログアウト処理-----//
				Logout(data);
				break;
				//-----他のクライアントが送信したデータ-----//
			case CommandList::SelectNextStage:
			{
				std::lock_guard<std::mutex> lock(mutex);
				//-----データを設定-----//
				client_select_stage.emplace_back(static_cast<WaveManager::STAGE_IDENTIFIER>(data[1]));

				break;
			}
			case CommandList::TransferEnemyControlRequest:
			{
				std::lock_guard<std::mutex> lock(mutex);
				//-----既にtrueなら他の接続者が今リクエストしているから中止させる-----//
				if (transfer_enemy_host_request)
				{
					char data[2]{};

					data[ComLocation::ComList] = CommandList::TransferEnemyControlResult;
					data[TransferEnemyControl::DataArray::Result] = TransferEnemyControl::Result::Prohibition;

					CorrespondenceManager::Instance().TcpSend(data, sizeof(data));
				}

				DebugConsole::Instance().WriteDebugConsole("リクエストを受けました", TextColor::Green);
				transfer_enemy_host_request = true;
				transfer_enemy_request_id = client_id;
				break;
			}
			case CommandList::ReturnEnemyControl:
			{
				std::lock_guard<std::mutex> lock(mutex);
				//-----帰ってきたことを知らせる-----//
				return_enemy_control = true;
				break;
			}
			case CommandList::SelectTryingAgain:
			{
				std::lock_guard<std::mutex> lock(mutex);
				select_trying_again.emplace_back(1);
				break;
			}
			//-----イベントが終了した時-----//
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
	//----------既に登録されている場合は止める----------//
	if (CorrespondenceManager::Instance().GetOpponentPlayerId().at(client_id) != -1) return;

	//-------------------------------排他制御をする----------------------------//
	//------mutexをロック-------//
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);

	SocketCommunicationManager& instance = SocketCommunicationManager::Instance();

	//-----受信データ-----//
	SendHostLoginData* receive_data = (SendHostLoginData*)data;

	LoginData login;
	login.cmd[0] = CommandList::Login;

	//-------通信相手の番号を保存---------//
	login.cmd[static_cast<int>(LoginDataCmd::OperationPrivateId)] = client_id;

	//----------自分の番号を保存する----------//
	login.cmd[static_cast<int>(LoginDataCmd::HostId)] = CorrespondenceManager::Instance().GetOperationPrivateId();


	//------------通信相手以外のプレイヤーの番号を保存----------------//
	std::string txt{};
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----今までに接続して来たプレイヤーの番号を配列に保存-----//
		login.opponent_player_id[i] = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		txt = std::to_string(i) + "番目に" + std::to_string(CorrespondenceManager::Instance().GetOpponentPlayerId().at(i)) + "を設定";
		DebugConsole::Instance().WriteDebugConsole(txt, TextColor::Green);

		//-----名前設定-----//
		//login.name[i] = CorrespondenceManager::Instance().names[i];

		std::memcpy(login.name[i], CorrespondenceManager::Instance().names[i].c_str(), sizeof(CorrespondenceManager::NAME_LENGTH));

		login.p_color[i] = CorrespondenceManager::Instance().player_colors[i];

		//Ipアドレスも保存
		login.game_udp_server_addr[i] = instance.game_udp_server_addr[i];
		std::string ip = std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b1) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b2) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b3) + "." + std::to_string(instance.game_udp_server_addr[i].sin_addr.S_un.S_un_b.s_b4);
		DebugConsole::Instance().WriteDebugConsole(ip, TextColor::Green);

	}
	sockaddr_in create;


	//-----受信したIPアドレスとポート番号で送信用アドレスを作成する-----//
	create = CorrespondenceManager::Instance().SetSendPortAddress(instance.game_udp_server_addr[client_id], receive_data->port);
	if (create.sin_addr.S_un.S_addr == 0)
	{
		DebugConsole::Instance().WriteDebugConsole("送信用アドレスが設定できていません", TextColor::Green);
	}
	instance.game_udp_server_addr[client_id] = create;
	//-----名前を保存-----//
	CorrespondenceManager::Instance().names[client_id] = receive_data->name;
	CorrespondenceManager::Instance().player_colors[client_id] = receive_data->cmd[static_cast<int>(SendHostLoginDataCmd::PlayerColor)];

	//-----------ホストの管理するIDの中に今接続して来たプレイヤーの番号を保存-------------//
	CorrespondenceManager::Instance().SetOpponentPlayerId(client_id);


	//-----------通信相手以外のプレイヤーに自分を入れる-------------//
	//基本的に自分の操作しているプレイヤー番号番目に値が入る
	login.opponent_player_id[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().GetOperationPrivateId();

	std::memcpy(login.name[CorrespondenceManager::Instance().GetOperationPrivateId()], CorrespondenceManager::Instance().my_name, sizeof(CorrespondenceManager::Instance().my_name));

	//login.name[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().my_name;
	login.p_color[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().my_player_color;

	//--------新しくログインして来た相手にデータを送信---------//
	CorrespondenceManager::Instance().TcpSend(client_id, (char*)&login, sizeof(LoginData));

	//-----今つながっているクライアントに新しくログインしてきた相手のデータを送る-----//


	//-----データを設定-----//
	SendClientLoginData client_send{};
	client_send.cmd[0] = CommandList::Login;
	client_send.cmd[static_cast<int>(SendClientLoginDataCmd::NewClientId)] = client_id;
	client_send.addr = create;
	client_send.name = receive_data->name;
	client_send.cmd[static_cast<int>(SendClientLoginDataCmd::PlayerColor)] = receive_data->cmd[static_cast<int>(SendHostLoginDataCmd::PlayerColor)];

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----今接続している相手のIDを取得-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----IDが0未満か今接続してきた者と同じならとばす-----//
		if (id < 0 || id == client_id) continue;

		//--------新しくログインして来た相手にデータを送信---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&client_send, sizeof(SendClientLoginData));

	}

	//-----プレイヤーの追加フラグとIDを設定-----//
	register_player = true;
	register_player_id = client_id;
	register_player_color = receive_data->cmd[static_cast<int>(SendHostLoginDataCmd::PlayerColor)];

	DebugConsole::Instance().WriteDebugConsole("プレイヤーがログインしてきました", TextColor::Green);

}

void SceneMultiGameHost::Logout(char* data)
{
	//-------------------------------排他制御をする----------------------------//
	//------mutexをロック-------//
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);

	DebugConsole::Instance().WriteDebugConsole("ホスト : ログアウトデータを受信",TextColor::Yellow);

	LogoutData* logout_data = (LogoutData*)data;

	//-----ログアウトするプレイヤーのIDを保存-----//
	logout_id.emplace_back(logout_data->id);

	//-----送るデータを作成-----//
	LogoutData d;
	d.cmd[0] = CommandList::Logout;
	d.id = logout_data->id;

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----今接続している相手のIDを取得-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----IDが0未満ならとばす-----//
		if (id < 0) continue;

		//--------ログアウトデータを送信---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&d, sizeof(LogoutData));

	}

}
