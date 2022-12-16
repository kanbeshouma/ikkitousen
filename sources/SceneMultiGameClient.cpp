#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "SceneMultiGameClient.h"
#include "scene_title.h"
#include "scene_loading.h"
#include "scene_manager.h"

#include "framework.h"
#include "graphics_pipeline.h"

#include "collision.h"
#include "shader.h"
#include "texture.h"

#include "user.h"
#include "volume_icon.h"
#include "LastBoss.h"
#include"ClientPlayer.h"
#include"Correspondence.h"
#include"NetWorkInformationStucture.h"


//-----TCPを終了するかのフラグ-----//
bool SceneMultiGameClient::end_tcp_thread = false;

//-----UDPを終了するかのフラグ-----//
bool SceneMultiGameClient::end_udp_thread = false;

//-----プレイヤーが登録されたかどうか-----//
bool SceneMultiGameClient::register_player = false;

//-----追加されたプレイヤーの番号-----//
int SceneMultiGameClient::register_player_id = -1;

//-----追加されたプレイヤーの色-----//
int SceneMultiGameClient::register_player_color = 0;

//-----ログアウトするプレイヤーの番号-----//
std::vector<int> SceneMultiGameClient::logout_id = {};

//-----ブロッキング-----//
std::mutex SceneMultiGameClient::mutex;

//-----プレイヤーのデータを入れる-----//
PlayerAllDataStruct SceneMultiGameClient::receive_all_player_data;

//-----敵のデータを入れる-----//
EnemyAllDataStruct SceneMultiGameClient::receive_all_enemy_data;

//-----敵のホスト権の譲渡されたかどうか-----//
bool SceneMultiGameClient::transfer_enemy_host_result = false;

//-----敵のホスト権の譲渡リクエスト-----//
bool SceneMultiGameClient::transfer_enemy_host_request = false;

//-----敵のホスト権の譲渡結果-----//
TransferEnemyControl::TransferEnemyResult SceneMultiGameClient::transfer_enemy_result;

//-----ステージの状況-----//
StageSituation SceneMultiGameClient::stage_situation = StageSituation::NoneFlg;

//-----今のステージ-----//
WaveManager::STAGE_IDENTIFIER SceneMultiGameClient::current_stage = WaveManager::STAGE_IDENTIFIER::S_1_1;

//-----ステージ番号を受信したかどうか-----//
bool SceneMultiGameClient::receive_stage_num = false;

//-----ステージ選択の最終決定データを受信したかどうか-----//
bool SceneMultiGameClient::receive_end_result_next_stage = false;

//-----最終決定したステージ-----//
WaveManager::STAGE_IDENTIFIER SceneMultiGameClient::result_next_stage = WaveManager::STAGE_IDENTIFIER::S_1_1;

//-----ゲームリトライ-----//
bool SceneMultiGameClient::game_retry = false;

//-----イベントが終了したかどうか-----//
bool SceneMultiGameClient::end_event_data = false;

//-----チェイン攻撃の時の敵の番号データ-----//
std::map<int, std::vector<char>> SceneMultiGameClient::chain_rock_on_enemy_id;

SceneMultiGameClient::SceneMultiGameClient()
{
	//<マルチスレッドで使用する変数を初期化>//
	end_tcp_thread = false;
	end_udp_thread = false;
	register_player = false;
	register_player_id = -1;
	register_player_color = 0;
	logout_id.clear();
	ResetPlayerAllData();
	ResetEnemyAllData();
	transfer_enemy_host_result = false;
	transfer_enemy_host_request = false;
	transfer_enemy_result.data[0] = transfer_enemy_result.data[1] = TransferEnemyControl::Result::None;
	stage_situation = StageSituation::NoneFlg;
	current_stage = WaveManager::STAGE_IDENTIFIER::S_1_1;
	receive_stage_num = false;
	receive_end_result_next_stage = false;
	result_next_stage = WaveManager::STAGE_IDENTIFIER::S_1_1;
	game_retry = false;
	end_event_data = false;
	chain_rock_on_enemy_id.clear();
}

SceneMultiGameClient::~SceneMultiGameClient()
{
	//-----TCPスレッドを終了する-----//
	tcp_thread.join();

	end_udp_thread = true;

	//-----UDPスレッドを終了する-----//
	udp_thread.join();
}

