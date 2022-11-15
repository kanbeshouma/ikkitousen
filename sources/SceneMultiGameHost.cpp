#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include<chrono>

#include "SceneMultiGameHost.h"
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

#include"SocketCommunication.h"
#include"Correspondence.h"

//-----TCP�X���b�h���I�����邩�̃t���O-----//
bool SceneMultiGameHost::end_tcp_thread = false;

//-----UDP�X���b�h���I�����邩�̃t���O-----//
bool SceneMultiGameHost::end_udp_thread = false;

//-----�v���C���[���o�^���ꂽ���ǂ���-----//
bool SceneMultiGameHost::register_player = false;

//-----�ǉ����ꂽ�v���C���[�̔ԍ�-----//
int SceneMultiGameHost::register_player_id = -1;

//-----�ǉ����ꂽ�v���C���[�̐F-----//
int SceneMultiGameHost::register_player_color = 0;

//-----���O�A�E�g����v���C���[�̔ԍ�-----//
std::vector<int> SceneMultiGameHost::logout_id = {};

//-----�u���b�L���O-----//
std::mutex SceneMultiGameHost::mutex;

PlayerAllDataStruct SceneMultiGameHost::receive_all_data;

EnemyAllDataStruct SceneMultiGameHost::receive_all_enemy_data;

//-----�G�̃z�X�g���̏��n���ꂽ���ǂ���-----//
bool SceneMultiGameHost::transfer_enemy_host_result = false;

//-----���N�G�X�g���ė����v���C���[�̔ԍ�-----//
int SceneMultiGameHost::transfer_enemy_request_id = -1;

//-----�G�̃z�X�g���̏��n���N�G�X�g-----//
bool SceneMultiGameHost::transfer_enemy_host_request = false;

//-----�G�̃z�X�g�����A���ė������ǂ���-----//
bool SceneMultiGameHost::return_enemy_control = false;

//-----���̃X�e�[�W-----//
WaveManager::STAGE_IDENTIFIER SceneMultiGameHost::current_stage = WaveManager::STAGE_IDENTIFIER::S_1_1;

//-----�N���C�A���g���I�������X�e�[�W-----//
std::vector<WaveManager::STAGE_IDENTIFIER>  SceneMultiGameHost::client_select_stage;

SceneMultiGameHost::SceneMultiGameHost()
{
}

SceneMultiGameHost::~SceneMultiGameHost()
{
	end_tcp_thread = true;

	//-----TCP�X���b�h���I������-----//
	tcp_thread.join();
	end_udp_thread = true;

	//-----UDP�X���b�h���I������-----//
	udp_thread.join();
}

void SceneMultiGameHost::initialize(GraphicsPipeline& graphics)
{
	// shadow_map
	shadow_map = std::make_unique<ShadowMap>(graphics);
	// post effect
	post_effect = std::make_unique<PostEffect>(graphics.get_device().Get());
	// bloom
	bloom_effect = std::make_unique<Bloom>(graphics.get_device().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
	bloom_constants = std::make_unique<Constants<BloomConstants>>(graphics.get_device().Get());
	// ���f���̃��[�h
	sky_dome = std::make_unique<SkyDome>(graphics);

	//--------------------<�e�̊Ǘ��N���X��������>--------------------//
	BulletManager& mBulletManager = BulletManager::Instance();
	mBulletManager.fInitialize();
	//--------------------<�G�̊Ǘ��N���X��������>--------------------//
	mWaveManager.fInitialize(graphics, mBulletManager.fGetAddFunction());
	//-----�z�X�g���ǂ�����ݒ�-----//
	mWaveManager.SetHost(true);

	//-----���̃X�e�[�W��ۑ����Ă���-----//
	current_stage = mWaveManager.get_current_stage();

	player_manager = std::make_unique<PlayerManager>();
	//-----�v���C���[��o�^-----//
	Player* player = new Player(graphics, PlayerPrivateObjectId);
	player->SetName(CorrespondenceManager::Instance().my_name);
	player->SetColor(static_cast<BasePlayer::PlayerColor> (CorrespondenceManager::Instance().my_player_color));
    player_manager->RegisterPlayer(player);
    player_manager->SetPrivateObjectId(PlayerPrivateObjectId);

	//----------�v���C���[(���삷�邱�Ƃ��ł��鎩���̂���)�̔ԍ���ۑ�
	CorrespondenceManager::Instance().SetOperationPrivateId(player_manager->GetPrivatePlayerId());
	CorrespondenceManager::Instance().SetHostId(player_manager->GetPrivatePlayerId());
	//-----�ڑ��Ґ��𑝉�������-----//
	CorrespondenceManager::Instance().AddConnectedPersons();

	//-----�T�[�o�[�̃\�P�b�g���̏�����-----//
	if (CorrespondenceManager::Instance().InitializeServer())
	{
		DebugConsole::Instance().WriteDebugConsole("�z�X�g: �\�P�b�g�̍쐬�ɐ������܂���", TextColor::Green);

		//-----TCP�p�̃}���`�X���b�h�𗧂��グ��-----//
		{
			end_tcp_thread = false;
			std::thread t(ReceiveTcpData);
			t.swap(tcp_thread);
		}

	    //-----UDP�p�̃}���`�X���b�h�𗧂��グ��-----//
		{
			end_udp_thread = false;
			std::thread t(ReceiveUdpData);
			t.swap(udp_thread);
		}
		CorrespondenceManager::Instance().SetHost(true);
	}
	else
	{
		DebugConsole::Instance().WriteDebugConsole("�z�X�g: �\�P�b�g�̍쐬�Ɏ��s���܂���", TextColor::Red);
	}

	// �J����
	cameraManager = std::make_unique<CameraManager>();
	cameraManager->RegisterCamera(new GameCamera(player));
	cameraManager->RegisterCamera(new ClearCamera(player));
	cameraManager->RegisterCamera(new JointCamera(graphics));
	cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));

	//-----�{�X�̉��o-----//
	last_boss_mode = LastBoss::Mode::None;
	old_last_boss_mode = LastBoss::Mode::None;

	//-----�I�[�f�B�I-----//
	audio_manager->stop_all_bgm();
	audio_manager->play_bgm(BGM_INDEX::GAME);

	//-----�X�J�C�h�[��-----//
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
	game_over_text.s = L"�Q�[���I�[�o�[";
	game_over_text.position = { 553.1f,124.3f };
	game_over_text.scale = { 1.0f,1.0f };
	back_title.s = L"�^�C�g���ɖ߂�";
	again.s = L"�Ē���";
	again.position = { 751.6f,236.7f };
	again.scale = { 1.0f,1.0f };
	game_clear_text.s = L"�Q�[���N���A";
	game_clear_text.position = { 552.0f,127.0f };


	//-----�Q�[���X�^�[�g��UI�Ɏg���ϐ��̏�����-----//
	game_start_gauge = std::make_unique<SpriteDissolve>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\skip.png",
		L".\\resources\\Sprites\\mask\\dissolve_mask1.png", 1);

	controller_on__back_keys = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\back_2_on.png", 1);

	controller_back_pram.texsize = { static_cast<float>(controller_on__back_keys->get_texture2d_desc().Width),
								static_cast<float>(controller_on__back_keys->get_texture2d_desc().Height) };
	controller_back_pram.scale = { 0.8f,0.8f };
	controller_back_pram.position = { -52.6f,194.0f };

	game_start_gauge_parm.pos = { 62.3f,439.6f };
	game_start_txt_parm.position = { 88.0f,445.1f };


	//-----�}���`�X���b�h�Ŏg�p����ϐ�������������-----//

	//-----�v���C���[�ǉ��ϐ��̏�����-----//
	register_player = false;
	register_player_id = -1;
}

