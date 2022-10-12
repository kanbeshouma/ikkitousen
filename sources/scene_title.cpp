#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include <thread>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include "framework.h"
#include "scene_title.h"
#include "scene_game.h"
#include"SceneMultiGameHost.h"
#include "SceneMultiGameClient.h"
#include "scene_loading.h"
#include "scene_manager.h"
#include "ModelCashe.h"
#include"SceneTutorial.h"
#include "WaveManager.h"
#include "LastBoss.h"
#include"SocketCommunication.h"
#include"Correspondence.h"

bool SceneTitle::is_load_ready = false;

//-----���O�C�����������������ꍇ�̃V�[���ؑւ����Ă������ǂ���----//
bool SceneTitle::change_scene_thread = false;

//-----�}�b�`���O���n�܂������ǂ���-----//
bool SceneTitle::start_matching = false;

//-----�}�b�`���O�ҋ@����-----//
float SceneTitle::standby_matching_timer = 0.0f;

//-----�u���b�L���O-----//
std::mutex SceneTitle::mutex;

void SceneTitle::initialize(GraphicsPipeline& graphics)
{
	//--�^�C�g���O���[�h--//
	ModelCashes::Load_PreTitle(graphics.get_device().Get());

	//----<3D�֘A>----//
	//�X�e�[�W�I�u�W�F�N�g
	title_stage_model = resource_manager->load_model_resource(graphics.get_device().Get(), ".\\resources\\Models\\stage\\title_stage.fbx", false, 60.0f);
	// player
	player = std::make_unique<Player>(graphics);
	player->TransitionTitleAnimationReadyIdle();
	// cameraManager
	cameraManager = std::make_unique<CameraManager>();
	cameraManager->RegisterCamera(new TitleCamera(player.get()));
	cameraManager->RegisterCamera(new JointCamera(graphics));

	cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Title));
	// shadow_map
	shadow_map = std::make_unique<ShadowMap>(graphics);
	// post_effect
	post_effect = std::make_unique<PostEffect>(graphics.get_device().Get());
	// bloom_effect
	bloom_effect = std::make_unique<Bloom>(graphics.get_device().Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
	bloom_constants = std::make_unique<Constants<BloomConstants>>(graphics.get_device().Get());
	// sky_dome
	sky_dome = std::make_unique<SkyDome>(graphics);

	// pointlight
	point_lights = std::make_unique<PointLights>(graphics);
	// effect
	fire_effect1 = std::make_unique<Effect>(graphics, effect_manager->get_effekseer_manager(), ".\\resources\\Effect\\fire.efk");
	fire_effect2 = std::make_unique<Effect>(graphics, effect_manager->get_effekseer_manager(), ".\\resources\\Effect\\fire.efk");
	fire_effect1->play(effect_manager->get_effekseer_manager(), fire_pos_1, 0.45f);
	fire_effect2->play(effect_manager->get_effekseer_manager(), fire_pos_2, 0.45f);

	//-------<2D�p�[�g>--------//
	// has_stageNo_json
	{
#if 0
		std::filesystem::path path = "./resources/Data/stage_to_start.json";
		if (std::filesystem::exists(path.c_str())) /*�E�F�[�u�f�[�^����*/ { has_stageNo_json = true; }
		else /*�E�F�[�u�f�[�^�Ȃ�*/ { has_stageNo_json = false; }
#else
		has_stageNo_json = false;
#endif
	}

	//--flash--//
	sprite_flash = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\title_slash.png", 1);
	flash.position = { 1280,360 };
	flash.scale    = { 1,1 };
	flash.texsize  = { static_cast<float>(sprite_flash->get_texture2d_desc().Width), static_cast<float>(sprite_flash->get_texture2d_desc().Height) };
	flash.color    = { 1,1,1,1 };
	//--selecter--//
	sprite_selecter    = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\selecter.png", 2);
	selecter1.texsize  = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
	//if (has_stageNo_json) selecter1.position = { 990.0f, 545.0f };
	//else selecter1.position = { 990.0f, 565.0f };
    selecter1.position = { 1001.0f,516.3f };
	selecter1.scale    = { 0.2f, 0.1f };

	selecter2.texsize  = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
	//if (has_stageNo_json) selecter2.position = { 1167.0f, 545.0f };
	//else selecter2.position = { 1167.0f, 565.0f };
	selecter2.position = { 1155.0f,516.3f };
	selecter2.scale    = { 0.2f, 0.1f };

	arrival_pos1 = selecter1.position;
	arrival_pos2 = selecter2.position;

	//--font--//
	beginning.s = L"���߂���";
	if (has_stageNo_json) beginning.position = { 1032, 495.0f };
	else beginning.position = { 1032, 495.0f };
	beginning.scale = { 0.7f,0.7f };

	multiplay.s = L"�}���`�v���C";
	multiplay.position = { 1019.0f, 550.0f };
	multiplay.scale = { 0.7f,0.7f };

	succession.s = L"��������";
	succession.position = { 1035, 575 };
	succession.scale = { 0.7f,0.7f };

	exit.s = L"�Q�[���I��";
	if (has_stageNo_json) exit.position = { 1025, 630 };
	else exit.position = { 1025, 605 };
	exit.scale = { 0.7f,0.7f };


	host_play_font.s = L"�z�X�g�v���C";
	host_play_font.position = { 1020.0f,500.0f };
	host_play_font.scale = { 0.7f,0.7f };

	client_play_font.s = L"�Q�[���ɎQ������";
	client_play_font.position = { 987.0f,557.0f };
	client_play_font.scale = { 0.7f,0.7f };

	multi_paly_entry_back.s = L"�߂�";
	multi_paly_entry_back.position = { 1066.0f,618.0f };
	multi_paly_entry_back.scale = { 0.7f,0.7f };



	now_loading.position = { 15,675 };
	now_loading.scale = { 0.6f,0.6f };

	//-----�}�b�`���O��-----//
	now_matching.position = { 15,675 };
	now_matching.scale = { 0.6f,0.6f };

	//--state--//
	state = TitleEntry::Beginning;

#if 0
	std::filesystem::path path = "./resources/Data/tutorial.json";
	if (std::filesystem::exists(path.c_str())) /*�`���[�g���A���f�[�^����*/ { have_tutorial_state = 0; }
	else /*�`���[�g���A���f�[�^�Ȃ�*/ { have_tutorial_state = -1; }
#else
	have_tutorial_state = -1;
#endif

	//--tutorial tab--//
	{
		tutorial_tab.display = false;

		tutorial_tab.selecterL.texsize = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
		tutorial_tab.selecterL.position = { 587, 335 };
		tutorial_tab.selecterL.scale = { 0.2f,0.1f };

		tutorial_tab.selecterR.texsize = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
		tutorial_tab.selecterR.position = { 692, 335 };
		tutorial_tab.selecterR.scale = { 0.2f,0.1f };

		tutorial_tab.arrival_posL = tutorial_tab.selecterL.position;
		tutorial_tab.arrival_posR = tutorial_tab.selecterR.position;

		tutorial_tab.headline.s = L"�`���[�g���A�����s���܂����H";
		tutorial_tab.headline.position = { 465, 235 };
		tutorial_tab.headline.scale = { 0.7f,0.7f };

		tutorial_tab.back.s = L"�߂�";
		tutorial_tab.back.position = { 790, 520 };
		tutorial_tab.back.scale = { 0.6f,0.6f };

		tutorial_tab.yes.s = L"�͂�";
		tutorial_tab.yes.position = { 625, 315 };
		tutorial_tab.yes.scale = { 0.7f,0.7f };

		tutorial_tab.no.s = L"������";
		tutorial_tab.no.position = { 615, 370 };
		tutorial_tab.no.scale = { 0.7f,0.7f };

		tutorial_tab.sprite_frame = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\title_back.png", 1);
		tutorial_tab.frame.texsize = { static_cast<float>(tutorial_tab.sprite_frame->get_texture2d_desc().Width),
										static_cast<float>(tutorial_tab.sprite_frame->get_texture2d_desc().Height) };
		tutorial_tab.frame.position = { 333.0f, 33.5f };
		tutorial_tab.frame.scale = { 1.0f, 1.0f };
	}

	for (auto& play : can_play_se)
	{
		play = false;
	}

	//--logo_parameters--//
	{
		logo_parameters.sprite_logo = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\title_logo.png", 1);
		logo_parameters.logo.texsize = { static_cast<float>(logo_parameters.sprite_logo->get_texture2d_desc().Width),
										 static_cast<float>(logo_parameters.sprite_logo->get_texture2d_desc().Height) };
		logo_parameters.logo.position = { 100.0f, -3.0f };
		logo_parameters.logo.scale = { 1.0f, 1.0f };

		logo_parameters.sprite_animation = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\title_anime.png", 1);
		logo_parameters.animation.texsize = logo_parameters.logo.texsize;
		logo_parameters.animation.position = logo_parameters.logo.position;
		logo_parameters.animation.scale = logo_parameters.logo.scale;

		logo_parameters.timer = 0;
		logo_parameters.frame_y = 0;
		logo_parameters.start_anim = false;
		logo_parameters.reset_timer = 0.0f;
		logo_parameters.glow_horizon = 0.0f;
	}
	//--slashing post effect--//
	{
		slashing_power = 0;
		slashing_wait_timer = 0;
	}
	// config
	{
		sprite_config_b        = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\b.png", 1);

		config_b.position      = { 795,525 };
		config_b.scale         = { 0.6f,0.6f };
		config_b.texsize       = { static_cast<float>(sprite_config_b->get_texture2d_desc().Width), static_cast<float>(sprite_config_b->get_texture2d_desc().Height) };
		config_b.color         = { 1,1,1,1 };

		config_b2.position      = { 370,320 };
		config_b2.scale         = { 0.6f,0.6f };
		config_b2.texsize       = { static_cast<float>(sprite_config_b->get_texture2d_desc().Width), static_cast<float>(sprite_config_b->get_texture2d_desc().Height) };
		config_b2.color         = { 1,1,1,1 };

		config_b_font.s        = L"����";
		config_b_font.position = { 1200, 675 };
		config_b_font.scale    = { 0.6f,0.6f };

		config_b_font2.s        = L"����";
		config_b_font2.position = { 785, 470 };
		config_b_font2.scale    = { 0.6f,0.6f };


		sprite_config_a = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\a.png", 1);

		config_a.position = { 393,345 };
		config_a.scale = { 0.6f,0.6f };
		config_a.texsize = { static_cast<float>(sprite_config_a->get_texture2d_desc().Width), static_cast<float>(sprite_config_a->get_texture2d_desc().Height) };
		config_a.color = { 1,1,1,1 };
	}

	//--audio--//
	audio_manager->stop_all_bgm();
	audio_manager->play_bgm(BGM_INDEX::TITLE);

	audio_manager->set_volume_se(SE_INDEX::OPEN_FIRE, 0.1f * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_se_volume());
	audio_manager->play_se(SE_INDEX::OPEN_FIRE);

	//�X���b�h�J�n
	std::thread thread(loading_thread, graphics.get_device().Get());
	//�X���b�h�̊Ǘ������
	thread.detach();

	//-----�l�b�g���[�N�֌W�̕ϐ�������������-----//
	SocketCommunicationManager::Instance().ClearData();
	//������IPAddress���擾
	CorrespondenceManager::Instance().AcquisitionMyIpAddress();

	//-----�}���`�X���b�h�ϐ��̏�����-----//
	standby_matching_timer = 0.0f;
	change_scene_thread = false;
	start_matching = false;
	// �{�X�̏�Ԃ����Z�b�g
	LastBoss::fLoadParam();
	LastBoss::fResetLoadRaram();
	LastBoss::fSaveParam();
}

