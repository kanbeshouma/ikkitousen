#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <string>
#include <filesystem>
#include <fstream>

#include "game_icon.h"
#include "Operators.h"
#include "collision.h"
#include "scene_option.h"

void GameFile::load()
{
	std::filesystem::path path = file_name;
	path.replace_extension(".json");
	if (std::filesystem::exists(path.c_str()))
	{
		std::ifstream ifs;
		ifs.open(path);
		if (ifs)
		{
			cereal::JSONInputArchive o_archive(ifs);
			o_archive(source);
		}
	}
	else
	{
		source.initialize();
	}
}

void GameFile::save()
{
	std::filesystem::path path = file_name;
	path.replace_extension(".json");
	std::ofstream ifs(path);
	if (ifs)
	{
		cereal::JSONOutputArchive o_archive(ifs);
		o_archive(source);
	}
}


GameIcon::GameIcon(ID3D11Device* device) : IconBase(device)
{
	GameFile::get_instance().load();
	//--shake--//
	shake.position = { 542.0f, 295.0f };
	shake.scale = { 0.9f, 0.9f };
	shake.s = L"カメラシェイク";
	//--vibration--//
	vibration.position = { 563.0f, 380.0f };
	vibration.scale = { 0.9f, 0.9f };
	vibration.s = L"コントローラー振動";
	//--operation--//
	operation.position = { 695.0f, 515.0f };
	operation.scale = { 0.9f, 0.9f };
	operation.s = L"操作確認";

	operation_back.position = { 1128.0f, 682.0f };
	operation_back.scale = { 0.9f, 0.9f };
	operation_back.s = L"Aボタンで戻る";

	display_operation = false;
	sprite_operation = std::make_unique<SpriteBatch>(device, L".\\resources\\Sprites\\option\\con_option.png", 1);
	sprite_mask      = std::make_unique<SpriteBatch>(device, L".\\resources\\Sprites\\mask\\white_mask.png", 1);

	//--button--//
	float on_pos_x = 820.0f; float off_pos_x = 1027.0f;
	// SHAKE
	choices[ChoicesType::SHAKE][0].position = { on_pos_x, shake.position.y };
	choices[ChoicesType::SHAKE][0].scale = { shake.scale };
	choices[ChoicesType::SHAKE][0].s = L"ON";
	choices[ChoicesType::SHAKE][1].position = { off_pos_x, shake.position.y };
	choices[ChoicesType::SHAKE][1].scale = { shake.scale };
	choices[ChoicesType::SHAKE][1].s = L"OFF";
	setup[ChoicesType::SHAKE] = GameFile::get_instance().get_shake();
	// VIBRATION
	choices[ChoicesType::VIBRATION][0].position = { on_pos_x, vibration.position.y };
	choices[ChoicesType::VIBRATION][0].scale = { vibration.scale };
	choices[ChoicesType::VIBRATION][0].s = L"ON";
	choices[ChoicesType::VIBRATION][1].position = { off_pos_x, vibration.position.y };
	choices[ChoicesType::VIBRATION][1].scale = { vibration.scale };
	choices[ChoicesType::VIBRATION][1].s = L"OFF";
	setup[ChoicesType::VIBRATION] = GameFile::get_instance().get_vibration();
	// selecter
	DirectX::XMFLOAT2 selecter_texsize = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
	DirectX::XMFLOAT2 selecter_pivot = { selecter_texsize * DirectX::XMFLOAT2(0.5f, 0.5f) };
	float selecter_posL[2] = { 730.0f,900.0f }; float selecter_posR[2] = { 855.0f, 1055.0f };
	selecter[ChoicesType::SHAKE][0].position     = { selecter_posL[setup[ChoicesType::SHAKE] ? 0 : 1], shake.position.y };
	selecter[ChoicesType::SHAKE][1].position     = { selecter_posR[setup[ChoicesType::SHAKE] ? 0 : 1], shake.position.y };
	selecter[ChoicesType::VIBRATION][0].position = { selecter_posL[setup[ChoicesType::VIBRATION] ? 0 : 1], vibration.position.y };
	selecter[ChoicesType::VIBRATION][1].position = { selecter_posR[setup[ChoicesType::VIBRATION] ? 0 : 1], vibration.position.y };
	for (int i = 0; i < BUTTON_COUNT; ++i)
	{
		for (int o = 0; o < 2; ++o)
		{
			selecter[i][o].texsize = selecter_texsize;
			selecter[i][o].pivot = selecter_pivot;
			selecter[i][o].scale = { 0.3f, 0.3f };
			selecter[i][o].color = { 1,1,1,1 };
			selecter_arrival_pos[i][o] = { selecter[i][o].position };
		}
	}

	//--selecterL--//
	selecterL.position = { 290.0f, shake.position.y };
	selecterL.scale = { 0.3f, 0.3f };
	//--selecterR--//
	selecterR.position = { 600.0f, shake.position.y };
	selecterR.scale = { 0.3f, 0.3f };

	selecterL_arrival_pos = selecterL.position;
	selecterR_arrival_pos = selecterR.position;
}