void SceneMultiGameHost::uninitialize()
{

	audio_manager->stop_all_se();
	audio_manager->stop_all_bgm();


	mWaveManager.fFinalize();
	BulletManager::Instance().fFinalize();
}

void SceneMultiGameHost::effect_liberation(GraphicsPipeline& graphics)
{
	effect_manager->finalize();
	effect_manager->initialize(graphics);
}

void SceneMultiGameHost::update(GraphicsPipeline& graphics, float elapsed_time)
{
	//-----�Q�[���N���A�A�Q�[���I�[�o�[�ł��Ȃ��Ƃ�-----//
	if (is_game_clear == false && is_game_over == false)
	{
		brack_back_pram.color.w = 0;
		is_set_black = false;
	}

	//-----�Q�[���N���A�ɂȂ�����-----//
	GameClearAct(elapsed_time, graphics);
	//-----�Q�[���I�[�o�[�ɂȂ�����-----//
	GameOverAct(elapsed_time);

	//-----�I�[�f�B�I-----//
	const float bgm_volume = 2.0f;
	const float se_volume = 2.0f;
	audio_manager->set_all_volume_bgm(bgm_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_bgm_volume());
	audio_manager->set_all_volume_se(se_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_se_volume());

	//-----�Q�[���I�[�o�[,�Q�[���N���A�̎��͎~�߂�-----//
	if (is_game_over) return;
	if (is_game_clear) return;

	//-----�Q�[���N���A-----//
	if (mWaveManager.get_game_clear()) { is_game_clear = true; }

	//-----option------//
	if (option->get_validity())
	{
		option->update(graphics, elapsed_time);
		return;
	}
	else
	{
		//-----�I�v�V�����{�^������������-----//
		if (game_pad->get_button_down() & GamePad::BTN_START)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			option->initialize();
			option->set_validity(true);
			return;
		}
	}

	//-----�Q�[���X�^�[�g���邩�ǂ����̔���-----//
	if(is_start_game == false)StartGameUi(elapsed_time);

	//-----�e�̃C���X�^���X�𐶐�-----//
	BulletManager& mBulletManager = BulletManager::Instance();


	if (is_start_game)
	{
		{
			//-----���b�N����-----//
			std::lock_guard<std::mutex> lock(mutex);

			if (client_select_stage.empty() == false)
			{
				//-----�I�������X�e�[�W�f�[�^��ݒ�-----//
				for (auto data : client_select_stage)
				{
					mWaveManager.SetStageVoting(data);
				}
				//-----�f�[�^���폜-----//
				client_select_stage.clear();
			}
			//-----�X�e�[�W���̃E�F�[�u�̍X�V����-----//
			mWaveManager.fMultiPlayUpdate(graphics, elapsed_time, mBulletManager.fGetAddFunction(), receive_all_enemy_data);

			//-----�G�̃f�[�^���폜-----//
			ClearEnemyReceiveData();
		}
	}
	//-----�N���A���o-----//
	if (mWaveManager.during_clear_performance())
	{
		//-----�g���l���̃A���t�@�l�𑝂₷-----//
		tunnel_alpha += elapsed_time * 0.5f;
		tunnel_alpha = (std::min)(tunnel_alpha, 1.0f);

		if (!during_clear)
		{
			//-----�J�������g���l���p�ɐ؂�ւ���-----//
			cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Tunnel));

			//-----�v���C���[���ړ����[�h�ɕς���-----//
			player_manager->TransitionStageMove();
			during_clear = true;
		}
	}
	else
	{

		if (during_clear)
		{
			//-----�g���l���𔖂����Ă���-----//
			tunnel_alpha -= elapsed_time;
			tunnel_alpha = (std::max)(tunnel_alpha, 0.0f);

			//-----�g���l���̃A���t�@�l���ق�0�Ȃ�-----//
			if (Math::equal_check(tunnel_alpha, 0.0f, 0.01f))
			{
				//-----�J�������Q�[���p�ɕύX-----//
				cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));

				//-----�v���C���[���Q�[���p�ɐ؂�ւ���-----//
				player_manager->TransitionStageMoveEnd();
				tunnel_alpha = 0.0f;
				during_clear = false;
			}
		}
	}

	//-----��M�f�[�^��ݒ肷��-----//
	SetReceiveData();

	//-----	�X���E����-----//
	JudgeSlow(elapsed_time);

	//--------------------<�G�̊Ǘ��N���X�̍X�V����>--------------------//
	old_last_boss_mode = last_boss_mode;

	//-----�G�̃C���X�^���X�𐶐�-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	// camera
	cameraManager->Update(elapsed_time);

	// �J�����̃r���[�s��v�Z
	cameraManager->CalcViewProjection(graphics);


	//-----�v���C���[�̈ʒu��ݒ�-----//
	enemyManager->fSetPlayerPosition(player_manager->GetPosition());

	//-----�v���C���[�̃`�F�C���U���̃��b�N�I�������ǂ�����ݒ�-----//
	enemyManager->fSetPlayerSearch(player_manager->DuringSearchTime());

	//-----�e�̍X�V����-----//
	mBulletManager.fUpdate(elapsed_time);

	//-----�������������������v���C���[�̍X�V�͂��̂�������������-----//


	//-----�v���C���[�����񂾂�Q�[���I�[�o�[-----//
	if (player_manager->GetIsAlive() == false)	is_game_over = true;

	//-----���O�C�������v���C���[�̐���-----//
	RegisterPlayer(graphics);

	//-----�v���C���[�֌W�̍X�V����-----//
	PlayerManagerUpdate(graphics, elapsed_time);

	//-----�v���C���[�֌W�̓����蔻��-----//
	PlayerManagerCollision(graphics, elapsed_time);

	//--------------------< �{�X��BGM�؂�ւ�&�X�J�C�{�b�N�X�̐F�ς��� >--------------------//
	last_boss_mode = enemyManager->fGetBossMode();

	//-----�{�X�̌`�Ԃɂ���ăQ�[���̃p�����[�^���Đݒ肷��-----//
	SetBossTypeGameParam();

	SetSkyDomeColor(elapsed_time);

	//-----�{�X�̃C�x���g�J�����̐ݒ�-----//
	BossEventCamera();


	//-----�v���C���[�Ƀ_�b�V���G�t�F�N�g��������-----//
	player_manager->DashPostEffect(graphics, post_effect.get());

	//-----�G�l�~�[��HP�Q�[�W�̍X�V����-----//
	enemy_hp_gauge->update(graphics, elapsed_time);

	//-----���b�N�I�������G�l�~�[��HP�o�[��\������-----//
	enemy_hp_gauge->focus(player_manager->GetTargetEnemy(), player_manager->GetEnemyLockOn());

	//-----�{�X��HP�Q�[�W�̍X�V-----//
	boss_hp_gauge->update(graphics, elapsed_time);

	//-----�{�X�̌`�Ԃɂ����HP�o�[�̕\����ύX����-----//
	if (last_boss_mode == LastBoss::Mode::None) { boss_hp_gauge->set_animation(false); }
	else { boss_hp_gauge->set_animation(true); }

	//-----���b�N�I�����̃��e�B�N���X�V-----//
	reticle->update(graphics, elapsed_time);

	//-----��荞�݉�����ł���Ƃ��̐F�ύX-----//
	reticle->SetAvoidanceCharge(player_manager->GetBehaindCharge());

	//-----���b�N�I�������G�ɕ\������-----//
	reticle->focus(player_manager->GetTargetEnemy(), player_manager->GetEnemyLockOn());
	{
		static DirectX::XMFLOAT2 pos{ 950.0f, 90.0f };
		static DirectX::XMFLOAT2 offset{ 50.0f, 0 };
		static DirectX::XMFLOAT2 scale{ 0.5f,0.5f };
		static DirectX::XMFLOAT4 color{ 1,1,1,1 };
		static int value{};
		static bool is_display_imgui = false;
		imgui_menu_bar("UI", "wave", is_display_imgui);
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
		wave->set_positoin(pos); wave->set_offset(offset); wave->set_scale(scale); wave->set_value(value); wave->set_color(color);
		wave->update(graphics, elapsed_time);
	}

	// shadow_map
	shadow_map->debug_imgui();

	effect_manager->update(elapsed_time);

	//=================================================//
	//-------------�G�̃z�X�g���̏��n���N�G�X�g�Ȃǂ̏���----------------//
	//=================================================//
	TransferEnenyControlProcessing();

	ReturnEnemyControl();

	//-----�G�̃z�X�g�������鎞�ɂ����`�F�C���U�����ł��Ȃ��悤�ɂ���-----//
	if (mWaveManager.GetHost())player_manager->PermitChainAttack();
	else player_manager->ProhibitionChainAttack();


	//****************************************************************
	//
	// �I�u�W�F�N�g�̍폜�����͂��̉��ł�郋�[��
	//
	//****************************************************************
	enemyManager->fDeleteEnemies();

	//-----���O�A�E�g�����v���C���[���폜����-----//
	DeletePlayer();
}