void SceneTitle::uninitialize()
{
	post_effect->clear_post_effect();
	audio_manager->stop_all_se();
	fire_effect1->stop(effect_manager->get_effekseer_manager());
	fire_effect2->stop(effect_manager->get_effekseer_manager());
}

void SceneTitle::effect_liberation(GraphicsPipeline& graphics)
{
	effect_manager->finalize();
	effect_manager->initialize(graphics);
}

void SceneTitle::update(GraphicsPipeline& graphics, float elapsed_time)
{

	//-----�I�[�f�B�I�̐ݒ�-----//
	audio_manager->set_all_volume_bgm(BGM_VOLUME * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_bgm_volume());
	audio_manager->set_all_volume_se(SE_VOLUME * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_se_volume());

	//-----���S�̃A�j���[�V����-----//
	LogoAnimation(elapsed_time);

#ifdef Telecommunications
	ImGui::Begin("host_ip_adress");
	ImGui::InputText("IP Adress", SocketCommunicationManager::Instance().host_ip, sizeof(SocketCommunicationManager::Instance().host_ip), ImGuiInputTextFlags_CharsDecimal);
	ImGui::InputText("Port", CorrespondenceManager::Instance().udp_port, sizeof(CorrespondenceManager::Instance().udp_port), ImGuiInputTextFlags_CharsDecimal);
	ImGui::Checkbox("re_name", &re_name);
	if (re_name)
	{
		ImGui::InputText("name", CorrespondenceManager::Instance().my_name, sizeof(CorrespondenceManager::Instance().my_name));
	}
	ImGui::Text("standby_matching_timer%f", standby_matching_timer);
	ImGui::Text("state%d", state);
	ImGui::Text("multi_play_entry_state%d", multi_play_entry_state);
	ImGui::End();
#endif // Telecommunications

	//----<3D�֘A>----//
	// cameraManager
	Camera* c = cameraManager->GetCurrentCamera();
	cameraManager->Update(elapsed_time);
	// shadow_map
	shadow_map->debug_imgui();
	// player
	player->SetCameraDirection(c->GetForward(), c->GetRight());
	player->SetCameraPosition(c->get_eye());
	player->UpdateTitle(elapsed_time);


	//-----�^�C�g���̍��ڑI��-----//
	if(re_name == false)TitleSelectEntry(elapsed_time);


	static float speed = 15000.0f;
#ifdef USE_IMGUI
	ImGui::Begin("slashing");
	ImGui::Text("slashing_wait_timer : %f", slashing_wait_timer);
	ImGui::DragFloat("slashing_power", &slashing_power, 0.01f);
	ImGui::DragFloat("speed", &speed, 0.1f);
	ImGui::End();
#endif // USE_IMGUI

	// joint camera test
	// JointCamera �Ƀ`�F���W�����t���[���Ɠ����t���[������ JointCamera ��������eye��focus���W���C���g������o���ăZ�b�g���Ăق���
	// cameraManager->GetCurrentCamera()->set_eye(eye);�@cameraManager->GetCurrentCamera()->set_target(focus);�@������̂�
	// JointCamera�̎������ɂ��Ȃ��ƃo�O��̂ł���p��bool�^�ϐ��p�ӂ��Ăق���(validity_joint_camera) false�ɂ���̖Y�ꂸ��!!
	// ����Imgui�Q�l�ɂ��Ă�������(static ���[�J���̓����o�ϐ���)
	// JointCamera�������@�̓w�b�_�[�� enum class CameraTypes �A��̃R���X�g���N�^�Q��(Register���鏇��enum class�Ɠ�����)
	// JointCamera��player��nullptr����Ă�̂�JointCamera�Ƀ`�F���W�����u��player��nullptr�ŗ�����\������
	// JointCamera���I������猳�̃J�����ɖ߂��̂��Y�ꂸ�ɂ��肢���܂�
	static bool validity_joint_camera = false;
	static DirectX::XMFLOAT3 eye{ 0,1,-30 };
	static DirectX::XMFLOAT3 focus{};
#ifdef USE_IMGUI
	ImGui::Begin("joint camera");
	if (ImGui::Button("change joint camera"))
	{
		cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Joint));
		validity_joint_camera = true;
	}
	if (ImGui::Button("change title camera"))
	{
		cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Title));
		validity_joint_camera = false;
	}
	ImGui::DragFloat3("eye", &eye.x, 0.1f);
	ImGui::DragFloat3("focus", &focus.x, 0.1f);
	ImGui::End();
	if (validity_joint_camera)
	{
		cameraManager->GetCurrentCamera()->set_eye(eye);
		cameraManager->GetCurrentCamera()->set_target(focus);
	}
