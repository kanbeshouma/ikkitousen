#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <string>
#include <filesystem>
#include <fstream>

#include "volume_icon.h"
#include "Operators.h"
#include "collision.h"

void VolumeFile::load()
{
	// Jsonファイルから値を取得
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

void VolumeFile::save()
{
	// Jsonにかきだし
	std::filesystem::path path = file_name;
	path.replace_extension(".json");
	std::ofstream ifs(path);
	if (ifs)
	{
		cereal::JSONOutputArchive o_archive(ifs);
		o_archive(source);
	}
}

VolumeIcon::VolumeIcon(ID3D11Device* device)
    : IconBase(device)
{
	VolumeFile::get_instance().load();
	//--master--//
	master.position = { 401.0f, 300.0f };
	master.scale    = { 0.9f, 0.9f };
	master.s = L"全体音量";
	//--bgm--//
	bgm.position = { 430.0f, 420.0f };
	bgm.scale    = { 0.9f, 0.9f };
	bgm.s = L"BGM音量";
	//--se--//
	se.position = { 420.0f, 540.0f };
	se.scale    = { 0.9f, 0.9f };
	se.s = L"SE音量";

	//--selecterL--//
	selecterL.position = { 255.0f, master.position.y };
	selecterL.scale    = { 0.3f, 0.3f };
	//--selecterR--//
	selecterR.position = { 485.0f, master.position.y };
	selecterR.scale    = { 0.3f, 0.3f };

	selecterL_arrival_pos = { selecterL.position.x, master.position.y };
	selecterR_arrival_pos = { selecterR.position.x, master.position.y };

	//--scales--//
	sprite_scale = std::make_unique<SpriteBatch>(device, L".\\resources\\Sprites\\option\\scale.png", MAX_SCALE_COUNT * BAR_COUNT * 2);
	float positions[BAR_COUNT]     = { master.position.y, bgm.position.y, se.position.y };
	float scale_factors[BAR_COUNT] = { VolumeFile::get_instance().get_master_volume(),
		VolumeFile::get_instance().get_bgm_volume(), VolumeFile::get_instance().get_se_volume() };
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		for (int o = 0; o < MAX_SCALE_COUNT * scale_factors[i]; ++o)
		{
			scales[i].emplace_back();
			scales[i].at(o).texsize = { static_cast<float>(sprite_scale->get_texture2d_desc().Width), static_cast<float>(sprite_scale->get_texture2d_desc().Height) };
			scales[i].at(o).pivot = scales[i].at(o).texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
			scales[i].at(o).scale = { 0.5f, 0.6f };
			scales[i].at(o).color = { 1,1,1,0.7f };
			scales[i].at(o).position = { 565.0f + 20.0f * o, positions[i] };
		}
		for (int o = 0; o < MAX_SCALE_COUNT; ++o)
		{
			shell_scales[i].emplace_back();
			shell_scales[i].at(o).texsize  = { static_cast<float>(sprite_scale->get_texture2d_desc().Width), static_cast<float>(sprite_scale->get_texture2d_desc().Height) };
			shell_scales[i].at(o).pivot    = shell_scales[i].at(o).texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
			shell_scales[i].at(o).scale    = { 0.5f, 0.6f };
			shell_scales[i].at(o).color    = { 1,1,1,0.25f };
			shell_scales[i].at(o).position = { 565.0f + 20.0f * o, positions[i] };
		}
	}
	//--volume_numbers--//
	{
		Number* number = new Number(device);
		number->set_offset_pos({ 1020.0f, master.position.y });
		number->set_offset_scale({ 0.3f, 0.3f });
		volume_numbers.insert(std::make_pair(BarType::MASTER, number));

		number = new Number(device);
		number->set_offset_pos({ 1020.0f, bgm.position.y });
		number->set_offset_scale({ 0.3f, 0.3f });
		volume_numbers.insert(std::make_pair(BarType::BGM, number));

		number = new Number(device);
		number->set_offset_pos({ 1020.0f, se.position.y });
		number->set_offset_scale({ 0.3f, 0.3f });
		volume_numbers.insert(std::make_pair(BarType::SE, number));
	}
}