#define OFF_SCREEN_RENDERING
#define SHADOW_MAP

void SceneMultiGameHost::render(GraphicsPipeline& graphics, float elapsed_time)
{
#ifdef OFF_SCREEN_RENDERING
	post_effect->begin(graphics.get_dc().Get());
#endif // DEBUG

	// �V���h�E�}�b�v�̃Z�b�g
#ifdef SHADOW_MAP
	shadow_map->set_shadowmap(graphics);
#endif // SHADOW_MAP

	/*-----!!!���������ɃI�u�W�F�N�g�̕`��͂��Ȃ���!!!!-----*/
	{
		graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::DEFAULT);
#ifdef USE_IMGUI
		ImGui::Begin("sky");
		ImGui::DragFloat("dimension", &dimension, 0.01f);
		ImGui::End();
#endif
		sky_dome->Render(graphics, elapsed_time);


	}

	//--�Q�[���I�[�o�[,�Q�[���N���A--//
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
		batch->begin(graphics.get_dc().Get());
		batch->render(graphics.get_dc().Get(), e.position, e.scale, e.pivot, e.color, e.angle, e.texpos, e.texsize, glow_horizon, glow_vertical);
		batch->end(graphics.get_dc().Get());
	};

	//-----�e�̃C���X�^���X�擾-----//
	BulletManager& mBulletManager = BulletManager::Instance();

	//--------------------<�G�̊Ǘ��N���X�̕`�揈��>--------------------//
	mWaveManager.fGetEnemyManager()->fRender(graphics);
	if (mIsBossCamera == false)
	{
		//-----�Q�[�����N���A�����玩��������`�悷��-----//
		if (mWaveManager.GetClearFlg())player_manager->RenderOperationPlayer(graphics, elapsed_time);
		//-----����ȊO�͑S���`�悷��-----//
		else player_manager->Render(graphics, elapsed_time);
	}
	mBulletManager.fRender(graphics);

	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	effect_manager->render(Camera::get_keep_view(), Camera::get_keep_projection());
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::WIREFRAME_CULL_BACK, DEPTH_STENCIL::DEON_DWON);
	debug_figure->render_all_figures(graphics.get_dc().Get());


	// �N���A���̃g���l��
	if (during_clear)
	{
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		tunnel->render(graphics.get_dc().Get(), elapsed_time, tunnel_alpha, [&]() {
			player_manager->RenderOperationPlayer(graphics, elapsed_time);
			});
	}

	/*-----!!!�������牺�ɃI�u�W�F�N�g�̕`��͂��Ȃ���!!!!-----*/

	// �V���h�E�}�b�v�̔j��
#ifdef SHADOW_MAP
	shadow_map->clear_shadowmap(graphics);
#endif // SHADOW_MAP

#ifdef OFF_SCREEN_RENDERING
	post_effect->end(graphics.get_dc().Get());
	// �`��X�e�[�g�ݒ�
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	// �G�t�F�N�g��������
	post_effect->apply_an_effect(graphics.get_dc().Get(), elapsed_time);
	post_effect->blit(graphics.get_dc().Get());
	post_effect->scene_preview();
#endif // DEBUG
	// bloom
	{
		// �萔�o�b�t�@�Ƀt�F�b�`����
		static bool display_bloom_imgui = false;
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

	//-----�Q�[���X�^�[�g���Ă��Ȃ��Ƃ��ɂ����`�悵�Ȃ�-----//
	if (is_start_game == false)
	{
		game_start_gauge->begin(graphics.get_dc().Get());
		game_start_gauge->render(graphics.get_dc().Get(), game_start_gauge_parm.pos, game_start_gauge_parm.scale, game_start_gauge_parm.threshold);
		game_start_gauge->end(graphics.get_dc().Get());

		sprite_render("controller_back_button_pram", controller_on__back_keys.get(), controller_back_pram, 0, 0);

		fonts->yu_gothic->Begin(graphics.get_dc().Get());
		fonts->yu_gothic->Draw(game_start_text, game_start_txt_parm.position, game_start_txt_parm.scale, game_start_txt_parm.color, game_start_txt_parm.angle, TEXT_ALIGN::UPPER_LEFT);
		fonts->yu_gothic->End(graphics.get_dc().Get());

	}

	//-----�Q�[���N���A�������̕`��-----//
	if (is_game_clear)
	{
		glow_vertical -= elapsed_time * 0.2f;
		sprite_render("back", brack_back.get(), brack_back_pram, 0, glow_vertical);
		//��ʂ������Ȃ�����
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

	//-----�Q�[���I�[�o�[�̎��̕`��-----//
	if (is_game_over)
	{
		glow_vertical -= elapsed_time * 0.2f;

		sprite_render("game_over_sprite", game_over_sprite.get(), game_over_sprite_pram, 0, 0);
		sprite_render("back", brack_back.get(), brack_back_pram, 0, glow_vertical);
		//��ʂ������Ȃ�����
		if (is_set_black)
		{
			sprite_render("frame", sprite_back.get(), game_over_pram, 0, glow_vertical);

			fonts->yu_gothic->Begin(graphics.get_dc().Get());
			r_font_render(game_over_text);
			r_font_render(back_title);
			r_font_render(again);
			fonts->yu_gothic->End(graphics.get_dc().Get());
			r_sprite_render(sprite_selecter.get(), selecter1);
			r_sprite_render(sprite_selecter.get(), selecter2);
		}
	}

	//-----�I�v�V�������J���Ă鎞�ɃI�v�V������`��-----//
	if (option->get_validity()) { option->render(graphics, elapsed_time); }
}

void SceneMultiGameHost::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
	return;
}