#endif // USE_IMGUI


	// SE
	if (!can_play_se[0] && slashing_wait_timer > 0.7f)
	{
		audio_manager->play_se(SE_INDEX::DRAW_SWORD);
		can_play_se[0] = true;
	}
	if (!can_play_se[1] && slashing_wait_timer > 2.0f)
	{
		audio_manager->play_se(SE_INDEX::RETURN_SWORD);
		can_play_se[1] = true;
	}
	if (!can_play_se[2] && slashing_wait_timer > 4.3f)
	{
		audio_manager->play_se(SE_INDEX::ATTACK_SWORD);
		can_play_se[2] = true;
	}
	// �M��������
	if (slashing_wait_timer > 4.3f)
	{
		flash.position.x -= speed * elapsed_time;
		flash.position.x = (std::max)(flash.position.x, -1700.0f);
	}

	if (player->GetEndTitleAnimation())
	{
		// �܂��͏������炷
		if (slashing_power <= 0 && slashing_wait_timer > 5.0f)
		{
			slashing_power = 0.015f;
			audio_manager->play_se(SE_INDEX::TITLE_CUT);
		}
		// ���������炷
		if (slashing_wait_timer > 6.0f)
		{
			if (Math::equal_check(slashing_power, 0.015f, 0.001f))
			{
				audio_manager->play_se(SE_INDEX::TITLE_CUTTING);
			}
			slashing_power += elapsed_time * 0.05f;
			slashing_power = (std::min)(slashing_power, SLASHING_MAX);
		}
	}

	// ��ʑJ��
	if (Math::equal_check(slashing_power, SLASHING_MAX))
	{
		if (have_tutorial_state == 1) /* �`���[�g���A���Ȃ� */
		{
			if (select_mulch_play)
			{
				switch (multi_play_entry_state)
				{
				case MultiPlayEntry::Host:
					SceneManager::scene_switching(new SceneLoading(new SceneMultiGameHost()), DISSOLVE_TYPE::HORIZON, 2.0f);
					DebugConsole::Instance().WriteDebugConsole("�z�X�g�v���C�J�n");
					CorrespondenceManager::Instance().SetMultiPlay(true);
					break;
				case MultiPlayEntry::Client:
					SceneManager::scene_switching(new SceneLoading(new SceneMultiGameClient()), DISSOLVE_TYPE::HORIZON, 2.0f);
					DebugConsole::Instance().WriteDebugConsole("�Q�[���ɎQ��");
					CorrespondenceManager::Instance().SetMultiPlay(true);
					break;
				default:
					break;
				}
			}
			else
			{
				SceneManager::scene_switching(new SceneLoading(new SceneGame()), DISSOLVE_TYPE::HORIZON, 2.0f);
				CorrespondenceManager::Instance().SetMultiPlay(false);
			}
		}
		else
		{
			SceneManager::scene_switching(new SceneLoading(new TutorialScene()), DISSOLVE_TYPE::HORIZON, 2.0f);
			CorrespondenceManager::Instance().SetMultiPlay(false);
		}

		point_lights->finalize(graphics);
		is_load_ready = false;
	}


	effect_manager->update(elapsed_time);

	post_effect->title_post_effect(slashing_power);
}

