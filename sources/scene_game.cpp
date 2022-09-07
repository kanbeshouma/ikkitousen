#include <memory>

#include "scene_game.h"
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

void SceneGame::initialize(GraphicsPipeline& graphics)
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
	// effect
	test_effect = std::make_unique<Effect>(graphics, effect_manager->get_effekseer_manager(), ".\\resources\\Effect\\enemy_vernier.efk");

	//--------------------<弾の管理クラスを初期化>--------------------//
	BulletManager& mBulletManager = BulletManager::Instance();
	mBulletManager.fInitialize();
	//--------------------<敵の管理クラスを初期化>--------------------//
	mWaveManager.fInitialize(graphics,mBulletManager.fGetAddFunction());

	// player
	player = std::make_unique<Player>(graphics);
	// カメラ
	//camera = std::make_unique<Camera>(graphics,player.get());
	//std::vector<int> cameraType{};
	//cameraType.push_back(CameraManager::CameraTypes::Game);
	//cameraManager = std::make_unique<CameraManager>(graphics, player.get(), cameraType);
	//cameraManager->Initialize(graphics, player.get());
	cameraManager = std::make_unique<CameraManager>();

	cameraManager->RegisterCamera(new GameCamera(player.get()));
	cameraManager->RegisterCamera(new ClearCamera(player.get()));
	cameraManager->RegisterCamera(new JointCamera(graphics));

	//cameraManager->SetCamera(static_cast<int>(CameraTypes::Game));
	//cameraManager->Initialize(graphics);
	cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));

	last_boss_mode     = LastBoss::Mode::None;
	old_last_boss_mode = LastBoss::Mode::None;

	audio_manager->stop_all_bgm();
	audio_manager->play_bgm(BGM_INDEX::GAME);
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
}

void SceneGame::uninitialize()
{
	audio_manager->stop_all_se();
	audio_manager->stop_all_bgm();

	BulletManager& mBulletManager = BulletManager::Instance();

	mWaveManager.fFinalize();
	mBulletManager.fFinalize();
}

void SceneGame::effect_liberation(GraphicsPipeline& graphics)
{
	effect_manager->finalize();
	effect_manager->initialize(graphics);
}