void SceneMultiGameHost::GameOverAct(float elapsed_time)
{
	if (is_game_over)
	{
		//�����Ń^�C�g���ɖ߂�̈ʒu�����߂Ă���̂�
		//�Q�[���N���A�̎��ɂ��������̂��g������
		back_title.position = { 365.6f,236.7f };
		back_title.scale = { 1.0f,1.0f };

		//��ʃ����X�ɍ�������
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
				//�X�v���C�g��texpos���Ō�܂ōs������
				if (game_over_sprite_pram.texpos.x >= 1536.0f
					&& game_over_sprite_pram.texpos.y >= 512.0f)
				{
					audio_manager->play_se(SE_INDEX::DRAW_PEN);
					is_game_over_sprite = true;
				}
				//�X�v���C�g��texpos.y��0�Ȃ烊�Z�b�g
				if (game_over_sprite_pram.texpos.x > 1536.0f
					&& game_over_sprite_pram.texpos.y < 512.0f)
				{
					audio_manager->play_se(SE_INDEX::DRAW_PEN);
					game_over_sprite_pram.texpos.x = 0.0f;
					game_over_sprite_pram.texpos.y += 512.0f;
				}

			}
			if (is_game_over_sprite)brack_back_pram.color.w += 1.0f * elapsed_time;
			//�Z���N�^�[�̏�����
			selecter1.position = { 328.0f,267.3f };
			selecter2.position = { 637.1f,267.3f };
		}
		// �X�e�B�b�N���X���������班���Ԃ������ē��͂�������
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
		//��ʂ������Ȃ��Ă��炵�������Ȃ��悤��
		if (is_set_black)
		{
			switch (game_over_state)
			{
			case 0://�^�C�g���ɖ߂�
				r_right_tutorial(1, { 710.5f,267.3f }, { 911.8f,267.3f });
				if (game_pad->get_button_down() & GamePad::BTN_B)
				{
					audio_manager->play_se(SE_INDEX::DECISION);
					SceneManager::scene_switching(new SceneLoading(new SceneTitle()), DISSOLVE_TYPE::TYPE1, 2.0f);
				}
				break;
			case 1://�Ē���
				r_left_tutorial(0, { 328.0f,267.3f }, { 637.1f,267.3f });
				if (game_pad->get_button_down() & GamePad::BTN_B)
				{
					audio_manager->play_se(SE_INDEX::DECISION);
					SceneManager::scene_switching(new SceneLoading(new SceneMultiGameHost()), DISSOLVE_TYPE::TYPE1, 2.0f);
				}
				break;
			default:
				break;
			}
		}
	}
}