void SceneMultiGameClient::initialize(GraphicsPipeline& graphics)
{
	// shadow_map
	shadow_map = std::make_unique<ShadowMap>(graphics);
	// post effect
	post_effect = std::make_unique<PostEffect>(graphics.get_device().Get());
	// bloom
	bloom_effect = std::make_unique<Bloom>(graphics.get_device().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
	bloom_constants = std::make_unique<Constants<BloomConstants>>(graphics.get_device().Get());
	// モデルのロード
	sky_dome = std::make_unique<SkyDome>(graphics);

	//--------------------<弾の管理クラスを初期化>--------------------//
	BulletManager& mBulletManager = BulletManager::Instance();
	mBulletManager.fInitialize();
	//--------------------<敵の管理クラスを初期化>--------------------//
	mWaveManager.fInitialize(graphics, mBulletManager.fGetAddFunction());
	//-----ホストかどうかを設定-----//
	mWaveManager.SetHost(false);

	//-----ステージ番号を初期化する-----//
	mWaveManager.SetCurrentStage(current_stage);
	mWaveManager.SetCurrentWaveNum(current_stage);

	player_manager = std::make_unique<PlayerManager>();
	//-----プレイヤーを登録-----//
	Player* player = new Player(graphics, CorrespondenceManager::Instance().GetOperationPrivateId());
	player->SetName(CorrespondenceManager::Instance().my_name);
	player->SetColor(static_cast<BasePlayer::PlayerColor> (CorrespondenceManager::Instance().my_player_color));
	player_manager->RegisterPlayer(player);
	player_manager->SetPrivateObjectId(CorrespondenceManager::Instance().GetOperationPrivateId());
	CorrespondenceManager::Instance().AddConnectedPersons();
	for(int i = 0; i < MAX_CLIENT; i++)
	{
		int id = CorrespondenceManager::Instance().GetOpponentPlayerId().at(i);
		if (id < 0 || i == player_manager->GetPrivatePlayerId()) continue;
		ClientPlayer* p = new ClientPlayer(graphics, id);
		p->SetName(CorrespondenceManager::Instance().names[i]);
		p->SetColor(static_cast<BasePlayer::PlayerColor> (CorrespondenceManager::Instance().player_colors[i]));
		player_manager->RegisterPlayer(p);
		//-----プレイヤーの体力を増やす-----//
		player_manager->AddPlayerMultiHealth();
		//-----接続者数を増やす-----//
		CorrespondenceManager::Instance().AddConnectedPersons();

	}

	//-----TCP用のマルチスレッドを立ち上げる-----//
	{
		end_tcp_thread = false;
		std::thread t(ReceiveTcpData);
		t.swap(tcp_thread);
	}

	//-----UDP用のマルチスレッドを立ち上げる-----//
	{
		end_udp_thread = false;
		std::thread t(ReceiveUdpData);
		t.swap(udp_thread);
	}

	CorrespondenceManager::Instance().SetHost(false);
	CorrespondenceManager::Instance().SetMultiPlay(true);

	// カメラ
	cameraManager = std::make_unique<CameraManager>();
	cameraManager->RegisterCamera(new GameCamera(player));
	cameraManager->RegisterCamera(new ClearCamera(player));
	cameraManager->RegisterCamera(new JointCamera(graphics));
	cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));

	//-----ボスの演出-----//
	last_boss_mode = LastBoss::Mode::None;
	old_last_boss_mode = LastBoss::Mode::None;

	//-----オーディオ-----//
	audio_manager->stop_all_bgm();
	audio_manager->play_bgm(BGM_INDEX::GAME);

	//-----スカイドーム-----//
	purple_threshold = 0; red_threshold = 0;
	sky_dome->set_purple_threshold(0); sky_dome->set_red_threshold(0);

	// enemy_hp_gauge
	enemy_hp_gauge = std::make_unique<EnemyHpGauge>(graphics);

	// boss_hp_gauge
	boss_hp_gauge = std::make_unique<BossHpGauge>(graphics);

	// reticle
	reticle = std::make_unique<Reticle>(graphics);

	// wave
	wave = std::make_unique<Counter>(graphics, L".\\resources\\Sprites\\ui\\wave.png");

	// option
	option = std::make_unique<Option>(graphics);

	// tunnel
	tunnel = std::make_unique<Tunnel>(graphics.get_device().Get());

	//mini_map
	minimap = std::make_unique<MiniMap>(graphics);


	sprite_selecter = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\selecter.png", 2);
	selecter1.position = { 328.0f,267.3f };
	selecter1.scale = { 0.2f,0.2f };
	selecter1.texsize = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
	selecter2.position = { 637.1f,267.3f };
	selecter2.scale = { 0.2f,0.2f };
	selecter2.texsize = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };

	sprite_back = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\title_back.png", 1);
	game_over_pram.texsize = { static_cast<float>(sprite_back->get_texture2d_desc().Width),
										static_cast<float>(sprite_back->get_texture2d_desc().Height) };
	game_over_pram.position = { 206.5f,44.0f };
	game_over_pram.scale = { 1.46f ,0.59f };
	brack_back = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\mask\\black_mask.png", 1);
	brack_back_pram.texsize = { static_cast<float>(brack_back->get_texture2d_desc().Width),
										static_cast<float>(brack_back->get_texture2d_desc().Height) };
	brack_back_pram.color = { 1.0f,1.0f,1.0f,0.0f };

	game_over_sprite = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\gameover.png", 1);
	game_over_sprite_pram.position = { 415.9f,0.0f };
	game_over_sprite_pram.texsize = { 512.0f,512.0f };
	//font
	game_over_text.s = L"ゲームオーバー";
	game_over_text.position = { 553.1f,124.3f };
	game_over_text.scale = { 1.0f,1.0f };
	back_title.s = L"タイトルに戻る";
	again.s = L"再挑戦";
	again.position = { 751.6f,236.7f };
	again.scale = { 1.0f,1.0f };
	game_clear_text.s = L"ゲームクリア";
	game_clear_text.position = { 552.0f,127.0f };

	game_over_idle = L"他の接続者の選択を待っています・・・";
	game_over_select_idle.position = { 325.7f,212.9f };
	game_over_select_idle.scale = { 1.0f,1.0f };

}

void SceneMultiGameClient::RestartInitialize()
{
	std::lock_guard<std::mutex> lock(mutex);

	//-----ゲームオーバーの時に使っていたフラグを初期化-----//
	is_game_over = false;
	game_over_trying_again = false;
	is_set_black = false;
	is_game_over_sprite = false;
	game_retry = false;
	brack_back_pram.color.w = 0.0f;

	//-----ゲームの初期化-----//
	player_manager->RestartInitialize();

	mWaveManager.RestartInitialize();

	BulletManager::Instance().RestartInitialize();
}

void SceneMultiGameClient::uninitialize()
{
	audio_manager->stop_all_se();
	audio_manager->stop_all_bgm();


	mWaveManager.fFinalize();
	BulletManager::Instance().fFinalize();

	LogoutData data;
	data.cmd[0] = CommandList::Logout;
	data.id = player_manager->GetPrivatePlayerId();

	//-----ログアウトデータをホストに送信-----//
	CorrespondenceManager::Instance().TcpSend((char*)&data, sizeof(LogoutData));

}

void SceneMultiGameClient::effect_liberation(GraphicsPipeline& graphics)
{
	effect_manager->finalize();
	effect_manager->initialize(graphics);
}

