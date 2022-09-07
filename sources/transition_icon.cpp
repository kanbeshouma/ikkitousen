#include "transition_icon.h"
#include "scene_option.h"
#include "scene_manager.h"
#include "scene_title.h"
#include "scene_loading.h"
#include "Operators.h"

TransitionIcon::TransitionIcon(ID3D11Device* device)
    : IconBase(device)
{
    //--game--//
	game.position = { 773.0f, 310.0f };
	game.scale = { 1.5f, 1.5f };
    game.s = L"ゲームに戻る";
    //--title--//
    title.position = { 793.0f, 455.0f };
	title.scale = { 1.5f, 1.5f };
    title.s = L"タイトルに戻る";

	//--selecterL--//
	selecterL.position = { 375.0f, game.position.y };
	//--selecterR--//
	selecterR.position = { 930.0f, game.position.y };

	selecterL_arrival_pos = { 375.0f, selecterL.position.y };
	selecterR_arrival_pos = { 930.0f, selecterR.position.y };
}

void TransitionIcon::update(GraphicsPipeline& graphics, float elapsed_time)
{
	switch (state)
	{
	case 0: // game
		if ((game_pad->get_button_down() & GamePad::BTN_DOWN) || (game_pad->get_axis_LY() < -0.5f))
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = 1;
			selecterL_arrival_pos = { 375.0f, title.position.y };
			selecterR_arrival_pos = { 930.0f, title.position.y };
		}
		if (game_pad->get_button_down() & GamePad::BTN_B)
		{
			audio_manager->play_se(SE_INDEX::DECISION);
			Option::set_switching(true);
			return;
		}
		break;
	case 1: // title
		if ((game_pad->get_button_down() & GamePad::BTN_UP) || (game_pad->get_axis_LY() > 0.5f))
		{
			audio_manager->play_se(SE_INDEX::SELECT);
			state = 0;
			selecterL_arrival_pos = { 375.0f, game.position.y };
			selecterR_arrival_pos = { 930.0f, game.position.y };
		}
		if (game_pad->get_button_down() & GamePad::BTN_B)
		{
			audio_manager->play_se(SE_INDEX::DECISION);
			Option::set_validity(false);
			SceneManager::scene_switching(new SceneLoading(new SceneTitle()));
			return;
		}
		break;
	}

	selecterL.position = Math::lerp(selecterL.position, selecterL_arrival_pos, 10.0f * elapsed_time);
	selecterR.position = Math::lerp(selecterR.position, selecterR_arrival_pos, 10.0f * elapsed_time);
}

void TransitionIcon::render(std::string gui, ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos)
{
    IconBase::render(gui, dc, add_pos);
#ifdef USE_IMGUI
    ImGui::Begin("option");
    if (ImGui::TreeNode("game string"))
    {
        ImGui::DragFloat2("pos", &game.position.x);
        ImGui::DragFloat2("scale", &game.scale.x, 0.1f);
        ImGui::ColorEdit4("color", &game.color.x);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("title"))
    {
        ImGui::DragFloat2("pos", &title.position.x);
        ImGui::DragFloat2("scale", &title.scale.x, 0.1f);
        ImGui::ColorEdit4("color", &title.color.x);
        ImGui::TreePop();
    }
    ImGui::End();
#endif // USE_IMGUI
    fonts->yu_gothic->Begin(dc);
    fonts->yu_gothic->Draw(game.s, game.position + add_pos, game.scale, game.color, game.angle, TEXT_ALIGN::MIDDLE, game.length);
    fonts->yu_gothic->Draw(title.s, title.position + add_pos, title.scale, title.color, title.angle, TEXT_ALIGN::MIDDLE, title.length);
    fonts->yu_gothic->End(dc);
}