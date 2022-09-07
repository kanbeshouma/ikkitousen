#include "icon_base.h"
#include "imgui_include.h"
#include "Operators.h"

IconBase::IconBase(ID3D11Device* device)
{
	//--selecter--//
	sprite_selecter = std::make_unique<SpriteBatch>(device, L".\\resources\\Sprites\\option\\selecter.png", 2 + 3 * 2);
	selecterL.texsize = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
	selecterL.pivot = selecterL.texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
	selecterL.scale = { 0.7f, 0.3f };
	selecterL.color = { 1,1,1,1 };

	selecterR.texsize = { static_cast<float>(sprite_selecter->get_texture2d_desc().Width), static_cast<float>(sprite_selecter->get_texture2d_desc().Height) };
	selecterR.pivot = selecterR.texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
	selecterR.scale = { 0.7f, 0.3f };
	selecterR.color = { 1,1,1,1 };
}

void IconBase::render(std::string gui, ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos)
{
	//--sprite_selecter--//
#ifdef USE_IMGUI
	ImGui::Begin("option");
	std::string s2 = gui + " selecterL";
	if (ImGui::TreeNode(s2.c_str()))
	{
		ImGui::DragFloat2("pos", &selecterL.position.x);
		ImGui::DragFloat2("scale", &selecterL.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &selecterL.color.x);
		ImGui::TreePop();
	}
	std::string s3 = gui + " selecterR";
	if (ImGui::TreeNode(s3.c_str()))
	{
		ImGui::DragFloat2("pos", &selecterR.position.x);
		ImGui::DragFloat2("scale", &selecterR.scale.x, 0.01f);
		ImGui::ColorEdit4("color", &selecterR.color.x);
		ImGui::TreePop();
	}
	ImGui::End();
#endif // USE_IMGUI
	sprite_selecter->begin(dc);
	sprite_selecter->render(dc, selecterL.position + add_pos, selecterL.scale,
		selecterL.pivot, selecterL.color, selecterL.angle, selecterL.texpos, selecterL.texsize);
	sprite_selecter->render(dc, selecterR.position + add_pos, selecterR.scale,
		selecterR.pivot, selecterR.color, selecterR.angle, selecterR.texpos, selecterR.texsize);
	sprite_selecter->end(dc);
}