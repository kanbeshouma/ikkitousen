#include "BaseCamera.h"
#include "framework.h"

DirectX::XMFLOAT4X4 BaseCamera::keep_view = {};
DirectX::XMFLOAT4X4 BaseCamera::keep_projection = {};
DirectX::XMFLOAT3 BaseCamera::keep_eye = {};
DirectX::XMFLOAT3 BaseCamera::keep_target = {};
float BaseCamera::keep_range = {};

void BaseCamera::UpdateViewProjection()
{
	DirectX::XMFLOAT3 up = { 0, 1, 0 };
	DirectX::XMVECTOR eye_vec = XMLoadFloat3(&eye);
	DirectX::XMVECTOR focus_vec = XMLoadFloat3(&target);
	keep_eye = eye;
	keep_target = target;
	keep_range = range;

	DirectX::XMMATRIX view_mat = DirectX::XMMatrixLookAtLH(eye_vec, focus_vec, DirectX::XMLoadFloat3(&up)); // V
	DirectX::XMStoreFloat4x4(&view, view_mat);
	//ビュー行列を逆行列かしワールド行列に戻す
	DirectX::XMMATRIX world_vec = DirectX::XMMatrixInverse(nullptr, view_mat);
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, world_vec);
	this->right.x = world._11;
	this->right.y = world._12;
	this->right.z = world._13;

	this->forward.x = world._31;
	this->forward.y = world._32;
	this->forward.z = world._33;

	float width = static_cast<float>(SCREEN_WIDTH);
	float height = static_cast<float>(SCREEN_HEIGHT);
	float aspect_ratio{ width / height };

	DirectX::XMMATRIX projection_mat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45), aspect_ratio, near_far.x, near_far.y); // P
	DirectX::XMStoreFloat4x4(&projection, projection_mat);
}

void BaseCamera::AttitudeControl(float elapsedTime)
{
	using namespace DirectX;
	DirectX::XMVECTOR Orientation = DirectX::XMLoadFloat4(&orientation);
	DirectX::XMVECTOR Forward, Right, Up;
	DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(Orientation);
	DirectX::XMFLOAT4X4 m4x4 = {};
	DirectX::XMStoreFloat4x4(&m4x4, m);
	Right = { m4x4._11, m4x4._12, m4x4._13 };
	Up = { 0,1,0 };
	Forward = { m4x4._31, m4x4._32, m4x4._33 };

	DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
	EyeVector *= -1;

	DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(EyeVector), DirectX::XMVector3Normalize(Forward));
	float dot{};
	DirectX::XMStoreFloat(&dot, Dot);
	const float angle = acosf(dot);

	if (dot > 0.98f)
	{
		//DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(EyeVector, Forward);
		DirectX::XMVECTOR Axis = Up;
		DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, angle);
		Orientation = DirectX::XMQuaternionMultiply(Orientation, Quaternion);
	}


	DirectX::XMStoreFloat4(&orientation, Orientation);

}

void BaseCamera::DebugGUI()
{
#ifdef USE_IMGUI
	ImGui::Begin("Camera", false);
	ImGui::SliderFloat3("Position", &eye.x, -20.0f, 10.0f);
	ImGui::InputFloat3("target", &target.x);
	ImGui::InputFloat2("near_far", &near_far.x);

	ImGui::InputFloat3("forward", &forward.x);

	ImGui::Checkbox("PlayerAttackStart", &attack);
	ImGui::Checkbox("PlayerAttacking", &attacking);

	ImGui::SliderFloat("range", &radius, 3.0f, 30.0f);
	ImGui::SliderFloat("up", &up, 3.0f, 30.0f);
	ImGui::SliderFloat("up1", &up1, 1.0f, 30.0f);

	ImGui::InputFloat("late", &lerpLate);
	ImGui::End();
#endif
}

void BaseCamera::calc_view_projection(GraphicsPipeline& graphics)
{
	// ビュー・プロジェクション変換行列を計算し、それを定数バッファにセットする
	using namespace DirectX;
#ifdef USE_IMGUI
	if (display_camera_imgui)
	{
		ImGui::Begin("Camera", false);
		ImGui::DragFloat4("LightDirection", &scene_constants->data.light_direction.x, 0.01f, -1, 1);
		ImGui::ColorEdit4("light_color", &scene_constants->data.light_color.x);
		ImGui::End();
		debug_gui();
	}
#endif
	// ビュー行列/プロジェクション行列を作成
	XMMATRIX V = XMLoadFloat4x4(&view);
	XMMATRIX P = XMLoadFloat4x4(&projection);
	keep_view = view;
	keep_projection = projection;
	// 定数バッファにフェッチする
	XMStoreFloat4x4(&scene_constants->data.view_projection, V * P);
	scene_constants->data.camera_position = { eye.x,eye.y,eye.z,0 };
	scene_constants->data.shake_matrix = camera_shake->get_shake_matrix();
	XMStoreFloat4x4(&scene_constants->data.inverse_view_projection, XMMatrixInverse(nullptr, V * P));

	scene_constants->bind(graphics.get_dc().Get(), 1, CB_FLAG::PS_VS_GS);
}

void BaseCamera::debug_gui()
{
#ifdef USE_IMGUI
	if (display_camera_imgui)
	{
		ImGui::Begin("Camera");
		ImGui::Checkbox("mouse_operation", &is_mouse_operation);
		ImGui::DragFloat("range", &range, 0.2f);
		ImGui::DragFloat2("angle", &angle.x, 0.1f);
		ImGui::DragFloat3("target", &target.x, 0.1f, -100, 100);
		ImGui::End();
	}
#endif
}