void SceneMultiGameClient::update(GraphicsPipeline& graphics, float elapsed_time)
{
	//-----ステージ番号を受信した時-----//
	if (receive_stage_num)
	{
		mWaveManager.SetCurrentStage(current_stage);
		mWaveManager.SetCurrentWaveNum(current_stage);
		receive_stage_num = false;
	}

	//-----ホストがタイトルに戻るを押した時-----//
	//強制的にタイトルに戻る//
	if (stage_situation & StageSituation::GameOverFlg)
	{
		SceneManager::scene_switching(new SceneLoading(new SceneTitle()), DISSOLVE_TYPE::TYPE1, 2.0f);
	}
	//-----ゲームクリア、ゲームオーバーでもないとき-----//
	if (is_game_clear == false && is_game_over == false)
	{
		brack_back_pram.color.w = 0;
		is_set_black = false;
	}

	//-----ゲームクリアになったら-----//
	GameClearAct(elapsed_time, graphics);
	//-----ゲームオーバーになったら-----//
	GameOverAct(elapsed_time);

	//-----オーディオ-----//
	const float bgm_volume = 2.0f;
	const float se_volume = 2.0f;
	audio_manager->set_all_volume_bgm(bgm_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_bgm_volume());
	audio_manager->set_all_volume_se(se_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_se_volume());

#if 1
#ifdef USE_IMGUI
	ImGui::Begin("StageNum");

	int a =  mWaveManager.get_current_stage();
	ImGui::Text("stage_num%d", a);
	bool cl = mWaveManager.GetClearFlg();
	ImGui::RadioButton("clear",cl);

	ImGui::End();
#endif // USE_IMGUI

#endif // 0
	//-----ゲームオーバー,ゲームクリアの時は止める-----//
	if (is_game_over) return;
	if (is_game_clear) return;



	//-----	ステージかゲームをクリア-----//
	if (stage_situation & StageSituation::StageClearFlg)
	{
		mWaveManager.SetClearFlg(true);
		//-----フラグを初期化-----//
		stage_situation = StageSituation::NoneFlg;
	}
	//-----ゲームクリア-----//
	if (mWaveManager.get_game_clear()) { is_game_clear = true; }



	//-----option------//
	if (option->get_validity())
	{
		option->update(graphics, elapsed_time);
	}
	else
	{
		//-----オプションボタンを押したら-----//
		if (game_pad->get_button_down() & GamePad::BTN_START)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			option->initialize();
			option->set_validity(true);
		}
	}
	player_manager->SetOption(option->get_validity());

	//-----弾のインスタンスを生成-----//
	BulletManager& mBulletManager = BulletManager::Instance();

	{
		//-----ロックする-----//
		std::lock_guard<std::mutex> lock(mutex);

		//-----ステージを設定-----//
		if (receive_end_result_next_stage)
		{
			mWaveManager.SetEndResultNextStage(result_next_stage);
			receive_end_result_next_stage = false;
		}

		//-----ボスのイベントが終わったかどうか-----//
		if (end_event_data)
		{
			mWaveManager.fGetEnemyManager()->EndEvent();
			end_event_data = false;
		}

		//-----ステージ中のウェーブの更新処理-----//
		mWaveManager.fMultiPlayUpdate(graphics, elapsed_time, mBulletManager.fGetAddFunction(), receive_all_enemy_data);

		//-----敵のデータを削除-----//
		ClearEnemyReceiveData();
	}
	//-----クリア演出-----//
	if (mWaveManager.during_clear_performance())
	{
		//-----トンネルのアルファ値を増やす-----//
		tunnel_alpha += elapsed_time * 0.5f;
		tunnel_alpha = (std::min)(tunnel_alpha, 1.0f);

		if (!during_clear)
		{
			//-----カメラをトンネル用に切り替える-----//
			cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Tunnel));

			//-----プレイヤーを移動モードに変える-----//
			player_manager->TransitionStageMove();
			during_clear = true;
		}
	}
	else
	{
		if (during_clear)
		{
			//-----トンネルを薄くしていく-----//
			tunnel_alpha -= elapsed_time;
			tunnel_alpha = (std::max)(tunnel_alpha, 0.0f);

			//-----トンネルのアルファ値がほぼ0なら-----//
			if (Math::equal_check(tunnel_alpha, 0.0f, 0.01f))
			{
				//-----カメラをゲーム用に変更-----//
				cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));

				//-----プレイヤーをゲーム用に切り替える-----//
				player_manager->TransitionStageMoveEnd();
				tunnel_alpha = 0.0f;
				during_clear = false;
			}
		}
	}

	//-----受信データを設定する-----//
	SetReceiveData();

	//-----	スロウ判定-----//
	JudgeSlow(elapsed_time);

	//--------------------<敵の管理クラスの更新処理>--------------------//
	old_last_boss_mode = last_boss_mode;

	//-----敵のインスタンスを生成-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	// camera
	cameraManager->Update(elapsed_time);


	//-----プレイヤーの位置を設定-----//
	//enemyManager->fSetPlayerPosition(player_manager->GetPosition());
	enemyManager->fSetPlayerIdPosition(player_manager->GetPlayerVector());

	//-----プレイヤーのチェイン攻撃のロックオン中かどうかを設定-----//
	enemyManager->fSetPlayerSearch(player_manager->DuringSearchTime());

	//-----弾の更新処理-----//
	mBulletManager.fUpdate(elapsed_time);

	//-----↓↓↓↓↓↓↓↓↓プレイヤーの更新はこのした↓↓↓↓↓-----//


	//-----プレイヤーが死んだらゲームオーバー-----//
	if (player_manager->GetIsAlive() == false)	is_game_over = true;

	//-----ログインしたプレイヤーの生成-----//
	RegisterPlayer(graphics);

	//-----プレイヤー関係の更新処理-----//
	PlayerManagerUpdate(graphics, elapsed_time);

	//-----プレイヤー関係の当たり判定-----//
	PlayerManagerCollision(graphics, elapsed_time);

	//--------------------< ボスのBGM切り替え&スカイボックスの色変える >--------------------//
	last_boss_mode = enemyManager->fGetBossMode();

	//-----ボスの形態によってゲームのパラメータを再設定する-----//
	SetBossTypeGameParam();

	SetSkyDomeColor(elapsed_time);

	//-----ボスのイベントカメラの設定-----//
	BossEventCamera();


	//-----プレイヤーにダッシュエフェクトをかける-----//
	player_manager->DashPostEffect(graphics, post_effect.get());

	//-----エネミーのHPゲージの更新処理-----//
	enemy_hp_gauge->update(graphics, elapsed_time);

	//-----ロックオンしたエネミーのHPバーを表示する-----//
	enemy_hp_gauge->focus(player_manager->GetTargetEnemy(), player_manager->GetEnemyLockOn());

	//-----ボスのHPゲージの更新-----//
	boss_hp_gauge->update(graphics, elapsed_time);

	//-----ボスの形態によってHPバーの表示を変更する-----//
	if (last_boss_mode == LastBoss::Mode::None) { boss_hp_gauge->set_animation(false); }
	else { boss_hp_gauge->set_animation(true); }

	//-----ロックオン時のレティクル更新-----//
	reticle->update(graphics, elapsed_time);

	//-----回り込み回避ができるときの色変更-----//
	reticle->SetAvoidanceCharge(player_manager->GetBehaindCharge());

	//-----ロックオンした敵に表示する-----//
	reticle->focus(player_manager->GetTargetEnemy(), player_manager->GetEnemyLockOn());
	{
		static DirectX::XMFLOAT2 pos{ 950.0f, 90.0f };
		static DirectX::XMFLOAT2 offset{ 50.0f, 0 };
		static DirectX::XMFLOAT2 scale{ 0.5f,0.5f };
		static DirectX::XMFLOAT4 color{ 1,1,1,1 };
		static int value{};
		static bool is_display_imgui = false;
		imgui_menu_bar("UI", "wave", is_display_imgui);

#if 0
#ifdef USE_IMGUI
		if (is_display_imgui)
		{
			ImGui::Begin("wave");
			ImGui::DragFloat2("pos", &pos.x);
			ImGui::DragFloat2("offset", &offset.x);
			ImGui::DragFloat2("scale", &scale.x, 0.01f);
			ImGui::DragInt("value", &value);
			ImGui::ColorEdit4("color", &color.x);
			ImGui::End();
		}

		ImGui::Begin("slow");
		ImGui::Checkbox("slow", &slow);
		ImGui::End();
		ImGui::Begin("game_over");
		ImGui::Checkbox("is_game_over", &is_game_over);
		ImGui::Checkbox("is_set_black", &is_set_black);
		if (is_game_over)
		{
			if (ImGui::TreeNode("game_over_text"))
			{
				ImGui::DragFloat2("pos", &game_over_text.position.x, 0.1f);
				ImGui::DragFloat2("scale", &game_over_text.scale.x, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("back_title"))
			{
				ImGui::DragFloat2("pos", &back_title.position.x, 0.1f);
				ImGui::DragFloat2("scale", &back_title.scale.x, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("again"))
			{
				ImGui::DragFloat2("pos", &again.position.x, 0.1f);
				ImGui::DragFloat2("scale", &again.scale.x, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("selecter"))
			{
				ImGui::DragFloat2("selecter1_pos", &selecter1.position.x, 0.1f);
				ImGui::DragFloat2("selecter1_scale", &selecter1.scale.x, 0.1f);
				ImGui::DragFloat2("selecter2_pos", &selecter2.position.x, 0.1f);
				ImGui::DragFloat2("selecter2_scale", &selecter2.scale.x, 0.1f);
				ImGui::TreePop();
			}
		}
		ImGui::End();

		ImGui::Begin("igame_clear");
		ImGui::Checkbox("is_game_clear", &is_game_clear);
		ImGui::Checkbox("is_set_black", &is_set_black);
		if (is_game_clear)
		{
			if (ImGui::TreeNode("game_clear_text"))
			{
				ImGui::DragFloat2("pos", &game_clear_text.position.x);
				ImGui::DragFloat2("scale", &game_clear_text.scale.x);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("selecter"))
			{
				ImGui::DragFloat2("selecter1_pos", &selecter1.position.x, 0.1f);
				ImGui::DragFloat2("selecter1_scale", &selecter1.scale.x, 0.1f);
				ImGui::DragFloat2("selecter2_pos", &selecter2.position.x, 0.1f);
				ImGui::DragFloat2("selecter2_scale", &selecter2.scale.x, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("back_title"))
			{
				ImGui::DragFloat2("pos", &back_title.position.x, 0.1f);
				ImGui::DragFloat2("scale", &back_title.scale.x, 0.1f);
				ImGui::TreePop();
			}

		}
		ImGui::End();
#endif

#endif // 0
		wave->set_positoin(pos); wave->set_offset(offset); wave->set_scale(scale); wave->set_value(value); wave->set_color(color);
		wave->update(graphics, elapsed_time);
	}

	// shadow_map
	shadow_map->debug_imgui();

	effect_manager->update(elapsed_time);

	//=================================================//
	//-------------敵のホスト権の譲渡リクエストなどの処理----------------//
	//=================================================//

	//-----譲渡のリクエスト結果の処理-----//
	CheckTransferEnemyControl();

	//-----敵のホスト権の返還処理-----//
	if (player_manager->GetReturnEnemyControl())
	{
		//-----敵のホスト権の返還-----//
		mWaveManager.ReturnEnemyControl();
		player_manager->SetReturnEnemyControl(false);
		mWaveManager.SetHost(false);
	}

	//****************************************************************
	//
	// オブジェクトの削除処理はこの下でやるルール
	//
	//****************************************************************
	enemyManager->fDeleteEnemies();

	//-----ログアウトしたプレイヤーを削除する-----//
	DeletePlayer();
}


#define OFF_SCREEN_RENDERING
#define SHADOW_MAP

void SceneMultiGameClient::render(GraphicsPipeline& graphics, float elapsed_time)
{
#ifdef OFF_SCREEN_RENDERING
	post_effect->begin(graphics.get_dc().Get());
#endif // DEBUG

	// シャドウマップのセット
#ifdef SHADOW_MAP
	shadow_map->set_shadowmap(graphics);
#endif // SHADOW_MAP
	// カメラのビュー行列計算
	cameraManager->CalcViewProjection(graphics);

	/*-----!!!ここから上にオブジェクトの描画はしないで!!!!-----*/
	{
		graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::DEFAULT);

#if 0
#ifdef USE_IMGUI
		ImGui::Begin("sky");
		ImGui::DragFloat("dimension", &dimension, 0.01f);
		ImGui::End();
#endif

#endif // 0
		sky_dome->Render(graphics, elapsed_time);


	}

	//--ゲームオーバー,ゲームクリア--//
	auto r_font_render = [&](StepFontElement& e)
	{
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		fonts->yu_gothic->Draw(e.s, e.position, e.scale, e.color, e.angle, TEXT_ALIGN::UPPER_LEFT, e.length);
	};

	auto r_sprite_render = [&](SpriteBatch* batch, Element& e)
	{
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		batch->begin(graphics.get_dc().Get());
		batch->render(graphics.get_dc().Get(), e.position, e.scale, e.pivot, e.color, e.angle, e.texpos, e.texsize);
		batch->end(graphics.get_dc().Get());
	};
	auto sprite_render = [&](std::string gui_name, SpriteBatch* batch, Element& e, float glow_horizon = 0, float glow_vertical = 0)
	{
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		//--sprite_string--//

#if 0
#ifdef USE_IMGUI
		ImGui::Begin("title");
		if (ImGui::TreeNode(gui_name.c_str()))
		{
			ImGui::DragFloat2("pos", &e.position.x, 0.1f);
			ImGui::DragFloat2("scale", &e.scale.x, 0.01f);
			ImGui::DragFloat4("color", &e.color.x, 0.01f);
			ImGui::TreePop();
		}
		ImGui::End();
#endif // USE_IMGUI

#endif // 0
		batch->begin(graphics.get_dc().Get());
		batch->render(graphics.get_dc().Get(), e.position, e.scale, e.pivot, e.color, e.angle, e.texpos, e.texsize, glow_horizon, glow_vertical);
		batch->end(graphics.get_dc().Get());
	};

	//-----弾のインスタンス取得-----//
	BulletManager& mBulletManager = BulletManager::Instance();

	//--------------------<敵の管理クラスの描画処理>--------------------//
	mWaveManager.fGetEnemyManager()->fRender(graphics);

	//-----プレイヤーの描画-----//
	if (mIsBossCamera == false)
	{
		//-----ゲームをクリアしたら自分だけを描画する-----//
		if(mWaveManager.GetClearFlg())player_manager->RenderOperationPlayer(graphics, elapsed_time);
		//-----それ以外は全員描画する-----//
		else player_manager->Render(graphics, elapsed_time);

	}
	mBulletManager.fRender(graphics);

	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	effect_manager->render(Camera::get_keep_view(), Camera::get_keep_projection());
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::WIREFRAME_CULL_BACK, DEPTH_STENCIL::DEON_DWON);
	debug_figure->render_all_figures(graphics.get_dc().Get());


	// クリア中のトンネル
	if (during_clear)
	{
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		tunnel->render(graphics.get_dc().Get(), elapsed_time, tunnel_alpha, [&]() {
			player_manager->RenderOperationPlayer(graphics, elapsed_time);
			});
	}

	/*-----!!!ここから下にオブジェクトの描画はしないで!!!!-----*/

	// シャドウマップの破棄
#ifdef SHADOW_MAP
	shadow_map->clear_shadowmap(graphics);
#endif // SHADOW_MAP

#ifdef OFF_SCREEN_RENDERING
	post_effect->end(graphics.get_dc().Get());
	// 描画ステート設定
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	// エフェクトをかける
	post_effect->apply_an_effect(graphics.get_dc().Get(), elapsed_time);
	post_effect->blit(graphics.get_dc().Get());
	post_effect->scene_preview();
#endif // DEBUG
	// bloom
	{
		// 定数バッファにフェッチする
		static bool display_bloom_imgui = false;

#if 0
#ifdef USE_IMGUI
		imgui_menu_bar("contents", "bloom", display_bloom_imgui);
		if (display_bloom_imgui)
		{
			ImGui::Begin("bloom");
			ImGui::DragFloat("extraction_threshold", &bloom_constants->data.bloom_extraction_threshold, 0.1f);
			ImGui::DragFloat("convolution_intensity", &bloom_constants->data.blur_convolution_intensity, 0.1f);
			ImGui::End();
		}
#endif // USE_IMGUI

#endif // 0
		bloom_constants->bind(graphics.get_dc().Get(), 8);

		graphics.set_pipeline_preset(BLEND_STATE::NO_PROCESS, RASTERIZER_STATE::CULL_NONE, DEPTH_STENCIL::DEOFF_DWOFF);
		bloom_effect->make(graphics.get_dc().Get(), post_effect->get_color_map().Get());
		graphics.set_pipeline_preset(BLEND_STATE::ADD, RASTERIZER_STATE::CULL_NONE, DEPTH_STENCIL::DEOFF_DWOFF);
		bloom_effect->blit(graphics.get_dc().Get());
	}
	//--------<ui>--------//
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	if (mIsBossCamera == false && is_game_clear == false)
	{
		// enemy_hp_gauge
		enemy_hp_gauge->render(graphics.get_dc().Get());
		// boss_hp_gauge
		boss_hp_gauge->render(graphics.get_dc().Get());
		// reticle
		reticle->render(graphics.get_dc().Get());
	}
	// wave
	Camera* c = cameraManager->GetCurrentCamera();
	const DirectX::XMFLOAT2 p_pos = { player_manager->GetMyTerminalPosition().x,player_manager->GetMyTerminalPosition().z };
	const DirectX::XMFLOAT2 p_forward = { player_manager->GetMyTerminalForward().x,player_manager->GetMyTerminalForward().z };
	const DirectX::XMFLOAT2 c_forward = { c->GetForward().x,c->GetForward().z };
	if (mIsBossCamera == false)player_manager->ConfigRender(graphics, elapsed_time);
	if (mIsBossCamera == false && during_clear == false && is_game_clear == false)
	{
		minimap->render(graphics, p_pos, p_forward, c_forward, mWaveManager.fGetEnemyManager()->fGetEnemies());
	}
	mWaveManager.render(graphics.get_dc().Get(), elapsed_time);

	//-----ゲームクリアした時の描画-----//
	if (is_game_clear)
	{
		glow_vertical -= elapsed_time * 0.2f;
		sprite_render("back", brack_back.get(), brack_back_pram, 0, glow_vertical);
		//画面が黒くなったら
		if (is_set_black)
		{
			sprite_render("frame", sprite_back.get(), game_over_pram, 0, glow_vertical);

			fonts->yu_gothic->Begin(graphics.get_dc().Get());
			r_font_render(game_clear_text);
			r_font_render(back_title);
			fonts->yu_gothic->End(graphics.get_dc().Get());
			r_sprite_render(sprite_selecter.get(), selecter1);
			r_sprite_render(sprite_selecter.get(), selecter2);
		}

	}

	//-----ゲームオーバーの時の描画-----//
	if (is_game_over)
	{
		glow_vertical -= elapsed_time * 0.2f;

		sprite_render("game_over_sprite", game_over_sprite.get(), game_over_sprite_pram, 0, 0);
		sprite_render("back", brack_back.get(), brack_back_pram, 0, glow_vertical);
		//画面が黒くなったら
		if (is_set_black)
		{
			sprite_render("frame", sprite_back.get(), game_over_pram, 0, glow_vertical);

			if (game_over_trying_again == false)
			{
				fonts->yu_gothic->Begin(graphics.get_dc().Get());
				r_font_render(game_over_text);
				r_font_render(back_title);
				r_font_render(again);
				fonts->yu_gothic->End(graphics.get_dc().Get());
				r_sprite_render(sprite_selecter.get(), selecter1);
				r_sprite_render(sprite_selecter.get(), selecter2);
			}
			else
			{
				fonts->yu_gothic->Begin(graphics.get_dc().Get());
				StepString(1.5f * elapsed_time, game_over_idle, game_over_select_idle, 2.0f, true);
				r_font_render(game_over_select_idle);
				fonts->yu_gothic->End(graphics.get_dc().Get());
			}

		}
	}

	//-----オプションを開いてる時にオプションを描画-----//
	if (option->get_validity()) { option->render(graphics, elapsed_time); }

}

void SceneMultiGameClient::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
	return;
}