GameIcon::~GameIcon() {}

void GameIcon::update(GraphicsPipeline& graphics, float elapsed_time)
{
	auto r_up = [&](ChoicesType type, const DirectX::XMFLOAT2& arrival_pos_L, const DirectX::XMFLOAT2& arrival_pos_R)
	{
		if (!display_operation && (game_pad->get_button_down() & GamePad::BTN_UP))
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = type;
			selecterL_arrival_pos = arrival_pos_L;
			selecterR_arrival_pos = arrival_pos_R;
		}
	};
	auto r_down = [&](ChoicesType type, const DirectX::XMFLOAT2& arrival_pos_L, const DirectX::XMFLOAT2& arrival_pos_R)
	{
		if (!display_operation && (game_pad->get_button_down() & GamePad::BTN_DOWN))
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = type;
			selecterL_arrival_pos = arrival_pos_L;
			selecterR_arrival_pos = arrival_pos_R;
		}
	};
	auto r_button = [&](ChoicesType type)
	{
		float selecter_posL[2] = { 730.0f,900.0f }; float selecter_posR[2] = { 855.0f, 1055.0f };
		if (game_pad->get_button_down() & GamePad::BTN_LEFT)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			if (!setup[type])
			{
				setup[type] = true;
				selecter_arrival_pos[type][0].x = selecter_posL[0];
				selecter_arrival_pos[type][1].x = selecter_posR[0];
			}
			save_source();
		}
		if (game_pad->get_button_down() & GamePad::BTN_RIGHT)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			if (setup[type])
			{
				setup[type] = false;
				selecter_arrival_pos[type][0].x = selecter_posL[1];
				selecter_arrival_pos[type][1].x = selecter_posR[1];
			}
			save_source();
		}
	};

	// stateがChoicesType::OPERATIONの時にon offにカーソルを合わせて決定すると
	// display_operationがtrueになってしまうのでその対処
	if (state != ChoicesType::OPERATION)
	{
		display_operation = false;
		Option::set_home_disabled(false);
	}

	switch (state)
	{
	case ChoicesType::SHAKE:
		r_down(ChoicesType::VIBRATION, { 255.0f, vibration.position.y }, { 640.0f, vibration.position.y });
		r_button(ChoicesType::SHAKE);
		break;

	case ChoicesType::VIBRATION:
		r_up(ChoicesType::SHAKE, { 290.0f, shake.position.y }, { 600.0f, shake.position.y });
		r_down(ChoicesType::OPERATION, { 540.0f, operation.position.y }, { 795.0f, operation.position.y });
		r_button(ChoicesType::VIBRATION);
		break;

	case ChoicesType::OPERATION:
		r_up(ChoicesType::VIBRATION, { 255.0f, vibration.position.y }, { 640.0f, vibration.position.y });
		// ボタン押したら一枚絵表示
		if (game_pad->get_button_down() & GamePad::BTN_B) { display_operation = true; }
		if (display_operation && (game_pad->get_button_down() & GamePad::BTN_A)) { display_operation = false; }

		Option::set_home_disabled(display_operation);
		break;
	}

	//--左の選択肢--//
#if 1
	selecterL.position = Math::lerp(selecterL.position, selecterL_arrival_pos, 10.0f * elapsed_time);
	selecterR.position = Math::lerp(selecterR.position, selecterR_arrival_pos, 10.0f * elapsed_time);
#endif // 0
	//--右の選択肢--//
	for (int i = 0; i < BUTTON_COUNT; ++i)
	{
		for (int o = 0; o < 2; ++o)
		{
			selecter[i][o].position = Math::lerp(selecter[i][o].position, selecter_arrival_pos[i][o], 10.0f * elapsed_time);
		}
	}
}