void SceneMultiGameHost::GameClearAct(float elapsed_time, GraphicsPipeline& graphics)
{
	if (is_game_clear)
	{
		//�v���C���[�̃N���A�p�̍X�V����
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
		//�����Ń^�C�g���ɖ߂�̈ʒu�����߂Ă���̂�
	   //�Q�[���I�[�o�[�̎��ɂ��������̂��g������
		back_title.position = { 529.7f,246.3f };
		back_title.scale = { 1.0f,1.0f };
		//�Z���N�^�[�̈ʒu������
		selecter1.position = { 495.3f,277.1f };
		selecter2.position = { 801.5f,277.1f };
		//�v���C���[�̃N���A���[�V�������I����Ă��炵�������Ȃ��悤�ɂ���
		if (player_manager->GetMyTerminalEndClearMotion())
		{
			//��ʂ����X�ɍ�������
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

void SceneMultiGameHost::JudgeSlow(float& elapsed_time)
{
	//�v���C���[���W���X�g���������slow
	if (player_manager->GetIsJustAvoidance())
	{
		slow = true;
	}
	else
	{
		slow_timer = 0.0f;
		slow = false;
	}
	//slow��true�Ȃ�
	if (slow)
	{
		slow_timer += 1.0f * elapsed_time;
		//�^�C�}�[��1.0�b�ȉ��Ȃ�x������
		if (slow_timer < 1.0f)
		{
			elapsed_time *= slow_rate;
		}
	}
}

void SceneMultiGameHost::SetBossTypeGameParam()
{
	//-----�J�����̃C���X�^���X���擾-----//
	Camera* c = cameraManager->GetCurrentCamera();

	//-----�{�X���l�^�ɂȂ鎞-----//
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::ShipToHuman)
	{
		c->boss_animation = true;
		player_manager->SetPlayerPosition({ 0,0,-120.0f });
		//�v���C���[�̍s���͈͕ύX
		player_manager->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_HUMANOID);
		purple_threshold = 0.01f;
	}

	//-----�{�X���h���S���ɂȂ鎞-----//
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::HumanToDragon)
	{
		c->boss_animation = true;
		player_manager->SetPlayerPosition({ 0,0,-120.0f });
		//�v���C���[�̍s���͈͕ύX
		player_manager->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_DRAGON);
		red_threshold = 0.01f;
	}

	//-----�ʏ펞-----//
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::ShipAppear)
	{
		c->boss_animation = true;
		player_manager->SetPlayerPosition({ 0,0,-120.0f });
		//�v���C���[�̍s���͈͕ύX.
		player_manager->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_BATTLESHIP);
	}

	//-----�Ē��펞-----//
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

void SceneMultiGameHost::BossEventCamera()
{
	//-----�G�̃C���X�^���X�𐶐�-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	//--------------------<�{�X�̕��ɃJ�����������鏈��>--------------------//
	if (enemyManager->fGetIsEventCamera() && !mIsBossCamera)
	{
		post_effect->clear_post_effect();
		// �܂��{�X�̂ق��������Ă��Ȃ��Ƃ�
		mIsBossCamera = true;
		cameraManager->SetCamera(static_cast<int>(CameraTypes::Joint));
	}

	//-----�{�X�̃C�x���g�J�����ɂȂ�-----//
	if (enemyManager->fGetIsEventCamera() && mIsBossCamera)
	{
		const DirectX::XMFLOAT3 eye = enemyManager->fGetEye();
		const DirectX::XMFLOAT3 focus = enemyManager->fGetFocus();

		// �J������EnemyManager���o�R��555���{�X�ɂ���čX�V�����
		cameraManager->GetCurrentCamera()->set_eye(eye);
		cameraManager->GetCurrentCamera()->set_target(focus);

		// �G�̈ړ��𐧌�
		enemyManager->fLimitEnemies();
	}

	//-----�C�x���g�J�����I��-----//
	if (!enemyManager->fGetIsEventCamera() && mIsBossCamera)
	{
		// �J���������I��
		mIsBossCamera = false;
		cameraManager->SetCamera(static_cast<int>(CameraTypes::Game));
	}
}