VolumeIcon::~VolumeIcon()
{
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		scales[i].clear();
		shell_scales[i].clear();
	}
}

void VolumeIcon::update(GraphicsPipeline& graphics, float elapsed_time)
{
	interval_LX += elapsed_time;

	auto r_bar = [&](BarType type)
	{
		if ((game_pad->get_button() & GamePad::BTN_LEFT) && interval_LX > INTERVAL)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			interval_LX = 0;
			if (!scales[type].empty()) { scales[type].pop_back(); }
			save_volumes();
		}
		if ((game_pad->get_button() & GamePad::BTN_RIGHT) && interval_LX > INTERVAL)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			interval_LX = 0;
			float positions[BAR_COUNT] = { master.position.y, bgm.position.y, se.position.y };
			size_t index = scales[type].size();
			if (index < MAX_SCALE_COUNT)
			{
				scales[type].emplace_back();
				scales[type].at(index).texsize  = { static_cast<float>(sprite_scale->get_texture2d_desc().Width), static_cast<float>(sprite_scale->get_texture2d_desc().Height) };
				scales[type].at(index).pivot    = scales[type].at(index).texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
				scales[type].at(index).scale    = { 0.5f, 0.6f };
				scales[type].at(index).color    = { 1,1,1,0.7f };
				scales[type].at(index).position = { 565.0f + 20.0f * index, positions[type] };
			}
			save_volumes();
		}
	};

	switch (state)
	{
	case BarType::MASTER:
		if (game_pad->get_button_down() & GamePad::BTN_DOWN)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = BarType::BGM;
			selecterL_arrival_pos.y = selecterR_arrival_pos.y = bgm.position.y;
		}
		r_bar(BarType::MASTER);

		break;
	case BarType::BGM:
		if (game_pad->get_button_down() & GamePad::BTN_UP)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = BarType::MASTER;
			selecterL_arrival_pos.y = selecterR_arrival_pos.y = master.position.y;
		}
		if (game_pad->get_button_down() & GamePad::BTN_DOWN)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = BarType::SE;
			selecterL_arrival_pos.y = selecterR_arrival_pos.y = se.position.y;
		}
		r_bar(BarType::BGM);

		break;
	case BarType::SE:
		if (game_pad->get_button_down() & GamePad::BTN_UP)
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = BarType::BGM;
			selecterL_arrival_pos.y = bgm.position.y;
			selecterR_arrival_pos.y = bgm.position.y;
		}
		r_bar(BarType::SE);

		break;
	}

	selecterL.position = Math::lerp(selecterL.position, selecterL_arrival_pos, 10.0f * elapsed_time);
	selecterR.position = Math::lerp(selecterR.position, selecterR_arrival_pos, 10.0f * elapsed_time);

	for (int i = 0; i < BAR_COUNT; ++i)
	{
		if (volume_numbers.count(BarType(i)))
		{
			volume_numbers.at(BarType(i))->set_value(((float)scales[i].size() / (float)MAX_SCALE_COUNT) * 100);
			volume_numbers.at(BarType(i))->update(graphics, elapsed_time);
		}
	}
}

void VolumeIcon::render(std::string gui, ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos)
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
	r_font_render("master", master);
	r_font_render("bgm", bgm);
	r_font_render("se", se);
	fonts->yu_gothic->End(dc);

	sprite_scale->begin(dc);
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		for (int o = 0; o < shell_scales[i].size(); ++o)
		{
			std::string s = "shell_scale" + std::to_string(i) + " " + std::to_string(o);
			r_sprite_render(s, shell_scales[i].at(o), sprite_scale.get());
		}
		for (int o = 0; o < scales[i].size(); ++o)
		{
			std::string s = "scale" + std::to_string(i) + " " + std::to_string(o);
			r_sprite_render(s, scales[i].at(o), sprite_scale.get());
		}
	}
	sprite_scale->end(dc);