void SceneTitle::render(GraphicsPipeline& graphics, float elapsed_time)
{
	// post_effect
	post_effect->begin(graphics.get_dc().Get());

	// �V���h�E�}�b�v�̃Z�b�g
	shadow_map->set_shadowmap(graphics);

	// �J�����̃r���[�s��v�Z
	cameraManager->CalcViewProjection(graphics);


	/*-----!!!���������ɃI�u�W�F�N�g�̕`��͂��Ȃ���!!!!-----*/
	//-------<3D�p�[�g>--------//
	// sky_dome
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::PBR);
	sky_dome->Render(graphics, elapsed_time);
	//�^�C�g���I�u�W�F�N�g
	graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::PBR);
	title_stage_model->render(graphics.get_dc().Get(), Math::calc_world_matrix(title_stage_parm.scale, title_stage_parm.angle, title_stage_parm.pos), { 1.0f,1.0f,1.0f,1.0f });
	// player
	player->TitleRender(graphics, elapsed_time);

	// point_lights
	point_lights->render(graphics, elapsed_time);

	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	effect_manager->render(Camera::get_keep_view(), Camera::get_keep_projection());

	auto r_sprite_render = [&](std::string gui_name, SpriteBatch* batch, Element& e, float glow_horizon = 0, float glow_vertical = 0)
	{
		//--sprite_string--//
#ifdef USE_IMGUI
		ImGui::Begin("title");
		if (ImGui::TreeNode(gui_name.c_str()))
		{
			ImGui::DragFloat2("pos", &e.position.x, 0.1f);
			ImGui::DragFloat2("scale", &e.scale.x, 0.01f);
			ImGui::TreePop();
		}
		ImGui::End();
#endif // USE_IMGUI
		batch->begin(graphics.get_dc().Get());
		batch->render(graphics.get_dc().Get(), e.position, e.scale, e.pivot, e.color, e.angle, e.texpos, e.texsize, glow_horizon, glow_vertical);
		batch->end(graphics.get_dc().Get());
	};
	auto r_font_render = [&](std::string name, StepFontElement& e)
	{
#ifdef USE_IMGUI
		ImGui::Begin("title");
		if (ImGui::TreeNode(name.c_str()))
		{
			ImGui::DragFloat2("pos", &e.position.x);
			ImGui::DragFloat2("scale", &e.scale.x, 0.1f);
			ImGui::ColorEdit4("color", &e.color.x);
			ImGui::TreePop();
		}
		ImGui::End();
#endif // USE_IMGUI
		fonts->yu_gothic->Draw(e.s, e.position, e.scale, e.color, e.angle, TEXT_ALIGN::UPPER_LEFT, e.length);
	};
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	//--logo_parameters--//
	r_sprite_render("animation", logo_parameters.sprite_animation.get(), logo_parameters.animation);
	r_sprite_render("logo", logo_parameters.sprite_logo.get(), logo_parameters.logo, logo_parameters.glow_horizon);


	/*-----!!!�������牺�ɃI�u�W�F�N�g�̕`��͂��Ȃ���!!!!-----*/

	// �V���h�E�}�b�v�̔j��
	shadow_map->clear_shadowmap(graphics);
	// post_effect
	post_effect->end(graphics.get_dc().Get());
	// �`��X�e�[�g�ݒ�
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	// �G�t�F�N�g��������
	post_effect->apply_an_effect(graphics.get_dc().Get(), elapsed_time);
	post_effect->blit(graphics.get_dc().Get());
	post_effect->scene_preview();
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
	//-------<2D�p�[�g>--------//
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	//--sprite_flash--//
	r_sprite_render("flash", sprite_flash.get(), flash);
	//--sprite_selecter--//
	r_sprite_render("selecter1", sprite_selecter.get(), selecter1);
	r_sprite_render("selecter2", sprite_selecter.get(), selecter2);

	//--font--//
	step_string(elapsed_time, L"���[�h��...", now_loading, 2.0f, true);

	//-----�}���`�v���C���ڂ�I�����Ă��Ȃ���-----//
	if (select_mulch_play == false)
	{
		fonts->yu_gothic->Begin(graphics.get_dc().Get());
		r_font_render("beginning", beginning);
		//if (has_stageNo_json) r_font_render("succession", succession);
		r_font_render("multiplay", multiplay);

		r_font_render("exit", exit);
	}
	//-----�}���`�v���C���ڂ�I�������Ƃ�-----//
	else
	{
		//-----�z�X�g�v���C-----//
		r_font_render("host_play_font", host_play_font);

		//-----�N���C�A���g�v���C-----//
		r_font_render("client_play_font", client_play_font);

		//-----�ЂƂO�̍��ڂɖ߂�-----//
		r_font_render("multi_paly_entry_back", multi_paly_entry_back);

	}

	if (start_matching)
	{
		step_string(elapsed_time, L"�}�b�`���O��...", now_matching, 2.0f, true);
		r_font_render("now_matching", now_matching);
	}

	if (!is_load_ready)	r_font_render("now_loading", now_loading);
	fonts->yu_gothic->End(graphics.get_dc().Get());
	// config
	r_sprite_render("config b", sprite_config_b.get(), config_b);
	fonts->yu_gothic->Begin(graphics.get_dc().Get());
	r_font_render("config_b_font", config_b_font);
	fonts->yu_gothic->End(graphics.get_dc().Get());

	if (tutorial_tab.display)
	{
		glow_vertical -= elapsed_time * 0.2f;

		r_sprite_render("frame", tutorial_tab.sprite_frame.get(), tutorial_tab.frame, 0, glow_vertical);
		r_sprite_render("tab selecterL", sprite_selecter.get(), tutorial_tab.selecterL);
		r_sprite_render("tab selecterR", sprite_selecter.get(), tutorial_tab.selecterR);

		fonts->yu_gothic->Begin(graphics.get_dc().Get());
		r_font_render("headline", tutorial_tab.headline);
		r_font_render("back", tutorial_tab.back);
		r_font_render("yes", tutorial_tab.yes);
		r_font_render("no", tutorial_tab.no);

		r_font_render("config_b_font2", config_b_font2);
		fonts->yu_gothic->End(graphics.get_dc().Get());

		r_sprite_render("config b2", sprite_config_b.get(), config_b2);

		r_sprite_render("config a", sprite_config_a.get(), config_a);
	}
}

