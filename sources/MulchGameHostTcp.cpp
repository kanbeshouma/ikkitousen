#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"SceneMulchGameHost.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"
void SceneMulchGameHost::ReceiveLoginData()
{
    CoInitializeEx(NULL,NULL);
    DebugConsole::Instance().WriteDebugConsole("マルチスレッド開始");
    for (;;)
    {
        if (end_login_thread)
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
			//-----ログイン処理-----//
			Login(client_id, data);
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
	CorrespondenceManager::Instance().GetOpponentPlayerId().at(client_id) = client_id;

	//-----------通信相手以外のプレイヤーに自分を入れる-------------//
	//基本的に自分の操作しているプレイヤー番号番目に値が入る
	login.opponent_player_id[CorrespondenceManager::Instance().GetOperationPrivateId()] = CorrespondenceManager::Instance().GetOperationPrivateId();


	//--------新しくログインして来た相手にデータを送信---------//
	CorrespondenceManager::Instance().TcpSend(client_id, (char*)&login, sizeof(LoginData));

	//----------今現在までにログインしてきている相手にデータを送信UDPで----------//

	//-----プレイヤーの追加フラグとIDを設定-----//
	register_player = true;
	register_player_id = client_id;


	DebugConsole::Instance().WriteDebugConsole("プレイヤーがログインしてきました", TextColor::Green);

}