#ifdef USE_IMGUI
	ImGui::Begin("option");
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		static DirectX::XMFLOAT2 pos[BAR_COUNT]{};
		static DirectX::XMFLOAT2 scale[BAR_COUNT]{};
		std::string s1 = "number" + std::to_string(i);
		if (ImGui::TreeNode(s1.c_str()))
		{
			ImGui::DragFloat2("pos", &pos[i].x);
			ImGui::DragFloat2("scale", &scale[i].x, 0.1f);
			ImGui::TreePop();
		}
		if (volume_numbers.count(BarType(i)))
		{
			//volume_numbers.at(BarType(i))->set_offset_pos(pos[i]);
			//volume_numbers.at(BarType(i))->set_offset_scale(scale[i]);
		}
	}
	ImGui::End();
#endif // USE_IMGUI

	for (int i = 0; i < BAR_COUNT; ++i)
	{
		if (volume_numbers.count(BarType(i))) { volume_numbers.at(BarType(i))->render(dc, add_pos); }
	}
}

void VolumeIcon::vs_cursor(const DirectX::XMFLOAT2& cursor_pos)
{
	//--bar--//
	for (int o = 0; o < BAR_COUNT; ++o)
	{
		DirectX::XMFLOAT2 bar_radius = { (shell_scales[o].at(shell_scales[o].size() - 1).position.x - shell_scales[o].begin()->position.x) / 2,
			shell_scales[o].begin()->texsize.y * shell_scales[o].begin()->scale.y };
		DirectX::XMFLOAT2 bar_position = { shell_scales[o].begin()->position.x + bar_radius.x, shell_scales[o].begin()->position.y };
		if (Collision::hit_check_rect(cursor_pos, { 10,10 }, bar_position, bar_radius + DirectX::XMFLOAT2(25.0f, 0)))
		{
			float selecter_arrival_pos_y[BAR_COUNT] = { master.position.y, bgm.position.y, se.position.y };
			if (game_pad->get_button_down() & GamePad::BTN_B)
			{
				audio_manager->play_se(SE_INDEX::SELECT);

				state = BarType(o);
				selecterL_arrival_pos.y = selecterR_arrival_pos.y = selecter_arrival_pos_y[o];

				int index = 0;
				float distance = 100;
				float positions[BAR_COUNT] = { master.position.y, bgm.position.y, se.position.y };
				for (int i = 0; i < shell_scales[o].size(); ++i)
				{
					if (distance > fabsf(shell_scales[o].at(i).position.x - cursor_pos.x + FLT_EPSILON))
					{
						distance = fabsf(shell_scales[o].at(i).position.x - cursor_pos.x + FLT_EPSILON);
						index = i;
					}
				}
				scales[o].clear();
				for (int i = 0; i <= index; ++i)
				{
					scales[o].emplace_back();
					scales[o].at(i).texsize  = { static_cast<float>(sprite_scale->get_texture2d_desc().Width), static_cast<float>(sprite_scale->get_texture2d_desc().Height) };
					scales[o].at(i).pivot    = scales[o].at(i).texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
					scales[o].at(i).scale    = { 0.5f, 0.6f };
					scales[o].at(i).color    = { 1,1,1,0.7f };
					scales[o].at(i).position = { 565.0f + 20.0f * i, positions[o] };
				}

				save_volumes();
			}
		}
	}
}

void VolumeIcon::save_volumes()
{
	VolumeFile::get_instance().set_master_volume((float)scales[BarType::MASTER].size() / (float)MAX_SCALE_COUNT);
	VolumeFile::get_instance().set_bgm_volume((float)scales[BarType::BGM].size() / (float)MAX_SCALE_COUNT);
	VolumeFile::get_instance().set_se_volume((float)scales[BarType::SE].size() / (float)MAX_SCALE_COUNT);

	VolumeFile::get_instance().save();
}