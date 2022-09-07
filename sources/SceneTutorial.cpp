#include "SceneTutorial.h"
#include <cereal/archives/json.hpp>

#include <memory>
#include "scene_title.h"
#include "scene_loading.h"
#include "scene_manager.h"
#include"scene_game.h"
#include "framework.h"
#include "graphics_pipeline.h"
#include"imgui_include.h"

#include "collision.h"
#include "shader.h"
#include "texture.h"

#include "user.h"


void TutorialScene::initialize(GraphicsPipeline& graphics)
{

	// Jsonにかきだし
	std::filesystem::path path = "./resources/Data/tutorial.json";
	path.replace_extension(".json");
	int a = 1;
	std::ofstream ifs(path);
	if (ifs)
	{
		cereal::JSONOutputArchive o_archive(ifs);
		o_archive(a);
	}

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
	mWaveManager.fGetEnemyManager()->fSetIsTutorial(true);
	player = std::make_unique<Player>(graphics);
	player->SetTutorialDamageFunc();
	player->SetIsTutorial(true);
	// カメラ
	cameraManager = std::make_unique<CameraManager>();

	cameraManager->RegisterCamera(new GameCamera(player.get()));
	cameraManager->RegisterCamera(new ClearCamera(player.get()));
	cameraManager->RegisterCamera(new JointCamera(graphics));

	//cameraManager->SetCamera(static_cast<int>(CameraTypes::Game));
	//cameraManager->Initialize(graphics);
	cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));

	// enemy_hp_gauge
	enemy_hp_gauge = std::make_unique<EnemyHpGauge>(graphics);
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

	check_mark = std::make_unique<SpriteDissolve>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\CheckMark.png",
		L".\\resources\\Sprites\\mask\\dissolve_mask1.png", 1, true);
	change_scene_gauge = std::make_unique<SpriteDissolve>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\skip.png",
		L".\\resources\\Sprites\\mask\\dissolve_mask1.png", 1);
	check_box = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\CheckBox.png", 1);
	arrow_mark = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\arrow_mark.png", 1);
	arrow_mark_pram.position = { 397.4f,216.9f };
	arrow_mark_pram.scale = { 0.5f,0.5f };
	arrow_mark_pram.angle = 108.0f;
	arrow_mark_pram.texsize = { static_cast<float>(arrow_mark->get_texture2d_desc().Width),static_cast<float>(arrow_mark->get_texture2d_desc().Height) };

	sprite_tutorial_frame = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\back.png", 1);
	frame_pram.position = { 249.9f,19.6f };
	frame_pram.scale = { 0.65f,0.35f };
	frame_pram.texsize = { static_cast<float>(sprite_tutorial_frame->get_texture2d_desc().Width),static_cast<float>(sprite_tutorial_frame->get_texture2d_desc().Height) };

	brack_back = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\mask\\black_mask.png", 1);
	brack_back_pram.texsize = { static_cast<float>(brack_back->get_texture2d_desc().Width),
										static_cast<float>(brack_back->get_texture2d_desc().Height) };
	brack_back_pram.color.w = 0.7f;

	sprite_frame = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\title\\title_back.png", 1);
	sprite_frame_parm.texsize = { static_cast<float>(sprite_frame->get_texture2d_desc().Width),
										static_cast<float>(sprite_frame->get_texture2d_desc().Height) };
	sprite_frame_parm.scale = { 1.74f,1.10f };
	sprite_frame_parm.position = { 118.0f,-17.2f };

	just_avoidance = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\just.png", 1);
	just_avoidance_parm.position = { 378.9f,93.0f };
	just_avoidance_parm.scale = { 0.3f,0.3f };
	just_avoidance_parm.texsize = { static_cast<float>(just_avoidance->get_texture2d_desc().Width),
										static_cast<float>(just_avoidance->get_texture2d_desc().Height) };
	awaiking_chain = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\awaiking.png", 1);
	awaiking_chain_parm.position = { 378.9f,93.7f };
	awaiking_chain_parm.scale = { 0.3f,0.3f };
	awaiking_chain_parm.texsize = { static_cast<float>(awaiking_chain->get_texture2d_desc().Width),
										static_cast<float>(awaiking_chain->get_texture2d_desc().Height) };

	//コントローラーの画像
	{
		controller_base = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\base.png", 1);
		controller_pram.texsize = { static_cast<float>(controller_base->get_texture2d_desc().Width),
										static_cast<float>(controller_base->get_texture2d_desc().Height) };
		controller_pram.position = { 1014.4f,259.6f };
		controller_pram.scale = { 0.3f,0.3f };

		controller_keys[ControllerSprite::A] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\a.png", 1);
		controller_on_keys[ControllerSprite::A] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\a_on.png", 1);
		controller_keys[ControllerSprite::B] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\b.png", 2);
		controller_on_keys[ControllerSprite::B] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\b_on.png", 1);
		controller_b_pram.position = controller_b_pram.position = { 365.7f, 270.2f };
		controller_b_pram.texsize = { static_cast<float>(controller_keys[ControllerSprite::B]->get_texture2d_desc().Width),
										static_cast<float>(controller_keys[ControllerSprite::B]->get_texture2d_desc().Height) };
		controller_keys[ControllerSprite::X] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\x.png", 1);
		controller_on_keys[ControllerSprite::X] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\x_on.png", 1);
		controller_keys[ControllerSprite::Y] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\y.png", 1);
		controller_on_keys[ControllerSprite::Y] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\y_on.png", 1);
		controller_keys[ControllerSprite::RB] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\rb.png", 1);
		controller_on_keys[ControllerSprite::RB] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\rb_on.png", 1);
		controller_keys[ControllerSprite::RT] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\rt.png", 1);
		controller_on_keys[ControllerSprite::RT] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\rt_on.png", 1);
		controller_keys[ControllerSprite::LB] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\lb.png", 1);
		controller_on_keys[ControllerSprite::LB] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\lb_on.png", 1);
		controller_keys[ControllerSprite::LT] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\lt.png", 1);
		controller_on_keys[ControllerSprite::LT] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\lt_on.png", 1);
		controller_keys[ControllerSprite::RightStick] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\right_stick.png", 1);
		controller_on_keys[ControllerSprite::RightStick] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\right_stick_on.png", 1);
		controller_keys[ControllerSprite::LeftStick] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\left_stick.png", 1);
		controller_on_keys[ControllerSprite::LeftStick] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\left_stick_on.png", 1);
		controller_keys[ControllerSprite::Cross] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\cross.png", 1);
		controller_keys[ControllerSprite::Menu] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\menu.png", 1);
		controller_on_keys[ControllerSprite::Menu] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\menu_on.png", 1);
		controller_keys[ControllerSprite::Back] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\back.png", 1);
		controller_on_keys[ControllerSprite::Back] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\back_2_on.png", 2);
		controller_keys[ControllerSprite::Back2] = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\controller\\back_2.png", 1);
		controller_back_pram.texsize = { static_cast<float>(controller_keys[ControllerSprite::Back]->get_texture2d_desc().Width),
										static_cast<float>(controller_keys[ControllerSprite::Back]->get_texture2d_desc().Height) };
		controller_back_button_pram.texsize = { static_cast<float>(controller_keys[ControllerSprite::Back2]->get_texture2d_desc().Width),
										static_cast<float>(controller_keys[ControllerSprite::Back2]->get_texture2d_desc().Height) };
		controller_back_button_pram.scale = { 0.8f,0.8f };
		controller_back_button_pram.position = { -52.6f,194.0f };
		controller_back_pram.position = { -156.3f,145.0f };
		controller_back_pram.scale = { 0.3f,0.3f };

	}
	//チュートリアル文の初期化
	tutorial_text_element[0].tutorial_text = L"Lスティックでプレイヤーを動かすことができ\nRスティックでカメラを動かすことができます";
	tutorial_text_element[1].tutorial_text = L"RBボタンかRTボタンを押すと回避することができます\nもう一度ボタンを押すと入力方向に方向転換できます";
	tutorial_text_element[2].tutorial_text = L"敵が視界に入っている時にLTボタンを押している間\nロックオンすることができます";
	tutorial_text_element[3].tutorial_text = L"Bボタンを押すと攻撃でき,ロックオン中は自動で敵を攻撃します\n敵に当たったらもう一度Bボタンを押して連続攻撃できます";
	tutorial_text_element[4].tutorial_text = L"ロックオン中に一定の近さの時に回避をすると\n回り込み,スタンさせます\n回り込み回避を成功させるとクールタイムが発生します";
	tutorial_text_element[5].tutorial_text = L"LBボタンを押し続けると\nカメラ内にいるスタンしている敵をロックオンして\n一度に攻撃することができます";
	tutorial_text_element[6].tutorial_text = L"敵に攻撃するとゲージがたまっていき\n満タンの状態でAボタンを押すと覚醒状態になります\nチェイン攻撃ではゲージはたまりません";
	tutorial_text_element[7].tutorial_text = L"チュートリアルを終了します";

	tutorial_text_element[0].position = { 376.0f,72.0f };
	tutorial_text_element[1].position = { 297.0f,76.0f };
	tutorial_text_element[2].position = { 341.0f,75.0f };
	tutorial_text_element[3].position = { 252.0f,86.0f };
	tutorial_text_element[4].position = { 276.0f,53.0f };
	tutorial_text_element[5].position = { 362.0f,55.0f };
	tutorial_text_element[6].position = { 320.0f,65.0f };
	tutorial_text_element[7].position = { 479.0f,102.0f };

	tutorial_count_text.s = L"あと３回";

	audio_manager->stop_all_bgm();
	audio_manager->play_bgm(BGM_INDEX::TUTORIAL);

	sky_dome->set_purple_threshold(0); sky_dome->set_red_threshold(0);
}