void SceneGame::update(GraphicsPipeline& graphics, float elapsed_time)
{
	if (is_game_clear == false && is_game_over == false)
	{
		brack_back_pram.color.w = 0;
		is_set_black = false;
	}

	//ゲームクリアになったら
	GameClearAct(elapsed_time,graphics);
	//ゲームオーバーになったら
	GameOverAct(elapsed_time);
	const float bgm_volume = 2.0f;
	const float se_volume = 2.0f;
	audio_manager->set_all_volume_bgm(bgm_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_bgm_volume());
	audio_manager->set_all_volume_se(se_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_se_volume());

	//ゲームオーバー,ゲームクリアの時は止める
	if (is_game_over) return;
	if (is_game_clear) return;
	// ゲームクリア
	if (mWaveManager.get_game_clear()) { is_game_clear = true; }

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
			audio_manager->play_se(SE_INDEX::SELECT);
			option->initialize();
			option->set_validity(true);
			return;
		}
	}
	BulletManager& mBulletManager = BulletManager::Instance();

	// クリア演出
	mWaveManager.fUpdate(graphics, elapsed_time, mBulletManager.fGetAddFunction());
	if (mWaveManager.during_clear_performance())
	{
		tunnel_alpha += elapsed_time * 0.5f;
		tunnel_alpha = (std::min)(tunnel_alpha, 1.0f);

		if (!during_clear)
		{
			player->SetPosition({ 0.0f,0.0f,20.0f });
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
				player->TransitionStageMoveEnd();
				tunnel_alpha = 0.0f;
				during_clear = false;
			}
		}
	}
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
		//タイマーが1.0秒以下なら遅くする
		if (slow_timer < 1.0f)
		{
			elapsed_time *= slow_rate;
		}
	}

	//--------------------<敵の管理クラスの更新処理>--------------------//

	// enemyManagerのupdateの上で
	old_last_boss_mode = last_boss_mode;

    const auto enemyManager = mWaveManager.fGetEnemyManager();

	if(enemyManager->fGetSlow())
	{
		//elapsed_time *= 0.5f;
	}

	enemyManager->fSetPlayerPosition(player->GetPosition());
	enemyManager->fSetPlayerSearch(player->during_search_time());
	mBulletManager.fUpdate(elapsed_time);

	// ↓↓↓↓↓↓↓↓↓プレイヤーの更新はこのした↓↓↓↓↓
    BaseEnemy* enemy = enemyManager->fGetNearestEnemyPosition();
	Camera* c = cameraManager->GetCurrentCamera();

	if (player->GetIsAlive() == false)	is_game_over = true;

	
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
			)
			,block);
		}
	}
    const bool isCounter= enemyManager->fCalcEnemiesAttackVsPlayerCounter(
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

	//--------------------< ボスのBGM切り替え&スカイボックスの色変える >--------------------//
	last_boss_mode = enemyManager->fGetBossMode();

	// 再挑戦
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::ShipToHuman)
	{
		c->boss_animation = true;
		player->SetPosition({ 0,0,-120.0f });
		//プレイヤーの行動範囲変更
		player->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_HUMANOID);
		purple_threshold = 0.01f;
	}
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::HumanToDragon)
	{
		c->boss_animation = true;
		player->SetPosition({ 0,0,-120.0f });
		//プレイヤーの行動範囲変更
		player->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_DRAGON);
		red_threshold = 0.01f;
	}
	// 通常時
	if (old_last_boss_mode == LastBoss::Mode::None && last_boss_mode == LastBoss::Mode::ShipAppear)
	{
		c->boss_animation = true;
		player->SetPosition({ 0,0,-120.0f });
		//プレイヤーの行動範囲変更.
		player->ChangePlayerJustificationLength();
		audio_manager->stop_all_bgm();
		audio_manager->play_bgm(BGM_INDEX::BOSS_BATTLESHIP);
	}
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
	// SkyDome
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

	//--------------------<ボスの方にカメラを向ける処理>--------------------//
	if(enemyManager->fGetIsEventCamera()&&!mIsBossCamera)
	{
		post_effect->clear_post_effect();
	    // まだボスのほうを向いていないとき
		mIsBossCamera = true;
		cameraManager->SetCamera(static_cast<int>(CameraTypes::Joint));
	}
	if(enemyManager->fGetIsEventCamera() && mIsBossCamera)
	{
		const DirectX::XMFLOAT3 eye = enemyManager->fGetEye();
		const DirectX::XMFLOAT3 focus = enemyManager->fGetFocus();

	    // カメラがEnemyManagerを経由し555たボスによって更新される
		cameraManager->GetCurrentCamera()->set_eye(eye);
		cameraManager->GetCurrentCamera()->set_target(focus);

		// 敵の移動を制限
		enemyManager->fLimitEnemies();
	}
	if (!enemyManager->fGetIsEventCamera() && mIsBossCamera)
	{
	    // カメラ処理終了
		mIsBossCamera = false;
		cameraManager->SetCamera(static_cast<int>(CameraTypes::Game));
	}


	// camera
    //camera->Update(elapsed_time,player.get());
	cameraManager->Update(elapsed_time);
	player->SetBossCamera(mIsBossCamera);
		player->SetCameraDirection(c->GetForward(), c->GetRight());
		player->Update(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());
		//player->UpdateTutorial(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());
		player->lockon_post_effect(elapsed_time, [=](float scope, float alpha) { post_effect->lockon_post_effect(scope, alpha); },
			[=]() { post_effect->clear_post_effect(); });
		player->SetCameraPosition(c->get_eye());
		player->SetTarget(enemy);
		player->SetCameraTarget(c->get_target());
		if (player->GetStartDashEffect()) post_effect->dash_post_effect(graphics.get_dc().Get(), player->GetPosition());
	enemy_hp_gauge->update(graphics, elapsed_time);
	enemy_hp_gauge->focus(player->GetPlayerTargetEnemy(), player->GetEnemyLockOn());
	boss_hp_gauge->update(graphics, elapsed_time);
	if (last_boss_mode == LastBoss::Mode::None) { boss_hp_gauge->set_animation(false); }
	else { boss_hp_gauge->set_animation(true); }

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


	//camera->update_with_quaternion(elapsed_time);
	// shadow_map
	shadow_map->debug_imgui();

	effect_manager->update(elapsed_time);

	// effect demo
#ifdef USE_IMGUI
	{
		ImGui::Begin("effect");
		if (ImGui::Button("play_effect"))
		{
			test_effect->play(effect_manager->get_effekseer_manager(), { 0,1,0 }, 3);
		}
		if (ImGui::Button("stop_effect"))
		{
			test_effect->stop(effect_manager->get_effekseer_manager());
		}
		{
			static DirectX::XMFLOAT3 pos{};
			ImGui::DragFloat3("position", &pos.x, 0.1f);
			test_effect->set_position(effect_manager->get_effekseer_manager(), pos);
			static DirectX::XMFLOAT3 scale{1,1,1};
			ImGui::DragFloat3("scale", &scale.x, 0.1f);
			test_effect->set_scale(effect_manager->get_effekseer_manager(), scale);
		}
		ImGui::End();
	}
#endif // USE_IMGUI
	// camera shake demo
