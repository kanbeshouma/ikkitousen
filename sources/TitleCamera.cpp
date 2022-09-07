#include "camera.h"

void TitleCamera::Initialize(GraphicsPipeline& graphics)
{
	HRESULT hr{ S_OK };
	//----定数バッファ----//
	// カメラ関連
	scene_constants = std::make_unique<Constants<SceneConstants>>(graphics.get_device().Get());
	// orientationの初期化
	{
		DirectX::XMFLOAT3 n(0, 1, 0); // 軸（正規化）
		constexpr float angle = DirectX::XMConvertToRadians(0); //角度（ラジアン）
		orientation = {
			sinf(angle / 2) * n.x,
			sinf(angle / 2) * n.y,
			sinf(angle / 2) * n.z,
			cosf(angle / 2)
		};
	}
	//using namespace DirectX;
	//const DirectX::XMFLOAT3 playerPosition = player->GetPosition();
	//const DirectX::XMFLOAT3 playerForward = player->GetForward();
	//const DirectX::XMFLOAT3 playerUp = player->GetUp();

	//const DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);
	//const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&playerForward);
	//const DirectX::XMVECTOR PlayerUp = DirectX::XMLoadFloat3(&playerUp);

	//const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * 2;
	//DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);

	//DirectX::XMVECTOR EyeVector = -PlayerForward * 10 + PlayerUp * 2;
	//radius = 20;
	//EyeVector = DirectX::XMVector3Normalize(EyeVector);
	//DirectX::XMStoreFloat3(&eyeVector, EyeVector);

	//const DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * 3;
	//DirectX::XMStoreFloat3(&target, Target);

	//const DirectX::XMVECTOR Eye = Target + EyeVector * radius;
	//DirectX::XMStoreFloat3(&eye, Eye);

	target = { 0.0f,6.5f,0.0f };
	eye = { 0.0f,4.0f,-20.0f };

	scene_constants->data.light_direction = { 0, -1, 1, 1 };
	scene_constants->data.light_direction.w = 0.6f;
}

void TitleCamera::Update(float elapsedTime)
{

	AttitudeControl(elapsedTime);
	DebugGUI();
	UpdateViewProjection();
}