void TutorialScene::uninitialize()
{
	BulletManager& mBulletManager = BulletManager::Instance();
	mWaveManager.fFinalize();
	mBulletManager.fFinalize();
}

void TutorialScene::effect_liberation(GraphicsPipeline& graphics)
{
	effect_manager->finalize();
	effect_manager->initialize(graphics);
}

void TutorialScene::update(GraphicsPipeline& graphics, float elapsed_time)
{
	const float bgm_volume = 2.0f;
	const float se_volume = 2.0f;
	audio_manager->set_all_volume_bgm(bgm_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_bgm_volume());
	audio_manager->set_all_volume_se(se_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_se_volume());

	TutorialUpdate(graphics, elapsed_time);
	//イベント中は動かない
	if (player->GetTutorialEvent()) return;
	//画像のチュートリアル中は進まない
	if (sprite_tutorial) return;
	// option
	if (option->get_validity())
	{
		option->update(graphics, elapsed_time);
		return;
	}
	else
	{
		if (game_pad->get_button_down() & GamePad::BTN_START)
		{
			option->initialize();
			option->set_validity(true);
			return;
		}
	}
	BulletManager& mBulletManager = BulletManager::Instance();
	// クリア演出
	mWaveManager.fUpdate(graphics, elapsed_time, mBulletManager.fGetAddFunction());
#if 0
	if (mWaveManager.during_clear_performance())
	{
		tunnel_alpha += elapsed_time * 0.5f;
		tunnel_alpha = (std::min)(tunnel_alpha, 1.0f);

		if (!during_clear)
		{
			cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Tunnel));
			player->TransitionStageMove();
			during_clear = true;
		}

		//return;
	}
	else
	{
		if (during_clear)
		{
			tunnel_alpha -= elapsed_time;
			tunnel_alpha = (std::max)(tunnel_alpha, 0.0f);
			if (Math::equal_check(tunnel_alpha, 0.0f, 0.01f))
			{
				cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));
				player->TransitionIdle();
				tunnel_alpha = 0.0f;
				during_clear = false;
			}
		}
	}