bool SceneMultiGameClient::StepString(float elapsed_time, std::wstring full_text, StepFontElement& step_font_element, float speed, bool loop)
{
	step_font_element.timer += elapsed_time * speed;
	step_font_element.step = static_cast<int>(step_font_element.timer);
	size_t size = full_text.size();
	if (step_font_element.index >= size + 1) // 一文字分時間を置く
	{
		if (!loop)
		{
			return true;
		}
		else
		{
			step_font_element.timer = 0.0f;
			step_font_element.step = 0;
			step_font_element.index = 0;
			step_font_element.s = L"";
		}
	}

	if (step_font_element.step % 2 == 0)
	{
		if (step_font_element.index < size)
		{
			step_font_element.s += full_text[step_font_element.index];
			step_font_element.step = 1;
			step_font_element.timer = 1.0f;
		}
		++step_font_element.index;
	}

	return false;
}

void SceneMultiGameClient::GameOverAct(float elapsed_time)
{
	if (is_game_over)
	{
		//ここでタイトルに戻るの位置を決めているのは
		//ゲームクリアの時にも同じものを使うから
		back_title.position = { 365.6f,236.7f };
		back_title.scale = { 1.0f,1.0f };

		//画面ヲ徐々に黒くする
		if (brack_back_pram.color.w > 0.7f)
		{
			is_set_black = true;
			brack_back_pram.color.w = 0.7f;
		}
		if (is_set_black == false)
		{
			static float icon_time{ 0.0f };
			icon_time += 1.0f * elapsed_time;
			if (is_game_over_sprite == false && icon_time > 0.1f)
			{
				icon_time = 0.0f;
				game_over_sprite_pram.texpos.x += game_over_sprite_pram.texsize.x;
				//スプライトのtexposが最後まで行った時
				if (game_over_sprite_pram.texpos.x >= 1536.0f
					&& game_over_sprite_pram.texpos.y >= 512.0f)
				{
					audio_manager->play_se(SE_INDEX::DRAW_PEN);
					is_game_over_sprite = true;
				}
				//スプライトのtexpos.yが0ならリセット
				if (game_over_sprite_pram.texpos.x > 1536.0f
					&& game_over_sprite_pram.texpos.y < 512.0f)
				{
					audio_manager->play_se(SE_INDEX::DRAW_PEN);
					game_over_sprite_pram.texpos.x = 0.0f;
					game_over_sprite_pram.texpos.y += 512.0f;
				}

			}
			if (is_game_over_sprite)brack_back_pram.color.w += 1.0f * elapsed_time;
			//セレクターの初期化
			selecter1.position = { 328.0f,267.3f };
			selecter2.position = { 637.1f,267.3f };
		}
		// スティックを傾け続けたら少し間をおいて入力を許可する
		if (!can_axis)
		{
			axis_wait_timer += elapsed_time;
			if (axis_wait_timer > AXIS_WAIT_TIME)
			{
				axis_wait_timer = 0;
				can_axis = true;
			}
		}
		auto r_right_tutorial = [&](int state, DirectX::XMFLOAT2 arrival_posL, DirectX::XMFLOAT2 arrival_posR)
		{
			if ((game_pad->get_button_down() & GamePad::BTN_RIGHT) || (can_axis && game_pad->get_axis_LX() > 0.5f))
			{
				audio_manager->play_se(SE_INDEX::SELECT);
				game_over_state = state;
				selecter1.position = arrival_posL;
				selecter2.position = arrival_posR;

				can_axis = false;
			}
		};
		auto r_left_tutorial = [&](int state, DirectX::XMFLOAT2 arrival_posL, DirectX::XMFLOAT2 arrival_posR)
		{
			if ((game_pad->get_button_down() & GamePad::BTN_LEFT) || (can_axis && game_pad->get_axis_LX() < -0.5f))
			{
				audio_manager->play_se(SE_INDEX::SELECT);
				game_over_state = state;
				selecter1.position = arrival_posL;
				selecter2.position = arrival_posR;
				can_axis = false;
			}
		};
		//画面が黒くなってからしか動かないように
		if (is_set_black)
		{
			if (game_over_trying_again == false)
			{
				switch (game_over_state)
				{
				case 0://タイトルに戻る
					r_right_tutorial(1, { 710.5f,267.3f }, { 911.8f,267.3f });
					if (game_pad->get_button_down() & GamePad::BTN_B)
					{
						audio_manager->play_se(SE_INDEX::DECISION);
						SceneManager::scene_switching(new SceneLoading(new SceneTitle()), DISSOLVE_TYPE::TYPE1, 2.0f);
					}
					break;
				case 1://再挑戦
					r_left_tutorial(0, { 328.0f,267.3f }, { 637.1f,267.3f });
					if (game_pad->get_button_down() & GamePad::BTN_B)
					{
						audio_manager->play_se(SE_INDEX::DECISION);
						game_over_trying_again = true;

						//-----ホストに再挑戦を押したことをしらセル------//
						char data = CommandList::SelectTryingAgain;
						CorrespondenceManager::Instance().TcpSend(&data, 1);

						//SceneManager::scene_switching(new SceneLoading(new SceneMultiGameClient()), DISSOLVE_TYPE::TYPE1, 2.0f);
					}
					break;
				default:
					break;
				}
			}

			if (game_over_trying_again)
			{
				//-----ゲームのリトライフラグを取得したら初期化する-----//
				if (game_retry)
				{
					//-----フォントを変更-----//
					game_over_idle = L"ステージを再挑戦します";

					//-----ゲームを再挑戦-----//
					RestartInitialize();

				}
			}
		}
	}
}

