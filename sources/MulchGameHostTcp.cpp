#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"SceneMulchGameHost.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"
void SceneMulchGameHost::ReceiveTcpData()
{
    CoInitializeEx(NULL,NULL);
    DebugConsole::Instance().WriteDebugConsole("ログインスレッド開始");
    for (;;)
    {
        if (end_tcp_thread)
        {
            DebugConsole::Instance().WriteDebugConsole("ログインスレッドを終了");
            break;
        }
        char data[256]{};
        int size = sizeof(data);
        //-----通信相手との接続を確立する-----//
        CorrespondenceManager::Instance().TcpAccept();

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
			default:
				break;
			}

		}
	}

    CoUninitialize();
}

void SceneMulchGameHost::Login(int client_id, char* data)
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
	login.operation_private_id = client_id;

	//----------自分の番号を保存する----------//
	login.host_id = CorrespondenceManager::Instance().GetOperationPrivateId();


	//------------通信相手以外のプレイヤーの番号を保存----------------//
	std::string txt{};
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----今までに接続して来たプレイヤーの番号を配列に保存-----//
		login.opponent_player_id[i] = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		txt = std::to_string(i) + "番目に" + std::to_string(CorrespondenceManager::Instance().GetOpponentPlayerId().at(i)) + "を設定";
		DebugConsole::Instance().WriteDebugConsole(txt, TextColor::Green);

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

	//-----------ホストの管理するIDの中に今接続して来たプレイヤーの番号を保存-------------//
	CorrespondenceManager::Instance().SetOpponentPlayerId(client_id);


	//-----------通信相手以外のプレイヤーに自分を入れる-------------//
	//基本的に自分の操作しているプレイヤー番号番目に値が入る
	login.opponent_player_id[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().GetOperationPrivateId();


	//--------新しくログインして来た相手にデータを送信---------//
	CorrespondenceManager::Instance().TcpSend(client_id, (char*)&login, sizeof(LoginData));

	//-----今つながっているクライアントに新しくログインしてきた相手のデータを送る-----//

	//-----データを設定-----//
	SendClientLoginData client_send{};
	client_send.cmd[0] = CommandList::Login;
	client_send.new_client_id = client_id;
	client_send.addr = create;

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----今接続している相手のIDを取得-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----IDが0未満か今接続してきた者と同じならとばす-----//
		if (id < 0 || id == client_id) continue;

		//--------新しくログインして来た相手にデータを送信---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&client_send, sizeof(LoginData));

	}

	//-----プレイヤーの追加フラグとIDを設定-----//
	register_player = true;
	register_player_id = client_id;


	DebugConsole::Instance().WriteDebugConsole("プレイヤーがログインしてきました", TextColor::Green);

}

void SceneMulchGameHost::Logout(char* data)
{
	//-------------------------------排他制御をする----------------------------//
	//------mutexをロック-------//
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);

	LogoutData* logout_data = (LogoutData*)data;

	//-----ログアウトするプレイヤーのIDを保存-----//
	logout_id.emplace_back(logout_data->id);

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		//-----今接続している相手のIDを取得-----//
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);

		//-----IDが0未満か今接続してきた者と同じならとばす-----//
		if (id < 0 || id == logout_data->id) continue;

		//--------新しくログインして来た相手にデータを送信---------//
		CorrespondenceManager::Instance().TcpSend(id, (char*)&data, sizeof(LogoutData));

	}

}