void SceneMultiGameHost::SetSkyDomeColor(float elapsed_time)
{
	//-----SkyDome�̐F-----//
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

void SceneMultiGameHost::PlayerManagerUpdate(GraphicsPipeline& graphics, float elapsed_time)
{
	//-----�G�̃C���X�^���X�𐶐�-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	//-----�v���C���[�Ɉ�ԋ߂��G���擾-----//
	BaseEnemy* enemy = enemyManager->fGetNearestEnemyPosition();

	//-----�J�����̃C���X�^���X���擾-----//
	Camera* c = cameraManager->GetCurrentCamera();

	//-----�{�X�p�̃J�����ɐݒ肷��-----//
	player_manager->SetBossCamera(mIsBossCamera);

	//-----�J�����̕�����ݒ肷��-----//
	player_manager->SetCameraDirection(c->GetForward(), c->GetRight());

	//-----�J������view�s��ƃv���W�F�N�V�����s���ݒ肷��-----//
	player_manager->SetCameraView(c->get_keep_view());
	player_manager->SetCameraProjection(c->get_keep_projection());

	//-----�X�V����-----//
	player_manager->Update(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());

	//-----���b�N�I���̃|�X�g�G�t�F�N�g��������-----//
	player_manager->LockOnPostEffect(elapsed_time, post_effect.get());

	//-----�J�����̈ʒu��ݒ肷��-----//
	player_manager->SetCameraPosition(c->get_eye());

	//-----�v���C���[�Ɉ�ԋ߂��G��ݒ肷��-----//
	player_manager->SetTarget(enemy);

	//-----�v���C���[�ɃJ�����̃^�[�Q�b�g��ݒ肷��-----//
	player_manager->SetCameraTarget(c->get_target());


}

void SceneMultiGameHost::PlayerManagerCollision(GraphicsPipeline& graphics, float elapsed_time)
{
	//-----�e�̃C���X�^���X�𐶐�-----//
	BulletManager& mBulletManager = BulletManager::Instance();

	//-----�G�̃C���X�^���X�𐶐�-----//
	const auto enemyManager = mWaveManager.fGetEnemyManager();

	//-----�N���C�A���g�����b�N�I�����Ă���G��ݒ肷��-----//
	player_manager->SearchClientPlayerLockOnEnemy(enemyManager);

	//-----�G�Ƃ̂����蔻��(����������R���{���Z)-----//
	player_manager->PlayerAttackVsEnemy(enemyManager, graphics, elapsed_time);

	//-----�W���X�g������\���ǂ����̓����蔻��-----//
	player_manager->PlayerCounterVsEnemyAttack(enemyManager);

	//-----�v���C���[�̗̑͂̓������Ƃ�-----//
	ReceivePlayerHealthData();

	//-----�G�̍U���ƃv���C���[�̓����蔻��-----//
	player_manager->EnemyAttackVsPlayer(enemyManager);

	//-----�v���C���[���W���X�g����������͈̔̓X�^���̓����蔻��-----//
	player_manager->PlayerStunVsEnemy(enemyManager);

	//-----�v���C���[���`�F�C����Ԃł��邱�Ƃ�G�ɒm�点�čs�����~������-----//
	player_manager->SetPlayerChainTime(enemyManager);

	//-----�e�ƃv���C���[�̓����蔻��-----//
	player_manager->BulletVsPlayer(mBulletManager);
}

void SceneMultiGameHost::StartGameUi(float elapsed_time)
{
	//�o�b�N�{�^���𒷉�������3�b��������Q�[���X�^�[�g
	if (game_pad->get_button() & GamePad::BTN_BACK)
	{
		game_start_timer += 1.0f * elapsed_time;
		game_start_gauge_parm.threshold -= (2.0f * elapsed_time) / 2.0f;
		if (game_start_timer > 1.4f)
		{
			is_start_game = true;

			//-----�p�����[�^��������-----//
			game_start_gauge_parm.threshold = 1.0f;
			game_start_timer = 0;

			//-----���̃X�e�[�W�𑗐M����-----//

			char data[2]{};
			data[0] = CommandList::CurrentStageNum;
			data[1] = current_stage;
			//-----�X�e�[�W�N���A�𑗐M-----//
			CorrespondenceManager::Instance().TcpSendAllClient((char*)&data, 2);
		}
	}
	else
	{
		game_start_gauge_parm.threshold = 1.0f;
		game_start_timer = 0;
	}

}

void SceneMultiGameHost::RegisterPlayer(GraphicsPipeline& graphics)
{
	//-----	�r������-----//
	std::lock_guard<std::mutex> lock(mutex);

	//�N���C�A���g�����O�C�����ė�����
	if (register_player && register_player_id >= 0)
	{
		ClientPlayer* player = new ClientPlayer(graphics, register_player_id);
		player->SetName(CorrespondenceManager::Instance().names[register_player_id]);
		player->SetColor(static_cast<BasePlayer::PlayerColor>(register_player_color));
		player_manager->RegisterPlayer(player);

		//�v���C���[��ǉ�
		std::string text = std::to_string(register_player_id) + "�ԖڂɃv���C���[��ǉ����܂���";
		DebugConsole::Instance().WriteDebugConsole(text, TextColor::Green);
		//�ǉ����鎞�ɕK�v�ȃp�����[�^�ƃt���O�̏�����
		register_player_id = -1;
		register_player = false;
		register_player_color = 0;

		if (CorrespondenceManager::Instance().GetHost())
		{
			//-----�v���C���[�̗̑͂𑝂₷-----//
			player_manager->AddPlayerMultiHealth();

			//-----�̗͂𑗐M����-----//
			player_manager->SendPlayerHealthData();

			//----�ڑ��Ґ��𑝂₷-----//
			CorrespondenceManager::Instance().AddConnectedPersons();
		}
	}
}

void SceneMultiGameHost::DeletePlayer()
{

	//-----�z�񂪋�Ȃ珈�������Ȃ�-----//
	if (logout_id.empty()) return;

	//-----�r������-----//
	SocketCommunicationManager& instance = SocketCommunicationManager::Instance();
	std::lock_guard<std::mutex> lock(CorrespondenceManager::Instance().GetMutex());
	std::lock_guard<std::mutex> lock2(mutex);
	std::lock_guard<std::mutex> lock3(instance.GetMutex());

	//-----���O�A�E�g�����v���C���[���폜����-----//
	for (auto id : logout_id)
	{
		//-----�ڑ��҂̔ԍ������Z�b�g-----//
		CorrespondenceManager::Instance().SetOpponentPlayerId(id, -1);

		//-----�A�h���X���폜-----//
	    instance.game_udp_server_addr[id].sin_addr.S_un.S_addr = 0;

		//-----FD����폜����-----//
		FD_CLR(instance.login_client_sock[id], &instance.client_tcp_fds);

		//-----�z�X�g��tcp�ʐM�p�̃\�P�b�g�폜-----//
		closesocket(instance.login_client_sock[id]);
		instance.login_client_sock[id] = INVALID_SOCKET;

		//-----�ڑ��������炷-----//
		instance.client_tcp_fds_count--;

		//-----�v���C���[�̍폜-----//
		player_manager->DeletePlayer(id);

		if (CorrespondenceManager::Instance().GetHost())
		{
			//-----�v���C���[�̗̑͂����炷-----//
			player_manager->SubPlayerMultiHealth();

			//-----�̗͂𑗐M����-----//
			player_manager->SendPlayerHealthData();

			//-----�ڑ��Ґ������炷-----//
			CorrespondenceManager::Instance().SubConnectedPersons();
		}
	}

	//-----���O�A�E�g�f�[�^���폜����-----//
	logout_id.clear();

}

void SceneMultiGameHost::SetReceiveData()
{
	//-----�v���C���[�̓����f�[�^�������Ă���ꍇ-----//
	if (receive_all_data.player_move_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----�f�[�^��ݒ肷��-----//
		for (const auto& p_data : receive_all_data.player_move_data)
		{
			player_manager->SetPlayerMoveData(p_data);
		}

		//-----�f�[�^���폜����-----//
		receive_all_data.player_move_data.clear();
	}

	//-----�v���C���[�̈ʒu�f�[�^�������Ă���ꍇ-----//
	if (receive_all_data.player_position_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----�f�[�^��ݒ肷��-----//
		for (const auto& p_data : receive_all_data.player_position_data)
		{
			player_manager->SetPlayerPositionData(p_data);
		}

		//-----�f�[�^���폜����-----//
		receive_all_data.player_position_data.clear();
	}

	//-----�v���C���[�̃A�N�V�����f�[�^�������Ă���ꍇ-----//
	if (receive_all_data.player_action_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----�f�[�^��ݒ肷��-----//
		for (const auto& p_data : receive_all_data.player_action_data)
		{
			player_manager->SetPlayerActionData(p_data);
		}

		//-----�f�[�^���폜����-----//
		receive_all_data.player_action_data.clear();
	}

}

void SceneMultiGameHost::SetEnemyDamageData(GraphicsPipeline& graphics_)
{
	//-----�G�̃_���[�W�f�[�^��ݒ肷��-----//
	if (receive_all_enemy_data.enemy_damage_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);

		//-----�f�[�^��ݒ肷��-----//
		for (const auto& data : receive_all_enemy_data.enemy_damage_data)
		{
			mWaveManager.fSetReceiveEnemyDamageData(data,graphics_);
		}

		//-----�f�[�^���폜����-----//
		receive_all_enemy_data.enemy_damage_data.clear();
	}
}

void SceneMultiGameHost::SetEnemyConditionData()
{
	//-----�G�̃_���[�W�f�[�^��ݒ肷��-----//
	if (receive_all_enemy_data.enemy_condition_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);

		//-----�f�[�^��ݒ肷��-----//
		for (const auto& data : receive_all_enemy_data.enemy_condition_data)
		{
			mWaveManager.fSetReceiveEnemyConditionData(data);
		}

		//-----�f�[�^���폜����-----//
		receive_all_enemy_data.enemy_condition_data.clear();
	}

}

void SceneMultiGameHost::ReceivePlayerHealthData()
{
	if (receive_all_data.player_health_data.empty() == false)
	{
		std::lock_guard<std::mutex> lock(mutex);
		//-----�f�[�^��ݒ肷��-----//
		for (const auto& p_data : receive_all_data.player_health_data)
		{
			player_manager->ReceivePlayerHealthData(p_data);
		}

		//-----�f�[�^���폜����-----//
		receive_all_data.player_health_data.clear();

		//-----������������̗͂𑗐M����-----//
		player_manager->SendPlayerHealthData();
	}


}

void SceneMultiGameHost::ClearEnemyReceiveData()
{
	//-----�G�̏o���f�[�^���폜����-----//
	receive_all_enemy_data.enemy_spawn_data.clear();

	//-----�G�̓����f�[�^���폜����-----//
	receive_all_enemy_data.enemy_move_data.clear();

	//-----�_���[�W�f�[�^�폜-----//
	receive_all_enemy_data.enemy_damage_data.clear();

	//-----���S�f�[�^�폜-----//
	receive_all_enemy_data.enemy_die_data.clear();

	//-----��ԃf�[�^�폜-----//
	receive_all_enemy_data.enemy_condition_data.clear();



}

void SceneMultiGameHost::TransferEnenyControlProcessing()
{
	if (transfer_enemy_host_request)
	{
		//-----�G�̃z�X�g�����擾���Ă�������nOK-----//
		if (mWaveManager.GetHost())
		{
			char data[2]{};

			data[ComLocation::ComList] = CommandList::TransferEnemyControlResult;
			data[TransferEnemyControl::DataArray::Result] = TransferEnemyControl::Result::Permit;

			CorrespondenceManager::Instance().TcpSend(transfer_enemy_request_id,data, sizeof(data));
			DebugConsole::Instance().WriteDebugConsole("�����M",TextColor::Green);

			//-----�z�X�g���𖳂���-----//
			mWaveManager.SetHost(false);


		}
		//-----�G�̃z�X�g���������Ă��Ȃ���������nNG-----//
		else
		{
			char data[2]{};

			data[ComLocation::ComList] = CommandList::TransferEnemyControlResult;
			data[TransferEnemyControl::DataArray::Result] = TransferEnemyControl::Result::Prohibition;

			CorrespondenceManager::Instance().TcpSend(transfer_enemy_request_id,data, sizeof(data));
			DebugConsole::Instance().WriteDebugConsole("�֎~���M", TextColor::Red);
		}

		transfer_enemy_request_id = -1;
		transfer_enemy_host_request = false;
	}
}

void SceneMultiGameHost::ReturnEnemyControl()
{
	if (return_enemy_control)
	{
		mWaveManager.SetHost(true);
		return_enemy_control = false;
	}
}