#endif // 0
	//プレイヤーがジャスト回避したらslow
	if (player->GetIsJustAvoidance())
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
		//タイマーが0.5秒以下なら遅くする
		if (slow_timer < 1.0f)
		{
			elapsed_time *= slow_rate;
		}
	}

	//--------------------<敵の管理クラスの更新処理>--------------------//

	const auto enemyManager = mWaveManager.fGetEnemyManager();


	enemyManager->fSetPlayerPosition(player->GetPosition());
	mBulletManager.fUpdate(elapsed_time);

	// ↓↓↓↓↓↓↓↓↓プレイヤーの更新はこのした↓↓↓↓↓
	BaseEnemy* enemy = enemyManager->fGetNearestEnemyPosition();

	Camera* c = cameraManager->GetCurrentCamera();

	if (player->GetIsAlive() == false)	SceneManager::scene_switching(new SceneLoading(new SceneTitle()), DISSOLVE_TYPE::DOT, 2.0f);


	// 敵とのあたり判定(当たったらコンボ加算)
	if (player->GetIsPlayerAttack())
	{
		bool block = false;
		if (player->GetIsAwakening())
		{
			player->AwakingAddCombo
			(
				enemyManager->fCalcPlayerAttackVsEnemies
				(
					player->GetSwordCapsuleParam(0).start,
					player->GetSwordCapsuleParam(0).end,
					player->GetSwordCapsuleParam(0).rasius,
					player->GetPlayerPower(),
					graphics,
					elapsed_time,
					block
				),
				enemyManager->fCalcPlayerAttackVsEnemies
				(
					player->GetSwordCapsuleParam(1).start,
					player->GetSwordCapsuleParam(1).end,
					player->GetSwordCapsuleParam(1).rasius,
					player->GetPlayerPower(),
					graphics,
					elapsed_time,
					block
				)
				, block
			);
		}
		else
		{
			player->AddCombo(enemyManager->fCalcPlayerAttackVsEnemies(
				player->GetSwordCapsuleParam(0).start,
				player->GetSwordCapsuleParam(0).end,
				player->GetSwordCapsuleParam(0).rasius,
				player->GetPlayerPower(),
				graphics,
				elapsed_time,
				block
			), block);
		}
	}

	const bool isCounter = enemyManager->fCalcEnemiesAttackVsPlayerCounter(
		player->GetJustAvoidanceCapsuleParam().start,
		player->GetJustAvoidanceCapsuleParam().end,
		player->GetJustAvoidanceCapsuleParam().rasius);
	player->PlayerJustAvoidance(isCounter);

	enemyManager->fCalcEnemiesAttackVsPlayer(player->GetBodyCapsuleParam().start,
		player->GetBodyCapsuleParam().end,
		player->GetBodyCapsuleParam().rasius, player->GetDamagedFunc());

	//プレイヤーがジャスト回避した時の範囲スタンの当たり判定
	enemyManager->fCalcPlayerStunVsEnemyBody(player->GetPosition(), player->GetStunRadius());

	//プレイヤーがチェイン状態であることを敵に知らせて行動を停止させる
	enemyManager->fSetIsPlayerChainTime(player->during_chain_attack());
	//弾とプレイヤーの当たり判定
	mBulletManager.fCalcBulletsVsPlayer(player->GetBodyCapsuleParam().start,
		player->GetBodyCapsuleParam().end,
		player->GetBodyCapsuleParam().rasius, player->GetDamagedFunc());

	// camera
	//camera->Update(elapsed_time,player.get());
	cameraManager->Update(elapsed_time);

	player->SetCameraDirection(c->GetForward(), c->GetRight());
	player->UpdateTutorial(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());
	player->lockon_post_effect(elapsed_time, [=](float scope, float alpha) { post_effect->lockon_post_effect(scope, alpha); },
		[=]() { post_effect->clear_post_effect(); });
	player->SetCameraPosition(c->get_eye());
	player->SetTarget(enemy);
	player->SetCameraTarget(c->get_target());
	if (player->GetStartDashEffect()) post_effect->dash_post_effect(graphics.get_dc().Get(), player->GetPosition());

	enemy_hp_gauge->update(graphics, elapsed_time);
	enemy_hp_gauge->focus(player->GetPlayerTargetEnemy(), player->GetEnemyLockOn());

	reticle->update(graphics, elapsed_time);
	reticle->SetAvoidanceCharge(player->GetBehaindCharge());
	reticle->focus(player->GetPlayerTargetEnemy(), player->GetEnemyLockOn());
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
#endif
		wave->set_positoin(pos); wave->set_offset(offset); wave->set_scale(scale); wave->set_value(value); wave->set_color(color);
		wave->update(graphics, elapsed_time);
	}


	//camera->update_with_quaternion(elapsed_time);
	// shadow_map
	shadow_map->debug_imgui();

	effect_manager->update(elapsed_time);


	//audio_manager->set_volume_bgm(BGM_INDEX::ENDING, bgm_volume);


	//****************************************************************
	//
	// オブジェクトの削除処理はこの下でやるルール
	//
	//****************************************************************
	//enemyManager->fDeleteEnemies();

}
#define OFF_SCREEN_RENDERING
#define SHADOW_MAP