void SceneMultiGameClient::GameClearAct(float elapsed_time, GraphicsPipeline& graphics)
{
	if (is_game_clear)
	{
		//プレイヤーのクリア用の更新処理
		const auto enemyManager = mWaveManager.fGetEnemyManager();
		cameraManager->Update(elapsed_time);
		player_manager->PlayerClearUpdate(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());

		if (set_joint_camera == false)
		{
			cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Joint));
			set_joint_camera = true;
		}
		if (set_joint_camera)
		{
			cameraManager->GetCurrentCamera()->set_eye(player_manager->GetMyTerminalJoint());
			cameraManager->GetCurrentCamera()->set_target(player_manager->GetMyTerminalEye());
		}
		//ここでタイトルに戻るの位置を決めているのは
	   //ゲームオーバーの時にも同じものを使うから
		back_title.position = { 529.7f,246.3f };
		back_title.scale = { 1.0f,1.0f };
		//セレクターの位置を入れる
		selecter1.position = { 495.3f,277.1f };
		selecter2.position = { 801.5f,277.1f };
		//プレイヤーのクリアモーションが終わってからしか動かないようにする
		if (player_manager->GetMyTerminalEndClearMotion())
		{
			//画面を徐々に黒くする
			if (brack_back_pram.color.w > 0.7f)
			{
				if (is_set_black == false)
				{
					audio_manager->play_bgm(BGM_INDEX::CLEAR);
					is_set_black = true;
				}
				brack_back_pram.color.w = 0.7f;
			}
			if (is_set_black == false) brack_back_pram.color.w += 1.0f * elapsed_time;

			if (is_set_black)
			{
				if (game_pad->get_button_down() & GamePad::BTN_B)
				{
					audio_manager->play_se(SE_INDEX::DECISION);
					SceneManager::scene_switching(new SceneLoading(new SceneTitle()), DISSOLVE_TYPE::TYPE1, 2.0f);
				}
			}
		}
	}

}