void GameIcon::render(std::string gui, ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos)
{
	IconBase::render(gui, dc, add_pos);

	auto r_sprite_render = [&](std::string name, Element& e, SpriteBatch* s)
	{
#ifdef USE_IMGUI
		ImGui::Begin("option");
		if (ImGui::TreeNode(name.c_str()))
		{
			ImGui::DragFloat2("pos", &e.position.x);
			ImGui::DragFloat2("scale", &e.scale.x, 0.1f);
			ImGui::ColorEdit4("color", &e.color.x);
			ImGui::TreePop();
		}
		ImGui::End();
#endif // USE_IMGUI
		s->render(dc, e.position + add_pos, e.scale, e.pivot, e.color, e.angle, e.texpos, e.texsize);
	};
	auto r_font_render = [&](std::string name, FontElement& e)
	{
#ifdef USE_IMGUI
		ImGui::Begin("option");
		if (ImGui::TreeNode(name.c_str()))
		{
			ImGui::DragFloat2("pos", &e.position.x);
			ImGui::DragFloat2("scale", &e.scale.x, 0.1f);
			ImGui::ColorEdit4("color", &e.color.x);
			ImGui::TreePop();
		}
		ImGui::End();
#endif // USE_IMGUI
		fonts->yu_gothic->Draw(e.s, e.position + add_pos, e.scale, e.color, e.angle, TEXT_ALIGN::MIDDLE, e.length);
	};

	fonts->yu_gothic->Begin(dc);
	r_font_render("shake", shake);
	r_font_render("vibration", vibration);
	r_font_render("operation", operation);
	//--button--//
	for (int i = 0; i < BUTTON_COUNT; ++i)
	{
		for (int o = 0; o < 2; ++o)
		{
			std::string s = "choices" + std::to_string(i) + " " + std::to_string(o);
			r_font_render(s, choices[i][o]);
		}
	}
	fonts->yu_gothic->End(dc);

	sprite_selecter->begin(dc);
	for (int i = 0; i < BUTTON_COUNT; ++i)
	{
		for (int o = 0; o < 2; ++o)
		{
			std::string s = "selecter" + std::to_string(i) + " " + std::to_string(o);
			r_sprite_render(s, selecter[i][o], sprite_selecter.get());
		}
	}
	sprite_selecter->end(dc);

	// 操作確認の描画
	if (display_operation)
	{
		{
			DirectX::XMFLOAT2 texsize = { static_cast<float>(sprite_mask->get_texture2d_desc().Width),
										   static_cast<float>(sprite_mask->get_texture2d_desc().Height) };
			sprite_mask->begin(dc);
			sprite_mask->render(dc, { 640, 360 }, { 1,1 }, { texsize * 0.5f }, { 0,0,0,0.6f }, 0, { 0,0 }, texsize);
			sprite_mask->end(dc);
		}
		{
			DirectX::XMFLOAT2 texsize = { static_cast<float>(sprite_operation->get_texture2d_desc().Width),
										   static_cast<float>(sprite_operation->get_texture2d_desc().Height) };
			sprite_operation->begin(dc);
			sprite_operation->render(dc, { 640, 360 }, { 1,1 }, { texsize * 0.5f }, { 1,1,1,1 }, 0, { 0,0 }, texsize);
			sprite_operation->end(dc);
		}
		fonts->yu_gothic->Begin(dc);
		r_font_render("operation_back", operation_back);
		fonts->yu_gothic->End(dc);
	}
}

void GameIcon::vs_cursor(const DirectX::XMFLOAT2& cursor_pos)
{
	for (int i = 0; i < BUTTON_COUNT; ++i)
	{
		float selecter_posL[2] = { 730.0f,900.0f }; float selecter_posR[2] = { 855.0f, 1055.0f };
		for (int o = 0; o < 2; ++o)
		{
			DirectX::XMFLOAT2 length = choices[i][o].length * DirectX::XMFLOAT2(0.8f, 0.7f);
			DirectX::XMFLOAT2 offset = choices[i][o].length * DirectX::XMFLOAT2(0.6f, 0.0f);
			if (Collision::hit_check_rect(cursor_pos, { 5,5 }, choices[i][o].position - offset, length))
			{
				if (game_pad->get_button_down() & GamePad::BTN_B)
				{
					audio_manager->play_se(SE_INDEX::SELECT);
					float selecter_arrival_pos_y[BUTTON_COUNT]  = { shake.position.y, vibration.position.y };
					float selecterL_arrival_pos_x[BUTTON_COUNT] = { 290.0f, 255.0f };
					float selecterR_arrival_pos_x[BUTTON_COUNT] = { 600.0f, 640.0f };
					state = ChoicesType(i);
					selecterL_arrival_pos = { selecterL_arrival_pos_x[i], selecter_arrival_pos_y[i] };
					selecterR_arrival_pos = { selecterR_arrival_pos_x[i], selecter_arrival_pos_y[i] };

					if (o == 0 && !setup[i]) // on
					{
						setup[i] = true;
						selecter_arrival_pos[i][0].x = selecter_posL[0];
						selecter_arrival_pos[i][1].x = selecter_posR[0];
					}
					if (o == 1 && setup[i]) // off
					{
						setup[i] = false;
						selecter_arrival_pos[i][0].x = selecter_posL[1];
						selecter_arrival_pos[i][1].x = selecter_posR[1];
					}
					save_source();
				}
			}
		}
	}

	// 操作確認へ遷移する当たり判定
	{
		DirectX::XMFLOAT2 center = { operation.position.x - 30, operation.position.y };
		if (Collision::hit_check_rect(cursor_pos, { 5,5 }, center, operation.length * 0.6f))
		{
			if (game_pad->get_button_down() & GamePad::BTN_B)
			{
				audio_manager->play_se(SE_INDEX::SELECT);
				state = ChoicesType::OPERATION;
				selecterL_arrival_pos = { 540.0f, operation.position.y };
				selecterR_arrival_pos = { 795.0f, operation.position.y };
			}
		}
	}
}

void GameIcon::save_source()
{
	GameFile::get_instance().set_shake(setup[ChoicesType::SHAKE]);
	GameFile::get_instance().set_vibration(setup[ChoicesType::VIBRATION]);

	GameFile::get_instance().save();
}