void SceneTitle::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
	Camera* c = cameraManager->GetCurrentCamera();
	//--�V���h�E�}�b�v�̐���--//
	shadow_map->activate_shadowmap(graphics, c->get_light_direction());


	//--���̃r���[�|�[�g�ɖ߂�--//
	shadow_map->deactivate_shadowmap(graphics);
}

void SceneTitle::loading_thread(ID3D11Device* device)
{
	//--�^�C�g�������[�h--//
	ModelCashes::Load_IntoTitle(device);

	is_load_ready = true;
}

bool SceneTitle::step_string(float elapsed_time, std::wstring full_text,
	StepFontElement& step_font_element, float speed, bool loop)
{
	step_font_element.timer += elapsed_time * speed;
	step_font_element.step = static_cast<int>(step_font_element.timer);
	size_t size = full_text.size();
	if (step_font_element.index >= size + 1) // �ꕶ�������Ԃ�u��
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

void SceneTitle::LogoAnimation(float elapsed_time)
{
	if (logo_parameters.timer > LOGO_ANIMATION_WAIT_TIME) { logo_parameters.start_anim = true; }

	if (!logo_parameters.start_anim) frame_x = 0;
	else frame_x = static_cast<int>(logo_parameters.timer / logo_animation_speed) % (FRAMW_COUNT_X + 1);
#ifdef USE_IMGUI
	ImGui::Begin("title");
	if (ImGui::TreeNode("logo animation"))
	{
		ImGui::DragFloat("speed", &logo_animation_speed, 0.01f);
		ImGui::Text("timer:%f", logo_parameters.timer);
		ImGui::Text("frame_x:%d", frame_x);
		ImGui::Text("frame_y:%d", logo_parameters.frame_y);
		if (ImGui::Button("restart"))
		{
			logo_parameters.timer = 0;
			frame_x = 0;
			logo_parameters.frame_y = 0;
		}
		ImGui::TreePop();
	}
	ImGui::End();
#endif // USE_IMGUI
	if (frame_x >= FRAMW_COUNT_X)
	{
		// 1�s���̃A�j���[�V������
		if (logo_parameters.frame_y < FRAMW_COUNT_Y - 1)
		{
			logo_parameters.timer = 0;
			++logo_parameters.frame_y;
		}
	}
	else
	{
		// �A�j���[�V����
		if (logo_parameters.start_anim)
		{
			logo_parameters.animation.texpos.x = frame_x * logo_parameters.animation.texsize.x;
			logo_parameters.animation.texpos.y = logo_parameters.frame_y * logo_parameters.animation.texsize.y;
		}
		logo_parameters.timer += elapsed_time;
	}
	// ���Z�b�g
	if (logo_parameters.frame_y >= FRAMW_COUNT_Y - 1)
	{
		logo_parameters.reset_timer += elapsed_time;
		logo_parameters.glow_horizon -= logo_parameters.reset_timer * 0.1f;
		logo_parameters.glow_horizon = (std::max)(logo_parameters.glow_horizon, -12.0f);

		const float RESET_TIME = 5.0f;

		if (logo_parameters.reset_timer > RESET_TIME)
		{
			logo_parameters.reset_timer = 0.0f;
			logo_parameters.glow_horizon = 0.0f;

			frame_x = 0;
			logo_parameters.timer = 0;
			logo_parameters.frame_y = 0;
		}
	}

}

void SceneTitle::TitleSelectEntry(float elapsed_time)
{
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


	if (player->GetStartTitleAnimation() == false)
	{
		//-----�}�b�`���O���͑I�������͍s���Ȃ��悤�ɂ���-----//
		if (start_matching)
		{
			//-----�}�b�`���O�^�C�}�[��i�߂�-----//
			standby_matching_timer += 1.0f * elapsed_time;
			return;
		}

		//-----�}�b�`���O�ɐ��������ꍇ���������s��Ȃ��悤�ɂ���-----//
		if (change_scene_thread)
		{
			have_tutorial_state = 1; // �`���[�g���A���Ȃ�
			player->StartTitleAnimation();
			return;
		}

		//-----�}���`�v���C��I�����Ă��Ȃ��ꍇ-----//
		if (select_mulch_play == false)
		{
			switch (state)
			{
			case TitleEntry::Beginning: // beginning
				// tutorial_tab
				if (tutorial_tab.display)
				{
					auto r_up_tutorial = [&](int state, DirectX::XMFLOAT2 arrival_posL, DirectX::XMFLOAT2 arrival_posR)
					{
						if ((game_pad->get_button_down() & GamePad::BTN_UP) || (can_axis && game_pad->get_axis_LY() > 0.5f) || (can_axis && game_pad->get_axis_RY() > 0.5f))
						{
							audio_manager->play_se(SE_INDEX::SELECT);
							have_tutorial_state = state;
							tutorial_tab.arrival_posL = arrival_posL;
							tutorial_tab.arrival_posR = arrival_posR;

							can_axis = false;
						}
					};
					auto r_down_tutorial = [&](int state, DirectX::XMFLOAT2 arrival_posL, DirectX::XMFLOAT2 arrival_posR)
					{
						if ((game_pad->get_button_down() & GamePad::BTN_DOWN) || (can_axis && game_pad->get_axis_LY() < -0.5f) || (can_axis && game_pad->get_axis_RY() < -0.5f))
						{
							audio_manager->play_se(SE_INDEX::SELECT);
							have_tutorial_state = state;
							tutorial_tab.arrival_posL = arrival_posL;
							tutorial_tab.arrival_posR = arrival_posR;

							can_axis = false;
						}
					};
					switch (have_tutorial_state)
					{
					case 0: // �͂�
						r_down_tutorial(1, { 580,395 }, { 706,395 });
						break;

					case 1: // ������
						r_up_tutorial(0, { 587, 335 }, { 692, 335 });
						break;
					}

					tutorial_tab.selecterL.position = Math::lerp(tutorial_tab.selecterL.position, tutorial_tab.arrival_posL, 10.0f * elapsed_time);
					tutorial_tab.selecterR.position = Math::lerp(tutorial_tab.selecterR.position, tutorial_tab.arrival_posR, 10.0f * elapsed_time);

					// A�{�^���Ŗ߂�
					if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_A)
					{
						audio_manager->play_se(SE_INDEX::SELECT);
						tutorial_tab.display = false;
					}
					// ����
					if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_B)
					{
#if 0
						// �X�e�[�W�ԍ�0����
						WaveFile::get_instance().set_stage_to_start(0);
#else
						// �X�e�[�W�ԍ��{�X��O����
						WaveFile::get_instance().set_stage_to_start(WaveManager::STAGE_IDENTIFIER::S_3_1);
#endif
						WaveFile::get_instance().save();

						audio_manager->play_se(SE_INDEX::DECISION);
						player->StartTitleAnimation();
						tutorial_tab.display = false;
						return;
					}
				}
				else
				{
					////-----json�t�@�C�������邩�ǂ���-----//
					//if (has_stageNo_json) r_down(TitleEntry::Succession, { 990.0f, 595.0f }, { 1167.0f, 595.0f });
					////-----�Ȃ������瑱�����炪�Ȃ�����}���`�v���C�̕��ɍs��
					//else r_down(TitleEntry::Multiplay, { 990.0f,570.0f }, { 1177.0f,570.0f });

					//-----����̍��ڂɍs��-----//
					TitleEntryDown(TitleEntry::Multiplay, { 990.0f,570.0f }, { 1177.0f,570.0f });

					if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_B)
					{
						if (have_tutorial_state >= 0) /* �`���[�g���A���f�[�^������̂Ń^�u���� */
						{
							audio_manager->play_se(SE_INDEX::SELECT);
							tutorial_tab.display = true;
						}
						else
						{
#if 0
							// �X�e�[�W�ԍ�0����
							WaveFile::get_instance().set_stage_to_start(0);
#else
							// �X�e�[�W�ԍ��{�X��O����
							WaveFile::get_instance().set_stage_to_start(WaveManager::STAGE_IDENTIFIER::S_3_1);
#endif
							WaveFile::get_instance().save();

							audio_manager->play_se(SE_INDEX::DECISION);
							player->StartTitleAnimation();
							return;
						}
					}
				}
				break;
			case TitleEntry::Succession: // succession
				TitleEntryUp(TitleEntry::Beginning, { 990.0f, 516.0f }, { 1167.0f, 516.0f });
				TitleEntryDown(TitleEntry::Multiplay, { 990.0f,570.0f }, { 1177.0f,570.0f });
				if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_B)
				{
					have_tutorial_state = 1; // �`���[�g���A���Ȃ�
					audio_manager->play_se(SE_INDEX::DECISION);
					player->StartTitleAnimation();
					return;
				}
				break;
				//-----�}���`�v���C-----//
			case TitleEntry::Multiplay:
				TitleEntryUp(TitleEntry::Beginning, { 1001.1f, 516.3f }, { 1155.0f, 516.3f });
				TitleEntryDown(TitleEntry::Exit, { 984.7f,626.8f }, { 1176.8f,626.8f });
				if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_B)
				{
					//----�}���`�v���C��I������-----//
					select_mulch_play = true;

					//-----�X�e�[�g��������-----//
					multi_play_entry_state = MultiPlayEntry::Host;

					//-----�Z���N�g�o�[�̈ʒu��ݒ�-----//
					arrival_pos1 = { 994.1f,520.0f };
					arrival_pos2 = { 1176.1f,520.0f };

					audio_manager->play_se(SE_INDEX::DECISION);
					return;
				}
				break;
			case TitleEntry::Exit: // exit
				//if (has_stageNo_json) r_up(1, { 990.0f, 595.0f }, { 1167.0f, 595.0f });
				//else r_up(0, { 990.0f, 565.0f }, { 1167.0f, 565.0f });

				TitleEntryUp(TitleEntry::Multiplay, { 990.0f,570.0f }, { 1177.0f,570.0f });

				if (game_pad->get_button_down() & GamePad::BTN_B)
				{
					audio_manager->play_se(SE_INDEX::DECISION);
					PostQuitMessage(0);
					return;
				}
				break;
			}
		}
		//-----�}���`�v���C��I�������ꍇ-----//
		else
		{
			switch (multi_play_entry_state)
			{
				//-----�z�X�g�v���C-----//
			case MultiPlayEntry::Host:
				MultiPlayEntryDown(MultiPlayEntry::Client, { 954.6f,579.0f }, { 1218.0f,579.0f });

				if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_B)
				{
						// �X�e�[�W�ԍ��{�X��O����
						WaveFile::get_instance().set_stage_to_start(WaveManager::STAGE_IDENTIFIER::S_3_1);
						WaveFile::get_instance().save();
						have_tutorial_state = 1; // �`���[�g���A���Ȃ�
						audio_manager->play_se(SE_INDEX::DECISION);
						player->StartTitleAnimation();

						return;
			    }

				break;
				//-----�Q�[���ɎQ��-----//
			case MultiPlayEntry::Client:

				MultiPlayEntryUp(MultiPlayEntry::Host, { 994.1f,520.0f }, { 1176.1f,520.0f });

				MultiPlayEntryDown(MultiPlayEntry::Back, { 1030.0f,640.0f }, { 1139.0f,640.0f });

				if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_B)
				{
					//-----�}�b�`���O����������-----//
					if (CorrespondenceManager::Instance().InitializeClient())
					{
						DebugConsole::Instance().WriteDebugConsole("�������ɐ������܂���", TextColor::Green);

						//-----�}���`�X���b�h���J�n-----//
						std::thread t(StandbyMatching);
						t.swap(standby_thread);
						standby_thread.detach();

						//-----�u���b�L���O�ݒ������-----//
						std::lock_guard<std::mutex> lock(mutex);
						//-----�}�b�`���O�J�n�t���O�𗧂Ă�-----//
						start_matching = true;
					}
					return;
				}

				break;
				//-----�ЂƂO�̍��ڂɖ߂�-----//
			case MultiPlayEntry::Back:

				MultiPlayEntryUp(MultiPlayEntry::Client, { 954.6f,579.0f }, { 1218.0f,579.0f });

				if (is_load_ready && game_pad->get_button_down() & GamePad::BTN_B)
				{
					select_mulch_play = false;
					//-----�X�e�[�g��������-----//
					state = TitleEntry::Beginning;

					//-----�Z���N�g�o�[�̈ʒu��ݒ�-----//
					arrival_pos1 = { 990.0f, 516.0f };
					arrival_pos2 = { 1167.0f, 516.0f };

					audio_manager->play_se(SE_INDEX::DECISION);
				}
				break;
			default:
				break;
			}

		}
	}
	else
	{
		slashing_wait_timer += elapsed_time;
		if (has_stageNo_json && (game_pad->get_button_down() & GamePad::BTN_B))
		{
			slashing_power = SLASHING_MAX;
		}
	}

	////-----�Z���N�^�[�̓���-----//
	selecter1.position = Math::lerp(selecter1.position, arrival_pos1, 10.0f * elapsed_time);
	selecter2.position = Math::lerp(selecter2.position, arrival_pos2, 10.0f * elapsed_time);

}