void SceneMultiGameClient::JudgeSlow(float& elapsed_time)
{
	//プレイヤーがジャスト回避したらslow
	if (player_manager->GetIsJustAvoidance())
	{
		slow = true;
	}
	else
	{
		slow_timer = 0.0f;
		slow = false;
	}
	//slowがtrueなら
	if (slow)
	{
		slow_timer += 1.0f * elapsed_time;
		//タイマーが1.0秒以下なら遅くする
		if (slow_timer < 1.0f)
		{
			elapsed_time *= slow_rate;
		}
	}
}

void SceneMultiGameClient::SetBossTypeGameParam()
{
	//-----カメラのインスタンスを取得-----//
	Camera* c = cameraManager->GetCurrentCamera();
	//-----ボスが人型になる時-----//
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::ShipToHuman)
	{
		c->boss_animation = true;
		player_manager->SetPlayerPosition({ 0,0,-120.0f });
		//プレイヤーの行動範囲変更
		player_manager->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_HUMANOID);
		purple_threshold = 0.01f;
	}

	//-----ボスがドラゴンになる時-----//
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::HumanToDragon)
	{
		c->boss_animation = true;
		player_manager->SetPlayerPosition({ 0,0,-120.0f });
		//プレイヤーの行動範囲変更
		player_manager->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_DRAGON);
		red_threshold = 0.01f;
	}

	//-----通常時-----//
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::ShipAppear)
	{
		c->boss_animation = true;
		player_manager->SetPlayerPosition({ 0,0,-120.0f });
		//プレイヤーの行動範囲変更.
		player_manager->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_BATTLESHIP);
	}

	//-----再挑戦時-----//
	if (old_last_boss_mode == LastBoss::Mode::Ship && last_boss_mode == LastBoss::Mode::ShipToHuman)
	{
		c->boss_animation = true;
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_HUMANOID);
		purple_threshold = 0.01f;
	}
	if (old_last_boss_mode == LastBoss::Mode::Human && last_boss_mode == LastBoss::Mode::HumanToDragon)
	{
		c->boss_animation = true;
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_DRAGON);
		red_threshold = 0.01f;
	}
}

