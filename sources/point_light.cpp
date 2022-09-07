#include "point_light.h"
#include "imgui_include.h"
#include "user.h"

PointLights::PointLights(GraphicsPipeline& graphics)
{
	//定数バッファ
	plig_constants = std::make_unique<Constants<PointLightConstants>>(graphics.get_device().Get());
	// モデルの生成
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        light_sources[i] = std::make_unique<SkinnedMesh>(graphics.get_device().Get(), ".\\resources\\Models\\point_light\\point_light.fbx");
    }
	// パラメータ設定
	set_all_parameters(0, { -4.5f,3.3f,0.8f,0 }, { 1,0,0,1 }, 5.0f, 1.0f);
	set_all_parameters(1, { -4.5f,3.3f,5.5f,0 }, { 1,0,0,1 }, 5.0f, 1.0f);
	set_all_parameters(2, { 4.2f,3.3f,0.8f,0 }, { 1,0,0,1 }, 5.0f, 1.0f);
	set_all_parameters(3, { 4.2f,3.3f,5.5f,0 }, { 1,0,0,1 }, 5.0f, 1.0f);
	set_all_parameters(4, { 22.245f,22.129f,37.306f,0 },  { 1,1,1,1 }, 5.3f, 3.6f);
	set_all_parameters(5, { FLT_MAX,FLT_MAX,FLT_MAX,0 }, { 0, 0,0,1 }, 5.0f, 1.0f);
	set_all_parameters(6, { FLT_MAX,FLT_MAX,FLT_MAX,0 }, { 0, 0,0,1 }, 5.0f, 1.0f);
	set_all_parameters(7, { FLT_MAX,FLT_MAX,FLT_MAX,0 }, { 0, 0,0,1 }, 5.0f, 1.0f);
}

PointLights::~PointLights() {}

void PointLights::render(GraphicsPipeline& graphics, float elapsed_time)
{
	// モデルの描画
	static DirectX::XMFLOAT3  angle = { 0,0,0 };
	static DirectX::XMFLOAT3  scale = { 0.001f,0.0f,0.006f };

#ifdef USE_IMGUI
	ImGui::Begin("point light");
	if (ImGui::TreeNode("transform"))
	{
		ImGui::DragFloat3("angle", &angle.x, 0.001f);
		ImGui::DragFloat3("scale", &scale.x, 0.001f, -10, 10);
		ImGui::TreePop();
	}
	ImGui::End();
#endif
	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
#ifdef USE_IMGUI
		ImGui::Begin("point light");
		std::string s_transform = "light" + std::to_string(i);
		if (ImGui::TreeNode(s_transform.c_str()))
		{
			std::string s = "pos" + std::to_string(i);
			ImGui::DragFloat3(s.c_str(), &plig_constants->data.point_lights[i].position.x, 0.001f);
			std::string s2 = "range" + std::to_string(i);
			ImGui::DragFloat(s2.c_str(), &plig_constants->data.point_lights[i].range, 0.1f);
			std::string s3 = "luminous_intensity" + std::to_string(i);
			ImGui::DragFloat(s3.c_str(), &plig_constants->data.point_lights[i].luminous_intensity, 0.01f);
			std::string s4 = "color" + std::to_string(i);
			ImGui::ColorEdit4(s4.c_str(), &plig_constants->data.point_lights[i].color.x);
			ImGui::TreePop();
		}
		ImGui::End();
#endif
		plig_constants->data.unique_id = i;
		// 定数バッファ
		plig_constants->bind(graphics.get_dc().Get(), 3);
		// 描画ステート設定
		graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::POINT_LIGHT);
		// スタティックメッシュ
		DirectX::XMFLOAT3 position = { plig_constants->data.point_lights[i].position.x, plig_constants->data.point_lights[i].position.y, plig_constants->data.point_lights[i].position.z };
		light_sources[i]->render(graphics.get_dc().Get(), Math::calc_world_matrix(scale, angle, position), plig_constants->data.point_lights[i].color);
	}
}

void PointLights::finalize(GraphicsPipeline& graphics)
{
	// 次のシーンへいった時の終了処理
	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		plig_constants->data.point_lights[i].luminous_intensity = 0;
		plig_constants->data.point_lights[i].range = 0;
	}
	plig_constants->bind(graphics.get_dc().Get(), 3);
}