void SceneTitle::StandbyMatching()
{
	DebugConsole::Instance().WriteDebugConsole("�X���b�h�J�n");
	CorrespondenceManager::Instance().Login();
	for (;;)
	{
		if (standby_matching_timer > 10.0f)
		{
			DebugConsole::Instance().WriteDebugConsole("�}�b�`���O�Ɏ��s���܂���", TextColor::Red);
			//-----�u���b�L���O�ݒ������-----//
			std::lock_guard<std::mutex> lock(mutex);

			//-----�}�b�`���O�J�n�t���O����������-----//
			start_matching = false;
			break;
		}
		if (CorrespondenceManager::Instance().LoginReceive())
		{
			DebugConsole::Instance().WriteDebugConsole("�}�b�`���O�ɐ������܂���", TextColor::Green);

			//-----�V�[���ؑւ����Ă������Ƃ�`����-----//
			change_scene_thread = true;

			//-----�u���b�L���O�ݒ������-----//
			std::lock_guard<std::mutex> lock(mutex);

			//-----�}�b�`���O�J�n�t���O����������-----//
			start_matching = false;

			break;
		}
	}
}

void SceneTitle::TitleEntryUp(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2)
{
	if ((game_pad->get_button_down() & GamePad::BTN_UP) || (can_axis && game_pad->get_axis_LY() > 0.5f) || (can_axis && game_pad->get_axis_RY() > 0.5f))
	{
		audio_manager->play_se(SE_INDEX::SELECT);
		state = next_state;
		this->arrival_pos1 = arrival_pos1;
		this->arrival_pos2 = arrival_pos2;

		can_axis = false;
	}

}