void TutorialScene::render(GraphicsPipeline& graphics, float elapsed_time)
{
#ifdef OFF_SCREEN_RENDERING
	post_effect->begin(graphics.get_dc().Get());
#endif // DEBUG

	// シャドウマップのセット
#ifdef SHADOW_MAP
	shadow_map->set_shadowmap(graphics);
#endif // SHADOW_MAP
	// カメラのビュー行列計算
	//camera->calc_view_projection(graphics);
	cameraManager->CalcViewProjection(graphics);

	/*-----!!!ここから上にオブジェクトの描画はしないで!!!!-----*/
	{
		graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::DEFAULT);
#ifdef USE_IMGUI
		ImGui::Begin("sky");
		ImGui::DragFloat("dimension", &dimension, 0.01f);
		ImGui::End();
#endif
		sky_dome->Render(graphics, elapsed_time);


	}


	// font demo
#if 0
	{
		graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		fonts->gothic->Begin(graphics.get_dc().Get());
		fonts->gothic->Draw(L"テスト", { 640, 360 }, { 1,1 }, { 0,0,0,1 }, 0, TEXT_ALIGN::MIDDLE);
		fonts->gothic->End(graphics.get_dc().Get());
	}
#endif // 0
	BulletManager& mBulletManager = BulletManager::Instance();

	//--------------------<敵の管理クラスの描画処理>--------------------//
	if (player->GetTutorialEvent() == false)mWaveManager.fGetEnemyManager()->fRender(graphics);
	player->Render(graphics, elapsed_time);
	mBulletManager.fRender(graphics);

	//--------<ui>--------//
	graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	// enemy_hp_gauge
	if (player->GetTutorialEvent() == false)enemy_hp_gauge->render(graphics.get_dc().Get());
	// reticle
	if (player->GetTutorialEvent() == false)reticle->render(graphics.get_dc().Get());
	// wave
	//wave->render(graphics.get_dc().Get());
	Camera* c = cameraManager->GetCurrentCamera();
	const DirectX::XMFLOAT2 p_pos = { player->GetPosition().x,player->GetPosition().z };
	const DirectX::XMFLOAT2 p_forward = { player->GetForward().x,player->GetForward().z };
	const DirectX::XMFLOAT2 c_forward = { c->GetForward().x,c->GetForward().z };
	if (player->GetTutorialEvent() == false)minimap->render(graphics, p_pos, p_forward, c_forward, mWaveManager.fGetEnemyManager()->fGetEnemies());

	if (player->GetTutorialEvent() == false)effect_manager->render(Camera::get_keep_view(), Camera::get_keep_projection());
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::WIREFRAME_CULL_BACK, DEPTH_STENCIL::DEON_DWON);
	debug_figure->render_all_figures(graphics.get_dc().Get());


	// クリア中のトンネル
	if (during_clear)
	{
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		tunnel->render(graphics.get_dc().Get(), elapsed_time, tunnel_alpha, [&]() {
			player->Render(graphics, elapsed_time);
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
	if (player->GetTutorialEvent() == false)
	{
		player->TutorialConfigRender(graphics, elapsed_time, free_practice);
	}
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	mWaveManager.render(graphics.get_dc().Get(), elapsed_time);
	if (player->GetTutorialEvent() == false)TutorialRender(graphics, elapsed_time);
	if (option->get_validity()) { option->render(graphics, elapsed_time); }

}

void TutorialScene::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
#ifdef SHADOW_MAP
	Camera* c = cameraManager->GetCurrentCamera();
	//--シャドウマップの生成--//
	shadow_map->activate_shadowmap(graphics, c->get_light_direction());







	//--元のビューポートに戻す--//
	shadow_map->deactivate_shadowmap(graphics);
#endif // SHADOW_MAP

}

void TutorialScene::TutorialUpdate(GraphicsPipeline& graphics, float elapsed_time)
{
#ifdef USE_IMGUI
	{
			ImGui::Begin("tutorial");
			ImGui::DragFloat("change_scene_timer", &change_scene_timer);
			if (ImGui::Button("is_next")) is_next = true;
			//static int state = 1;
			//ImGui::SliderInt("tutorial_state", &state, 1, 7);
			//tutorial_state = static_cast<TutorialState>(state);
			ImGui::End();
	}
#endif
		//イベント中はこっちでプレイヤーとカメラの更新処理を書いておく
		const auto enemyManager = mWaveManager.fGetEnemyManager();
	//プレイヤーのイベント中
	if (player->GetTutorialEvent())
	{
		cameraManager->Update(elapsed_time);
		player->UpdateTutorial(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());
		//もしまだジョイントカメラをセットしていなかったら
		if (set_joint_camera == false)
		{
			//ここでセット
			cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Joint));
			set_joint_camera = true;
		}
		if (set_joint_camera)
		{
			//セットされていたら位置を代入
			cameraManager->GetCurrentCamera()->set_eye(player->GetEnentCameraJoint());
			cameraManager->GetCurrentCamera()->set_target(player->GetEnentCameraEye());
		}
	}
	else
	{
		//もしイベント中出なかったら
		if (set_joint_camera)
		{
			//カメラをゲームのものに戻す
			cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));
			set_joint_camera = false;
		}
	}

	//プレイヤー側の今のチュートリアルが終わってるときかつis_nextがtrueの時
	if (player->GetNextTutorial() && is_end_text)
	{
		//チェックボックスのディゾルブを開始
		check_mark_parm.is_threshold = true;
	}
	if (check_mark_parm.is_threshold)
	{
		//0よりも大きかったら引いていく
		if (check_mark_parm.threshold > 0)check_mark_parm.threshold -= 1.0f * elapsed_time;
		else
		{
			is_next = true;
		}
	}

	std::wstring count_text_first = L"あと";
	std::wstring count_text_count = std::to_wstring(player->GetTutorialCount());
	std::wstring count_text_last = L"回";

	switch (tutorial_state)
	{
	case TutorialScene::TutorialState::MoveTutorial:
		button_priset = BottunPriset::MoveTutorialPriset;
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		tutorial_check_text = L"Lスティックで移動させる";
		if (is_next)
		{
			Judea_timer += 1.0f * elapsed_time;
			if (Judea_timer > 1.0f)
			{
				//次のステートに設定
				tutorial_state = TutorialState::AvoidanceTutorial;
				//次に進むフラグの初期化
				is_next = false;
				//プレイヤーの次に進むフラグを初期化
				player->FalseNextTutorial();
				//猶予時間を初期化
				Judea_timer = 0;
				//ディゾルブ時間を初期化
				check_mark_parm.threshold = 1.0f;
				//ディゾルブしていいかどうかのフラグを初期化
				check_mark_parm.is_threshold = false;
			}
		}
		break;
	case TutorialScene::TutorialState::AvoidanceTutorial:
		button_priset = BottunPriset::AvoidanceTutorialPriset;
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		tutorial_check_text = L"RB,RT,ボタンを押して回避する";
		if (is_next)
		{
			Judea_timer += 1.0f * elapsed_time;
			if (Judea_timer > 1.0f)
			{
				//次のステートに設定
				tutorial_state = TutorialState::LockOnTutorial;
				//次に進むフラグの初期化
				is_next = false;
				//プレイヤーの次に進むフラグを初期化
				player->FalseNextTutorial();
				//猶予時間を初期化
				Judea_timer = 0;
				//ディゾルブ時間を初期化
				check_mark_parm.threshold = 1.0f;
				//ディゾルブしていいかどうかのフラグを初期化
				check_mark_parm.is_threshold = false;
			}
		}

		break;
	case TutorialScene::TutorialState::LockOnTutorial:
		button_priset = BottunPriset::LockOnTutorialPriset;
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		tutorial_check_text = L"LTボタンでロックオンする";
		if (is_next)
		{
			Judea_timer += 1.0f * elapsed_time;
			if (Judea_timer > 1.0f)
			{
				//次のステートに設定
				tutorial_state = TutorialState::AttackTutorial;
				//次に進むフラグの初期化
				is_next = false;
				//プレイヤーの次に進むフラグを初期化
				player->FalseNextTutorial();
				//猶予時間を初期化
				Judea_timer = 0;
				//ディゾルブ時間を初期化
				check_mark_parm.threshold = 1.0f;
				//ディゾルブしていいかどうかのフラグを初期化
				check_mark_parm.is_threshold = false;
			}
		}

		// 敵がわいてくる
		enemyManager->fSpawnTutorial_NoAttack(elapsed_time, graphics);

		break;
	case TutorialScene::TutorialState::AttackTutorial:
		button_priset = BottunPriset::AttackTutorialPriset;
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		tutorial_check_text = L"Bボタンを押して攻撃";
		if (is_next)
		{
			Judea_timer += 1.0f * elapsed_time;
			if (Judea_timer > 1.0f)
			{
				//回数を設定
				player->SetTutorialCount(3);
				//次のステートに設定
				tutorial_state = TutorialState::BehindAvoidanceTutorial;
				//次に進むフラグの初期化
				is_next = false;
				//プレイヤーの次に進むフラグを初期化
				player->FalseNextTutorial();
				//猶予時間を初期化
				Judea_timer = 0;
				//ディゾルブ時間を初期化
				check_mark_parm.threshold = 1.0f;
				//ディゾルブしていいかどうかのフラグを初期化
				check_mark_parm.is_threshold = false;
			}
		}

		break;
	case TutorialScene::TutorialState::BehindAvoidanceTutorial:
		button_priset = BottunPriset::BehindAvoidanceTutorialPriset;
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		tutorial_check_text = L"回り込み回避をする";
		tutorial_count_text.s = count_text_first + count_text_count + count_text_last;
		tutorial_count_text.position = { 955.0f,173.0f };
		tutorial_count_text.color = { 1.0f,1.0f,0.0f,1.0f };
		//画像のチュートリアルのパラメータ設定
		sprite_tutorial_text.position = { 265.0f,430.0f };
		sprite_tutorial_text.s = L"敵の攻撃が当たりそうなときに回避するとジャスト回避ができます\nジャスト回避は近くの範囲内の敵をスタンさせることができます\nジャスト回避をするとHPが少量回復します";
		if (is_next)
		{
			//ジャスト回避の説明をする
			sprite_tutorial = true;
			//タイマーを進める
			sprite_tutorial_timer += 1.0f * elapsed_time;
			if (sprite_tutorial_timer > 1.0f && game_pad->get_button_down() & GamePad::BTN_B)
			{
				end_sprite_tutorial = true;
			}
		}
		//画像のチュートリアルが終わったら
		if (end_sprite_tutorial)
		{
			Judea_timer += 1.0f * elapsed_time;
			if (Judea_timer > 1.0f)
			{
				//回数を設定
				player->SetTutorialCount(3);
				//次のステートに設定
				tutorial_state = TutorialState::ChainAttackTutorial;
				//次に進むフラグの初期化
				is_next = false;
				//プレイヤーの次に進むフラグを初期化
				player->FalseNextTutorial();
				//猶予時間を初期化
				Judea_timer = 0;
				//ディゾルブ時間を初期化
				check_mark_parm.threshold = 1.0f;
				//ディゾルブしていいかどうかのフラグを初期化
				check_mark_parm.is_threshold = false;
				//画像の説明のフラグを初期化
				sprite_tutorial = false;
				end_sprite_tutorial = false;
				sprite_tutorial_timer = 0.0f;
			}
		}

		// 敵がわいてくる
		enemyManager->fSpawnTutorial_NoAttack(elapsed_time, graphics);

		break;
	case TutorialScene::TutorialState::ChainAttackTutorial:
		button_priset = BottunPriset::ChainAttackTutorialPriset;
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		tutorial_check_text = L"LBボタンを長押ししてスタンしている敵をロックオン";
		tutorial_count_text.s = count_text_first + count_text_count + count_text_last;
		if (player->EnemiesIsStun(enemyManager->fGetEnemies()))is_stun_timer = 0.0f;
		else is_stun_timer += 1.0f * elapsed_time;

		for (auto enemy : enemyManager->fGetEnemies())
		{
			if (enemy->fGetStun() == false && enemy->fGetDissolve() <= 0)enemy->fSetStun(true);
		}
		//スタンしている敵が5秒間いなかったら
		if (is_stun_timer > 5.0f)
		{
			if (change_text == false)
			{
				tutorial_text_element[5].position = { 22.0f,72.0f };
				tutorial_text_element[5].s = L"ロックオン中に一定の近さの時に回避をすると回り込み,スタンさせます";
				change_text = true;
			}
		}
		else
		{
			if (change_text)
			{
				tutorial_text_element[5].position = { 30.0f,49.0f };
				tutorial_text_element[5].s = L"LBボタンを押し続けるとカメラ内にいるスタンしている敵をロックオンして\n一度に攻撃することができます";
				change_text = false;
			}
		}

		if (is_next)
		{
			Judea_timer += 1.0f * elapsed_time;
			if (Judea_timer > 1.0f)
			{
				//次のステートに設定
				tutorial_state = TutorialState::AwaikingTutorial;
				//次に進むフラグの初期化
				is_next = false;
				//プレイヤーの次に進むフラグを初期化
				player->FalseNextTutorial();
				//猶予時間を初期化
				Judea_timer = 0;
				//ディゾルブ時間を初期化
				check_mark_parm.threshold = 1.0f;
				//ディゾルブしていいかどうかのフラグを初期化
				check_mark_parm.is_threshold = false;
				for (int i = 0; i < 50; ++i)
				{
					bool fa = false;
					player->AddCombo(2,fa);
				}
			}
		}

		// 敵がわいてくる
		enemyManager->fSpawnTutorial_NoAttack(elapsed_time, graphics);

		break;
	case TutorialScene::TutorialState::AwaikingTutorial:
		button_priset = BottunPriset::AwaikingTutorialPriset;
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		tutorial_check_text = L"Aボタンを押して覚醒";
		sprite_tutorial_text.position = { 359.0f,408.0f };
		sprite_tutorial_text.s = L"覚醒状態の時は敵がスタンしていない場合でも\nロックオンをしチェイン攻撃を行うことができます";
		arrow_rate += 1.0f * elapsed_time;
		if (arrow_move_change)
		{
			if (arrow_rate > 1.0f)
			{
				arrow_rate = 0.0f;
				arrow_move_change = false;
			}
			else
			{
				arrow_mark_pram.position = Math::lerp({ 397.4f,216.0f }, { 467.2f,236.6f }, arrow_rate);
			}
		}
		else
		{
			if (arrow_rate > 1.0f)
			{
				arrow_rate = 0.0f;
				arrow_move_change = true;
			}
			else
			{
				arrow_mark_pram.position = Math::lerp({ 467.2f,236.6f }, { 397.4f,216.0f }, arrow_rate);
			}
		}
		if (is_next)
		{
			//ジャスト回避の説明をする
			sprite_tutorial = true;
			//タイマーを進める
			sprite_tutorial_timer += 1.0f * elapsed_time;
			if (sprite_tutorial_timer > 1.0f && game_pad->get_button_down() & GamePad::BTN_B)
			{
				end_sprite_tutorial = true;
			}
		}

		if (end_sprite_tutorial)
		{
			Judea_timer += 1.0f * elapsed_time;
			if (Judea_timer > 1.0f)
			{
				//次のステートに設定
				tutorial_state = TutorialState::FreePractice;
				//次に進むフラグの初期化
				is_next = false;
				//プレイヤーの次に進むフラグを初期化
				player->FalseNextTutorial();
				//猶予時間を初期化
				Judea_timer = 0;
				//ディゾルブ時間を初期化
				check_mark_parm.threshold = 1.0f;
				//ディゾルブしていいかどうかのフラグを初期化
				check_mark_parm.is_threshold = false;
				//画像の説明のフラグを初期化
				sprite_tutorial = false;
				end_sprite_tutorial = false;
				sprite_tutorial_timer = 0.0f;

			}
		}

		// 敵がわいてくる
		enemyManager->fSpawnTutorial_NoAttack(elapsed_time, graphics);

		break;
	case TutorialScene::TutorialState::FreePractice:
		player->ChangeTutorialState(static_cast<int>(tutorial_state));
		end_tutorial_text_timer += 1.0f * elapsed_time;
		tutorial_check_text = L"自由に練習する";
		//チュートリアル終了の文字の位置を再設定
		check_mark_parm.threshold = 1.0f;
		enemyManager->fSpawnTutorial(elapsed_time, graphics);

		//チュートリアルが最後までいったら何秒間かは上のフレームにチュートリアル終了を表示させておくため
		if (end_tutorial_text_timer > 10.0f)
		{
			change_gauge_parm.pos = { 62.3f,439.6f };
			change_scene_txt.position = { 88.0f,445.1f };
			check_mark_parm.pos = { 80.5f,329.5f };
			tutorial_check_text_parm.position = { 151.4f,332.3f };
			end_tutorial_text = true;
			free_practice = true;
		}
		break;
	default:
		break;
	}
	back_button_rate += 0.5f * elapsed_time;
	if (back_button_move_change)
	{
		if (back_button_rate > 1.0f)
		{
			back_button_rate = 0.0f;
			back_button_move_change = false;
		}
		else
		{
			controller_back_button_pram.color.w = Math::lerp(1.0f, 0.0f, back_button_rate);
		}
	}
	else
	{
		if (back_button_rate > 1.0f)
		{
			back_button_rate = 0.0f;
			back_button_move_change = true;
		}
		else
		{
			controller_back_button_pram.color.w = Math::lerp(0.0f, 1.0f, back_button_rate);
		}
	}

	//バックボタンを長押しして3秒たったらチュートリアルスキップ
	if (game_pad->get_button() & GamePad::BTN_BACK)
	{
		change_scene_timer += 1.0f * elapsed_time ;
		change_gauge_parm.threshold -=(2.0f * elapsed_time) / 2.0f;
		if (change_scene_timer > 1.4f)
		{
			SceneManager::scene_switching(new SceneLoading(new SceneGame()), DISSOLVE_TYPE::DOT, 2.0f);
		}
	}
	else
	{
		change_gauge_parm.threshold = 1.0f;
		change_scene_timer = 0;
	}

}

