#include "scene_option.h"
#include "scene_title.h"
#include "scene_loading.h"
#include "scene_manager.h"
#include "Operators.h"
#include "collision.h"

bool Option::validity = false;
bool Option::switching = false;
bool Option::home_disabled = false;

Option::Option(GraphicsPipeline& graphics)
{
	//----icon_sprites----//
	{
		SpriteBatch* sprite = new SpriteBatch(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\volume_icon.png", 1);
		icon_sprites.insert(std::make_pair(IconType::VOLUME, sprite));
		sprite = new SpriteBatch(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\game_icon.png", 1);
		icon_sprites.insert(std::make_pair(IconType::GAME, sprite));
		sprite = new SpriteBatch(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\move_icon.png", 1);
		icon_sprites.insert(std::make_pair(IconType::TRANSITION, sprite));
	}
	//----icon_elements----//
	{
		Element element;
		element.texsize = { static_cast<float>(icon_sprites.at(IconType::VOLUME)->get_texture2d_desc().Width),
			                static_cast<float>(icon_sprites.at(IconType::VOLUME)->get_texture2d_desc().Height) };
		element.pivot = element.texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
		element.scale = { 0.25f, 0.25f };
		element.color = { 1,1,1,1 };

		element.position = { 480.0f, 188.0f };
		icon_elements.insert(std::make_pair(IconType::VOLUME, element));
		element.position.x = 610.0f;
		icon_elements.insert(std::make_pair(IconType::GAME, element));
		element.position.x = 740.0f;
		icon_elements.insert(std::make_pair(IconType::TRANSITION, element));
	}
	//----icon_map----//
	{
		IconBase* icon = new VolumeIcon(graphics.get_device().Get());
		icon_map.insert(std::make_pair(IconType::VOLUME, icon));
		icon = new GameIcon(graphics.get_device().Get());
		icon_map.insert(std::make_pair(IconType::GAME, icon));
		icon = new TransitionIcon(graphics.get_device().Get());
		icon_map.insert(std::make_pair(IconType::TRANSITION, icon));
	}
	//--back--//
	sprite_back = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\back.png", 1);
	back.position = { 0, 70 };
	back.texsize = { static_cast<float>(sprite_back->get_texture2d_desc().Width), static_cast<float>(sprite_back->get_texture2d_desc().Height) };
	back.color = { 1.0f,1.0f,1.0f,1.0f };
	//--tab--//
	sprite_tab = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\tab.png", 1);
	tab.position = { 0, 70 };
	// 右端に謎の線が出るので-1
	tab.texsize = { static_cast<float>(sprite_tab->get_texture2d_desc().Width) - 1.0f, static_cast<float>(sprite_tab->get_texture2d_desc().Height) };
	tab.color = { 1.0f,1.0f,1.0f,1.0f };
	//--frame--//
	sprite_frame = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\icon_frame.png", 1);
	if (icon_elements.count(state)) { frame.position = icon_elements.at(state).position; }
	frame_arrival_pos = frame.position;
	frame.texsize = { static_cast<float>(sprite_frame->get_texture2d_desc().Width), static_cast<float>(sprite_frame->get_texture2d_desc().Height) };
	frame.pivot = frame.texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
	frame.color = { 1.0f,1.0f,1.0f,1.0f };
	frame.scale = { 0.25f, 0.25f };
	frame_arrival_scale = frame.scale;

	add_position = { -back.texsize.x * back.scale.x, 0 };
	tab_add_position = { -tab.texsize.x * tab.scale.x, 0 };
	//--cursor--//
	sprite_cursor = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\option\\cursor.png", 1);
	cursor.texsize = { static_cast<float>(sprite_cursor->get_texture2d_desc().Width), static_cast<float>(sprite_cursor->get_texture2d_desc().Height) };
	cursor.pivot = { cursor.texsize * DirectX::XMFLOAT2(0.5f, 0.5f) };
	cursor.position = { 1280.0f / 2.0f, 720.0f / 2.0f };
	cursor.scale = { 0.5f, 0.5f };
	cursor.color = { 1.0f,1.0f,1.0f,1.0f };

	cursor_velocity = {};
}

void Option::initialize()
{
	state = IconType::VOLUME;
	cursor.position = { 1280.0f / 2.0f, 720.0f / 2.0f };
	switching = false;
	home_disabled = false;
}

void Option::update(GraphicsPipeline& graphics, float elapsed_time)
{
	if (!home_disabled)
	{
		if ((game_pad->get_button_down() & GamePad::BTN_START) || (game_pad->get_button_down() & GamePad::BTN_A))
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			switching = true;
			return;
		}
	}

	// tab back 演出
	if (!switching)
	{
		float ep   = 50.0f;
		float rate = 7.0f;
		if (Math::equal_check(tab_add_position.x, 0, ep))
		{
			add_position = Math::lerp(add_position, { 0,0 }, rate * elapsed_time);
		}
		tab_add_position = Math::lerp(tab_add_position, { 0,0 }, rate * elapsed_time);
	}
	else
	{
		float arrival_add_pos_x = -back.texsize.x * back.scale.x;
		float arrival_tab_add_pos_x = -tab.texsize.x * tab.scale.x;
		float ep = 100.0f;
		float rate = 7.0f;
		float tab_rate = 5.0f;
		add_position = Math::lerp(add_position, { arrival_add_pos_x, 0 }, rate * elapsed_time);
		if (Math::equal_check(add_position.x, arrival_add_pos_x, ep))
		{
			tab_add_position = Math::lerp(tab_add_position, { arrival_tab_add_pos_x, 0 }, tab_rate * elapsed_time);
		}
		if (Math::equal_check(tab_add_position.x, arrival_tab_add_pos_x, 30.0f)) { validity = false; }
	}

	if (switching) return;

	//--icon--//
	if (icon_map.count(state))
	{
		icon_map.at(state)->update(graphics, elapsed_time);
		icon_map.at(state)->vs_cursor(cursor.position);
	}

	if (home_disabled) return;

	switch (state)
	{
	case IconType::VOLUME:
		if (game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER) { audio_manager->play_se(SE_INDEX::SELECT); state = IconType::GAME; }
		break;
	case IconType::GAME:
		if (game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER) { audio_manager->play_se(SE_INDEX::SELECT); state = IconType::TRANSITION; }
		if (game_pad->get_button_down() & GamePad::BTN_LEFT_SHOULDER) { audio_manager->play_se(SE_INDEX::SELECT); state = IconType::VOLUME; }
		break;
	case IconType::TRANSITION:
		if (game_pad->get_button_down() & GamePad::BTN_LEFT_SHOULDER) { audio_manager->play_se(SE_INDEX::SELECT); state = IconType::GAME; }
		break;
	}
	// frame移動
	if (icon_elements.count(state)) { frame_arrival_pos = icon_elements.at(state).position; }
	frame.position = Math::lerp(frame.position, frame_arrival_pos, 10 * elapsed_time);
	if (Math::equal_check(frame.scale.x, 0.25f, 0.01f)) { frame_arrival_scale = { 0.3f, 0.3f }; }
	if (Math::equal_check(frame.scale.x, 0.3f, 0.01f)) { frame_arrival_scale = { 0.25f, 0.25f }; }
	frame.scale = Math::lerp(frame.scale, frame_arrival_scale, 3 * elapsed_time);
	//--cursor--//
	float cursor_speed = 700.0f * elapsed_time;
	cursor_velocity = { fabsf(game_pad->get_axis_LX() + FLT_EPSILON) > 0.1f ? game_pad->get_axis_LX() : 0,
		                fabsf(game_pad->get_axis_LY() + FLT_EPSILON) > 0.1f ? -game_pad->get_axis_LY() : 0 };
	cursor.position += DirectX::XMFLOAT2(cursor_speed, cursor_speed) * cursor_velocity;
	cursor.position.x = Math::clamp(cursor.position.x, 20.0f, 1260.0f);
	cursor.position.y = Math::clamp(cursor.position.y, 20.0f, 700.0f);
	//--カーソルでのstate移動--//
	for (int i = 0; i < (int)IconType::ICON_COUNT; ++i)
	{
		DirectX::XMFLOAT2 radius = icon_elements.at((IconType)i).texsize * icon_elements.at((IconType)i).scale * DirectX::XMFLOAT2(0.5f, 0.5f);
		if (Collision::hit_check_rect(cursor.position, { 10,10 }, icon_elements.at((IconType)i).position, radius))
		{
			if (game_pad->get_button_down() & GamePad::BTN_B)
			{
				if (state != IconType::TRANSITION)
				{
					audio_manager->play_se(SE_INDEX::SELECT);
					state = IconType(i);
				}
			}
		}
	}



#ifdef USE_IMGUI
	ImGui::Begin("option");
	if (ImGui::TreeNode("add_position"))
	{
		ImGui::DragFloat2("add_position", &add_position.x);
		ImGui::DragFloat2("tab_add_position", &tab_add_position.x);
		ImGui::TreePop();
	}
	ImGui::End();
#endif // USE_IMGUI
}

void Option::render(GraphicsPipeline& graphics, float elapsed_time)
{
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	auto r_render = [&](std::string gui, Element& e, SpriteBatch* s, const DirectX::XMFLOAT2& add_pos, float glow_horizon = 0, float glow_vertical = 0)
	{
#ifdef USE_IMGUI
		ImGui::Begin("option");
		if (ImGui::TreeNode(gui.c_str()))
		{
			ImGui::DragFloat2("pos", &e.position.x);
			ImGui::DragFloat2("scale", &e.scale.x, 0.1f);
			ImGui::ColorEdit4("color", &e.color.x);
			ImGui::TreePop();
		}
		ImGui::End();
#endif // USE_IMGUI
		s->begin(graphics.get_dc().Get());
		s->render(graphics.get_dc().Get(), e.position + add_pos, e.scale, e.pivot, e.color, e.angle, e.texpos, e.texsize, glow_horizon, glow_vertical);
		s->end(graphics.get_dc().Get());
	};
	//--back--//
	glow_vertical -= elapsed_time * 0.2f;
	r_render("back", back, sprite_back.get(), add_position, 0, glow_vertical);
	//--frame--//
	r_render("frame", frame, sprite_frame.get(), add_position);
	//--icon--//
	r_render("volume",     icon_elements.at(IconType::VOLUME),     icon_sprites.at(IconType::VOLUME).get(), add_position);
	r_render("game",       icon_elements.at(IconType::GAME),       icon_sprites.at(IconType::GAME).get(), add_position);
	r_render("transition", icon_elements.at(IconType::TRANSITION), icon_sprites.at(IconType::TRANSITION).get(), add_position);
	//--fonts--//
	auto r_font_render = [&](std::string name, std::wstring string, DirectX::XMFLOAT2& pos, DirectX::XMFLOAT2& scale)
	{
#ifdef USE_IMGUI
		ImGui::Begin("option");
		if (ImGui::TreeNode(name.c_str()))
		{
			ImGui::DragFloat2("pos", &pos.x);
			ImGui::DragFloat2("scale", &scale.x, 0.1f);
			ImGui::TreePop();
		}
		ImGui::End();
#endif // USE_IMGUI
		fonts->yu_gothic->Draw(string, pos + add_position, scale, { 1,1,1,1 }, 0, TEXT_ALIGN::MIDDLE);
	};
	fonts->yu_gothic->Begin(graphics.get_dc().Get());
	{
		static DirectX::XMFLOAT2 pos{ 420.0f, 170.0f };
		static DirectX::XMFLOAT2 scale{ 1.0f, 1.0f };
		r_font_render("LB", L"LB", pos, scale);
	}
	{
		static DirectX::XMFLOAT2 pos{ 885.0f, 170.0f };
		static DirectX::XMFLOAT2 scale{ 1.0f, 1.0f };
		r_font_render("RB", L"RB", pos, scale);
	}
	fonts->yu_gothic->End(graphics.get_dc().Get());
	//--icons--//
	std::string gui_names[3] = { "volume", "game", "transition" };
	if (icon_map.count(state)) { icon_map.at(state)->render(gui_names[static_cast<int>(state)], graphics.get_dc().Get(), add_position); }
	//--tab--//
	if (!home_disabled) r_render("tab", tab, sprite_tab.get(), tab_add_position);
	//--cursor--//
	if (state != IconType::TRANSITION && !home_disabled) { r_render("cursor", cursor, sprite_cursor.get(), {}); }
	//--一番下--//
	graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
	debug_2D->all_render(graphics.get_dc().Get());
}