void SceneMultiGameClient::BossEventCamera()
{
	//-----敵のインスタンスを生成-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	//--------------------<ボスの方にカメラを向ける処理>--------------------//
	if (enemyManager->fGetIsEventCamera() && !mIsBossCamera)
	{
		post_effect->clear_post_effect();
		// まだボスのほうを向いていないとき
		mIsBossCamera = true;
		cameraManager->SetCamera(static_cast<int>(CameraTypes::Joint));
	}

	//-----ボスのイベントカメラになる-----//
	if (enemyManager->fGetIsEventCamera() && mIsBossCamera)
	{
		const DirectX::XMFLOAT3 eye = enemyManager->fGetEye();
		const DirectX::XMFLOAT3 focus = enemyManager->fGetFocus();

		// カメラがEnemyManagerを経由し555たボスによって更新される
		cameraManager->GetCurrentCamera()->set_eye(eye);
		cameraManager->GetCurrentCamera()->set_target(focus);

		// 敵の移動を制限
		enemyManager->fLimitEnemies();
	}

	//-----イベントカメラ終了-----//
	if (!enemyManager->fGetIsEventCamera() && mIsBossCamera)
	{
		// カメラ処理終了
		mIsBossCamera = false;
		cameraManager->SetCamera(static_cast<int>(CameraTypes::Game));
	}

}

void SceneMultiGameClient::SetSkyDomeColor(float elapsed_time)
{
	//-----SkyDomeの色-----//
	if (purple_threshold >= 0.01f && purple_threshold <= 1.0f)
	{
		purple_threshold += 0.2f * elapsed_time;
		SkyDome::set_purple_threshold(purple_threshold);
	}
	if (red_threshold >= 0.01f && red_threshold <= 1.0f)
	{
		red_threshold += 0.2f * elapsed_time;
		SkyDome::set_red_threshold(red_threshold);
	}
}

void SceneMultiGameClient::PlayerManagerUpdate(GraphicsPipeline& graphics, float elapsed_time)
{
	//-----敵のインスタンスを生成-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	//-----プレイヤーに一番近い敵を取得-----//
	BaseEnemy* enemy = enemyManager->fGetNearestEnemyPosition();

	//-----カメラのインスタンスを取得-----//
	Camera* c = cameraManager->GetCurrentCamera();

	//-----ボス用のカメラに設定する-----//
	player_manager->SetBossCamera(mIsBossCamera);

	//-----カメラの方向を設定する-----//
	player_manager->SetCameraDirection(c->GetForward(), c->GetRight());

	//-----カメラのview行列とプロジェクション行列を設定する-----//
	player_manager->SetCameraView(c->get_keep_view());
	player_manager->SetCameraProjection(c->get_keep_projection());

	//-----更新処理-----//
	player_manager->Update(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());

	//-----クライアントがロックオンしている敵を設定する-----//
	player_manager->SearchClientPlayerLockOnEnemy(enemyManager);

	//-----ロックオンのポストエフェクトをかける-----//
	player_manager->LockOnPostEffect(elapsed_time, post_effect.get());

	//-----カメラの位置を設定する-----//
	player_manager->SetCameraPosition(c->get_eye());

	//-----プレイヤーに一番近い敵を設定する-----//
	player_manager->SetTarget(enemy);

	//-----プレイヤーにカメラのターゲットを設定する-----//
	player_manager->SetCameraTarget(c->get_target());
}

void SceneMultiGameClient::PlayerManagerCollision(GraphicsPipeline& graphics, float elapsed_time)
{
	//-----弾のインスタンスを生成-----//
	BulletManager& mBulletManager = BulletManager::Instance();

	//-----敵のインスタンスを生成-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	//-----クライアントがロックオンしている敵を設定する-----//
	player_manager->SearchClientPlayerLockOnEnemy(enemyManager);

	//-----敵とのあたり判定(当たったらコンボ加算)-----//
	player_manager->PlayerAttackVsEnemy(enemyManager, graphics, elapsed_time);

	////-----ジャスト回避が可能かどうかの当たり判定-----//
	player_manager->PlayerCounterVsEnemyAttack(enemyManager);

	//-----プレイヤーの体力の同期をとる-----//
	ReceivePlayerHealthData();

	////-----敵の攻撃とプレイヤーの当たり判定-----//
	player_manager->EnemyAttackVsPlayer(enemyManager);

	////-----プレイヤーがジャスト回避した時の範囲スタンの当たり判定-----//
	player_manager->PlayerStunVsEnemy(enemyManager);

	////-----プレイヤーがチェイン状態であることを敵に知らせて行動を停止させる-----//
	//player_manager->SetPlayerChainTime(enemyManager);

	////-----弾とプレイヤーの当たり判定-----//
	player_manager->BulletVsPlayer(mBulletManager);

}