void TutorialScene::TutorialRender(GraphicsPipeline& graphics, float elapsed_time)
{
	graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	auto sprite_render = [&](std::string gui_name, SpriteBatch* batch, Element& e, float glow_horizon = 0, float glow_vertical = 0)
	{
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
		//--sprite_string--//
#ifdef USE_IMGUI
		ImGui::Begin("title");
		if (ImGui::TreeNode(gui_name.c_str()))
		{
			ImGui::DragFloat2("pos", &e.position.x, 0.1f);
			ImGui::DragFloat("angle", &e.angle, 0.1f);
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
	auto r_font_render = [&](std::string name, StepFontElement& e)
	{
#ifdef USE_IMGUI
		ImGui::Begin(name.c_str());
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

	if (end_tutorial_text == false)
	{
		//ここで-1してるのは1から始まっているから
		if (StepString(elapsed_time, tutorial_text_element[static_cast<int>(tutorial_state) - 1])) is_end_text = true;
		else is_end_text = false;
		glow_vertical -= elapsed_time * 0.2f;

		sprite_render("frame", sprite_tutorial_frame.get(), frame_pram, 0, glow_vertical);


		fonts->yu_gothic->Begin(graphics.get_dc().Get());
		r_font_render("tutorial_text", tutorial_text_element[static_cast<int>(tutorial_state) - 1]);
		fonts->yu_gothic->End(graphics.get_dc().Get());

		sprite_render("controller_base", controller_base.get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::A_)sprite_render("controller_base", controller_on_keys[ControllerSprite::A].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::A].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::B_)sprite_render("controller_base", controller_on_keys[ControllerSprite::B].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::B].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::X_)sprite_render("controller_base", controller_on_keys[ControllerSprite::X].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::X].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::Y_)sprite_render("controller_base", controller_on_keys[ControllerSprite::Y].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::Y].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::RT_)sprite_render("controller_base", controller_on_keys[ControllerSprite::RT].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::RT].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::RB_)sprite_render("controller_base", controller_on_keys[ControllerSprite::RB].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::RB].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::LT_)sprite_render("controller_base", controller_on_keys[ControllerSprite::LT].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::LT].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::LB_)sprite_render("controller_base", controller_on_keys[ControllerSprite::LB].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::LB].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::RightStick_)sprite_render("controller_base", controller_on_keys[ControllerSprite::RightStick].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::RightStick].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::LeftStick_)sprite_render("controller_base", controller_on_keys[ControllerSprite::LeftStick].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::LeftStick].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::Cross_)sprite_render("controller_base", controller_keys[ControllerSprite::Cross].get(), controller_pram, 0, 0);
		if(button_priset & BottunPriset::Menu_)sprite_render("controller_base", controller_on_keys[ControllerSprite::Menu].get(), controller_pram, 0, 0);
		else sprite_render("controller_base", controller_keys[ControllerSprite::Menu].get(), controller_pram, 0, 0);
		sprite_render("controller_back_pram", controller_on_keys[ControllerSprite::Back].get(), controller_pram, 0, 0);

	}
