#include "camera_shake.h"
#include "imgui_include.h"
#include "framework.h"

CameraShake::CameraShake() {}

CameraShake::~CameraShake() {}

void CameraShake::shake(GraphicsPipeline& graphics, float elapsed_time)
{
#ifdef USE_IMGUI
	ImGui::Begin("camera_shake");
	ImGui::SliderFloat("max_sway_x [pixel]", &max_sway_x, 0.0f, 64.0f);
	ImGui::SliderFloat("max_sway_y [pixel]", &max_sway_y, 0.0f, 64.0f);
	ImGui::SliderFloat("max_skew [degree]", &max_skew, 0.0f, 10.0f);
	ImGui::SliderFloat("seed_shifting_factor", &seed_shifting_factor, 0.0f, 10.0f);
	ImGui::End();
#endif
	using namespace DirectX;
	// camerashake
	seed += elapsed_time;
	const float shake = 2.0f * static_cast<float>(pn.noise(seed * seed_shifting_factor, seed * seed_shifting_factor, 0)) - 1.0f;
	XMStoreFloat4x4(&shake_matrix,
		XMMatrixTranslation(shake * max_sway_x / SCREEN_WIDTH, shake * max_sway_y / SCREEN_HEIGHT, 0) *
		XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(shake * max_skew)));
}

void CameraShake::shake(GraphicsPipeline& graphics, float elapsed_time,
	float max_skew, float max_sway_x, float max_sway_y, float seed_shifting_factor)
{
	this->max_skew = max_skew;
	this->max_sway_x = max_sway_x;
	this->max_sway_y = max_sway_y;
	this->seed_shifting_factor = seed_shifting_factor;
	shake(graphics, elapsed_time);
}

void CameraShake::reset(GraphicsPipeline& graphics)
{
	// íPà çsóÒÇëóÇÈ
	shake_matrix = { 1, 0, 0, 0,
					 0, 1, 0, 0,
					 0, 0, 1, 0,
					 0, 0, 0, 1 };
}