void SceneMultiGameClient::RegisterPlayer(GraphicsPipeline& graphics)
{
	//-----	排他制御-----//
	std::lock_guard<std::mutex> lock(mutex);

	//クライアントがログインして来たら
	if (register_player && register_player_id >= 0)
	{
		ClientPlayer* player = new ClientPlayer(graphics, register_player_id);
		player->SetName(CorrespondenceManager::Instance().names[register_player_id]);
		player->SetColor(static_cast<BasePlayer::PlayerColor>(register_player_color));

		player_manager->RegisterPlayer(player);

		//プレイヤーを追加
		std::string text = std::to_string(register_player_id) + "番目にプレイヤーを追加しました";
		DebugConsole::Instance().WriteDebugConsole(text, TextColor::Green);
		//追加する時に必要なパラメータとフラグの初期化
		register_player_id = -1;
		register_player = false;
		register_player_color = 0;

		//-----プレイヤーの体力を増やす-----//
		player_manager->AddPlayerMultiHealth();

		//-----接続者数を増やす-----//
		CorrespondenceManager::Instance().AddConnectedPersons();

	}

}

void SceneMultiGameClient::DeletePlayer()
{
	//-----配列が空なら処理をしない-----//
	if (logout_id.empty()) return;

	//-----排他制御-----//
	SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);
	std::lock_guard<std::mutex> lock3(instance.GetMutex());

	//-----ログアウトしたプレイヤーを削除する-----//
	for (auto id : logout_id)
	{
		//-----接続者の番号をリセット-----//
		CorrespondenceManager::Instance().SetOpponentPlayerId(id, -1);

		//-----アドレスを削除-----//
		instance.game_udp_server_addr[id].sin_addr.S_un.S_addr = 0;

		//-----プレイヤーの削除-----//
		player_manager->DeletePlayer(id);

		//-----プレイヤーの体力を減らす-----//
		player_manager->SubPlayerMultiHealth();

		//-----接続者数を減らす-----//
		CorrespondenceManager::Instance().SubConnectedPersons();
	}



	//-----ログアウトデータを削除する-----//
	logout_id.clear();

}

void SceneMultiGameClient::SetReceiveData()
{
	//-----プレイヤーの動きデータが入っている場合-----//
	if (receive_all_player_data.player_move_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----データを設定する-----//
		for (const auto& p_data : receive_all_player_data.player_move_data)
		{
			player_manager->SetPlayerMoveData(p_data);
		}

		//-----データを削除する-----//
		receive_all_player_data.player_move_data.clear();
	}

	//-----プレイヤーの位置データが入っている場合-----//
	if (receive_all_player_data.player_position_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----データを設定する-----//
		for (const auto& p_data : receive_all_player_data.player_position_data)
		{
			player_manager->SetPlayerPositionData(p_data);
		}

		//-----データを削除する-----//
		receive_all_player_data.player_position_data.clear();
	}

	//-----プレイヤーのアクションデータが入っている場合-----//
	if (receive_all_player_data.player_action_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----データを設定する-----//
		for (const auto& p_data : receive_all_player_data.player_action_data)
		{
			player_manager->SetPlayerActionData(p_data);
		}

		//-----データを削除する-----//
		receive_all_player_data.player_action_data.clear();
	}

	//-----チェイン攻撃のデータ設定-----//
	if (chain_rock_on_enemy_id.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);

		for (const auto& data : chain_rock_on_enemy_id)
		{
			player_manager->ReceiveLockOnChain(data.first, data.second);
		}

		//-----データを削除する-----//
		chain_rock_on_enemy_id.clear();
	}
}

void SceneMultiGameClient::ClearEnemyReceiveData()
{
	//-----敵の出現データを削除する-----//
	receive_all_enemy_data.enemy_spawn_data.clear();

	//-----敵の動きデータを削除する-----//
	receive_all_enemy_data.enemy_move_data.clear();

	//-----ダメージデータ削除-----//
	receive_all_enemy_data.enemy_damage_data.clear();

	//-----死亡データ削除-----//
	receive_all_enemy_data.enemy_die_data.clear();

	//-----状態データ削除-----//
	receive_all_enemy_data.enemy_condition_data.clear();

}

void SceneMultiGameClient::ReceivePlayerHealthData()
{
	if (receive_all_player_data.player_health_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----データを設定する-----//
		for (const auto& p_data : receive_all_player_data.player_health_data)
		{
			player_manager->ReceivePlayerHealthData(p_data);
		}

		//-----データを削除する-----//
		receive_all_player_data.player_health_data.clear();
	}

}

void SceneMultiGameClient::CheckTransferEnemyControl()
{
	if (transfer_enemy_host_result)
	{
		switch (transfer_enemy_result.data[TransferEnemyControl::DataArray::Result])
		{
			//-----許可-----//
		case TransferEnemyControl::Result::Permit:
			mWaveManager.SetHost(true);
			DebugConsole::Instance().WriteDebugConsole("許可を受けました", TextColor::Green);
			player_manager->PermitChainAttack();
			break;
			//-----無許可-----//
		case TransferEnemyControl::Result::Prohibition:
			DebugConsole::Instance().WriteDebugConsole("許可をもらえませんでした", TextColor::Red);

			break;
		default:
			DebugConsole::Instance().WriteDebugConsole("値がおかしいです", TextColor::Red);
			break;
		}

		//-----初期値を入れる-----//
		transfer_enemy_result.data[TransferEnemyControl::DataArray::Result] = TransferEnemyControl::Result::None;
		//-----処理をしたらフラグを戻す-----//
		transfer_enemy_host_result = false;
	}
}

void SceneMultiGameClient::ResetPlayerAllData()
{
	receive_all_player_data.player_action_data.clear();
	receive_all_player_data.player_health_data.clear();
	receive_all_player_data.player_move_data.clear();
	receive_all_player_data.player_position_data.clear();
}

void SceneMultiGameClient::ResetEnemyAllData()
{
	receive_all_enemy_data.enemy_condition_data.clear();
	receive_all_enemy_data.enemy_damage_data.clear();
	receive_all_enemy_data.enemy_die_data.clear();
	receive_all_enemy_data.enemy_move_data.clear();
	receive_all_enemy_data.enemy_spawn_data.clear();
}