#ifdef USE_IMGUI
	static bool display_scape_imgui;
	imgui_menu_bar("Tutorial", "Tutorial", display_scape_imgui);
	if (display_scape_imgui)
	{
		ImGui::Begin("Tutorial");
		if (ImGui::TreeNode("check_mark_parm"))
		{
			ImGui::DragFloat2("pos", &check_mark_parm.pos.x, 0.1f);
			ImGui::DragFloat2("scale", &check_mark_parm.scale.x, 0.1f);
			ImGui::DragFloat("threshold", &check_mark_parm.threshold, 0.01f);
			ImGui::Checkbox("is_threshold", &check_mark_parm.is_threshold);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("tutorial_check_text_parm"))
		{
			ImGui::DragFloat2("pos", &tutorial_check_text_parm.position.x, 0.1f);
			ImGui::DragFloat2("scale", &tutorial_check_text_parm.scale.x, 0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("change_gauge_parm"))
		{
			ImGui::DragFloat2("pos", &change_gauge_parm.pos.x, 0.1f);
			ImGui::DragFloat2("scale", &change_gauge_parm.scale.x, 0.1f);
			ImGui::DragFloat("threshold", &change_gauge_parm.threshold, 0.01f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("change_scene_txt"))
		{
			ImGui::DragFloat2("pos", &change_scene_txt.position.x, 0.1f);
			ImGui::DragFloat2("scale", &change_scene_txt.scale.x, 0.1f);
			ImGui::DragFloat4("color", &change_scene_txt.color.x, 0.1f);
			ImGui::TreePop();
		}
		ImGui::End();
	}
#endif // USE_IMGUI

	change_scene_gauge->begin(graphics.get_dc().Get());
	change_scene_gauge->render(graphics.get_dc().Get(), change_gauge_parm.pos, change_gauge_parm.scale,change_gauge_parm.threshold);
	change_scene_gauge->end(graphics.get_dc().Get());

	sprite_render("controller_back_button_pram", controller_on_keys[ControllerSprite::Back].get(), controller_back_button_pram, 0, 0);

	fonts->yu_gothic->Begin(graphics.get_dc().Get());
	fonts->yu_gothic->Draw(tutorial_skip_text, change_scene_txt.position, change_scene_txt.scale, change_scene_txt.color, change_scene_txt.angle, TEXT_ALIGN::UPPER_LEFT);
	fonts->yu_gothic->End(graphics.get_dc().Get());


	check_box->begin(graphics.get_dc().Get());
	check_box->render(graphics.get_dc().Get(), check_mark_parm.pos, check_mark_parm.scale);
	check_box->end(graphics.get_dc().Get());

	fonts->yu_gothic->Begin(graphics.get_dc().Get());
	fonts->yu_gothic->Draw(tutorial_check_text, tutorial_check_text_parm.position, tutorial_check_text_parm.scale, tutorial_check_text_parm.color, tutorial_check_text_parm.angle, TEXT_ALIGN::UPPER_LEFT);
	fonts->yu_gothic->End(graphics.get_dc().Get());

	check_mark->begin(graphics.get_dc().Get());
	check_mark->render(graphics.get_dc().Get(), check_mark_parm.pos, check_mark_parm.scale, check_mark_parm.threshold);
	check_mark->end(graphics.get_dc().Get());

	//回り込み回避の時とチェイン攻撃の時にしかうつさない
	if (tutorial_state == TutorialState::BehindAvoidanceTutorial || tutorial_state == TutorialState::ChainAttackTutorial)
	{
		//ChainAttackTutorialの時にスタンしている敵がいない時間が5秒以上の時はうつさない
		if (is_stun_timer < 5.0f)
		{
			fonts->yu_gothic->Begin(graphics.get_dc().Get());
			r_font_render("tutorial_count_text", tutorial_count_text);
			fonts->yu_gothic->End(graphics.get_dc().Get());
		}
	}
	if (tutorial_state == TutorialState::AwaikingTutorial)
	{
		sprite_render("arrow_mark", arrow_mark.get(), arrow_mark_pram, 0);
	}
	//画像のチュートリアルの時
	if (sprite_tutorial)
	{
		glow_vertical -= elapsed_time * 0.2f;
		b_bottun_rate += 1.0f * elapsed_time;
		if (b_button_change)
		{
			if (b_bottun_rate > 1.0f)
			{
				b_bottun_rate = 0.0f;
				b_button_change = false;
			}
			else
			{
				controller_b_pram.position = Math::lerp({ 365.7f,270.2f }, { 365.7f,289.6f }, b_bottun_rate);
			}
		}
		else
		{
			if (b_bottun_rate > 1.0f)
			{
				b_bottun_rate = 0.0f;
				b_button_change = true;
			}
			else
			{
				controller_b_pram.position = Math::lerp({ 365.7f,289.6f }, { 365.7f,270.2f }, b_bottun_rate);
			}
		}
		sprite_render("back", brack_back.get(), brack_back_pram, 0);
		sprite_render("sprite_frame", sprite_frame.get(), sprite_frame_parm, 0, glow_vertical);
		if(button_priset == BottunPriset::BehindAvoidanceTutorialPriset)sprite_render("just_avoidance", just_avoidance.get(), just_avoidance_parm, 0, 0);
		if(button_priset == BottunPriset::AwaikingTutorialPriset)sprite_render("awaiking_chain", awaiking_chain.get(), awaiking_chain_parm, 0, 0);
		sprite_render("controller_b_pram", controller_keys[ControllerSprite::B].get(), controller_b_pram);
		fonts->yu_gothic->Begin(graphics.get_dc().Get());
		r_font_render("sprite_tutorial_text", sprite_tutorial_text);
		fonts->yu_gothic->End(graphics.get_dc().Get());
	}

}

bool TutorialScene::StepString(float elapsed_time, StepFontElement& step_font_element, bool loop)
{
	step_font_element.timer += elapsed_time * step_font_element.speed;
	step_font_element.step = static_cast<int>(step_font_element.timer);
	size_t size = step_font_element.tutorial_text.size();
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
			step_font_element.s += step_font_element.tutorial_text[step_font_element.index];
			step_font_element.step = 1;
			step_font_element.timer = 1.0f;
		}
		++step_font_element.index;
	}

	return false;
}