#ifdef USE_IMGUI
	static bool is_shake = false;
	ImGui::Begin("camera_shake");
	if (ImGui::Checkbox("shake", &is_shake))
	{
		camera_shake->reset(graphics);
	}
	if (is_shake) camera_shake->shake(graphics, elapsed_time);
	ImGui::End();
#endif // USE_IMGUI

	// hit_stop demo
#ifdef USE_IMGUI
	{
		ImGui::Begin("hit_stop");
		if (ImGui::Button("stop")) { hit_stop->damage_hit_stop(); }
		ImGui::End();
	}
#endif // USE_IMGUI


	//****************************************************************
	//
	// オブジェクトの削除処理はこの下でやるルール
	//
	//****************************************************************
	//enemyManager->fDeleteEnemies();
}

#define OFF_SCREEN_RENDERING
#define SHADOW_MAP

void SceneGame::render(GraphicsPipeline& graphics, float elapsed_time)
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
		sky_dome->Render(graphics,elapsed_time);


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
	mWaveManager.fGetEnemyManager()->fRender(graphics);
	if(mIsBossCamera == false)player->Render(graphics, elapsed_time);
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
	//wave->render(graphics.get_dc().Get());
	Camera* c = cameraManager->GetCurrentCamera();
	const DirectX::XMFLOAT2 p_pos = { player->GetPosition().x,player->GetPosition().z };
	const DirectX::XMFLOAT2 p_forward = { player->GetForward().x,player->GetForward().z };
	const DirectX::XMFLOAT2 c_forward = { c->GetForward().x,c->GetForward().z };
	if (mIsBossCamera == false)player->ConfigRender(graphics, elapsed_time);
    if (mIsBossCamera == false && during_clear == false && is_game_clear == false)
	{
		minimap->render(graphics, p_pos, p_forward, c_forward, mWaveManager.fGetEnemyManager()->fGetEnemies());
	}
	mWaveManager.render(graphics.get_dc().Get(), elapsed_time);
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
	if (is_game_over)
	{
		glow_vertical -= elapsed_time * 0.2f;

	    sprite_render("game_over_sprite", game_over_sprite.get(), game_over_sprite_pram, 0, 0);
		sprite_render("back", brack_back.get(), brack_back_pram, 0, glow_vertical);
		//画面が黒くなったら
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
	if (option->get_validity()) { option->render(graphics, elapsed_time); }

}

void SceneGame::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
	return;
#ifdef SHADOW_MAP
	Camera* c = cameraManager->GetCurrentCamera();
	//--シャドウマップの生成--//
	shadow_map->activate_shadowmap(graphics, c->get_light_direction());







	//--元のビューポートに戻す--//
	shadow_map->deactivate_shadowmap(graphics);
#endif // SHADOW_MAP
}

void SceneGame::GameOverAct(float elapsed_time)
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
					&& game_over_sprite_pram.texpos.y < 512.0f )
				{
					audio_manager->play_se(SE_INDEX::DRAW_PEN);
					game_over_sprite_pram.texpos.x = 0.0f;
					game_over_sprite_pram.texpos.y += 512.0f;
				}

			}
			if(is_game_over_sprite)brack_back_pram.color.w += 1.0f * elapsed_time;
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
					SceneManager::scene_switching(new SceneLoading(new SceneGame()), DISSOLVE_TYPE::TYPE1, 2.0f);
				}
				break;
			default:
				break;
			}
		}
	}
}

void SceneGame::GameClearAct(float elapsed_time,GraphicsPipeline& graphics)
{
	if (is_game_clear)
	{
		//プレイヤーのクリア用の更新処理
		const auto enemyManager = mWaveManager.fGetEnemyManager();
		cameraManager->Update(elapsed_time);
		player->PlayerClearUpdate(elapsed_time, graphics, sky_dome.get(), enemyManager->fGetEnemies());

		if (set_joint_camera == false)
		{
			cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Joint));
			set_joint_camera = true;
		}
		if (set_joint_camera)
		{
			cameraManager->GetCurrentCamera()->set_eye(player->GetEnentCameraJoint());
			cameraManager->GetCurrentCamera()->set_target(player->GetEnentCameraEye());
		}
		//ここでタイトルに戻るの位置を決めているのは
	   //ゲームオーバーの時にも同じものを使うから
		back_title.position = { 529.7f,246.3f };
		back_title.scale = { 1.0f,1.0f };
		//セレクターの位置を入れる
		selecter1.position = { 495.3f,277.1f };
		selecter2.position = { 801.5f,277.1f };
		//プレイヤーのクリアモーションが終わってからしか動かないようにする
		if (player->GetEndClearMotion())
		{
			//画面ヲ徐々に黒くする
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