void SceneTitle::TitleEntryDown(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2)
{
	if ((game_pad->get_button_down() & GamePad::BTN_DOWN) || (can_axis && game_pad->get_axis_LY() < -0.5f) || (can_axis && game_pad->get_axis_RY() < -0.5f))
	{
		audio_manager->play_se(SE_INDEX::SELECT);
		state = next_state;
		this->arrival_pos1 = arrival_pos1;
		this->arrival_pos2 = arrival_pos2;

		can_axis = false;
	}

}

void SceneTitle::MultiPlayEntryUp(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2)
{
	if ((game_pad->get_button_down() & GamePad::BTN_UP) || (can_axis && game_pad->get_axis_LY() > 0.5f) || (can_axis && game_pad->get_axis_RY() > 0.5f))
	{
		audio_manager->play_se(SE_INDEX::SELECT);
		this->multi_play_entry_state = next_state;
		this->arrival_pos1 = arrival_pos1;
		this->arrival_pos2 = arrival_pos2;

		can_axis = false;
	}
}

void SceneTitle::MultiPlayEntryDown(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2)
{
	if ((game_pad->get_button_down() & GamePad::BTN_DOWN) || (can_axis && game_pad->get_axis_LY() < -0.5f) || (can_axis && game_pad->get_axis_RY() < -0.5f))
	{
		audio_manager->play_se(SE_INDEX::SELECT);
		this->multi_play_entry_state = next_state;
		this->arrival_pos1 = arrival_pos1;
		this->arrival_pos2 = arrival_pos2;

		can_axis = false;
	}
}
