#include "camera.h"


void ClearCamera::Initialize(GraphicsPipeline& graphics)
{
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

    eye = keep_eye;
    target = keep_target;
	using namespace DirectX;
	const DirectX::XMFLOAT3 playerPosition = player->GetPosition();
	const DirectX::XMFLOAT3 playerForward = player->GetForward();
	const DirectX::XMFLOAT3 playerUp = player->GetUp();

	const DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);
	const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&playerForward);
	const DirectX::XMVECTOR PlayerUp = DirectX::XMLoadFloat3(&playerUp);

	const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * 6;
	DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);

	DirectX::XMVECTOR EyeVector = -PlayerForward * 10 + PlayerUp * 1;
	radius = 25;
	EyeVector = DirectX::XMVector3Normalize(EyeVector);
	DirectX::XMStoreFloat3(&eyeVector, EyeVector);

	const DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * 12;
	DirectX::XMStoreFloat3(&target, Target);

	const DirectX::XMVECTOR Eye = Target + EyeVector * radius;
	DirectX::XMStoreFloat3(&eye, Eye);


	//const DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);

	//const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * 6;
	//DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);

	//DirectX::XMVECTOR EyeVector = EyeCenter - CameraPosition;
	//EyeVector = DirectX::XMVector3Normalize(EyeVector);
	//DirectX::XMStoreFloat3(&eyeVector, EyeVector);

    UpdateViewProjection();
}

void ClearCamera::Update(float elapsedTime)
{
	if(cameraReset)
	{
		using namespace DirectX;

		const DirectX::XMFLOAT3 playerPosition = player->GetPosition();
		const DirectX::XMFLOAT3 playerForward = player->GetForward();
		const DirectX::XMFLOAT3 playerUp = player->GetUp();

		const DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);
		const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&playerForward);
		const DirectX::XMVECTOR PlayerUp = DirectX::XMLoadFloat3(&playerUp);

		const DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);

		const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * 6;
		DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);

		DirectX::XMVECTOR EyeVector =  CameraPosition - EyeCenter;
		EyeVector = DirectX::XMVector3Normalize(EyeVector);
		DirectX::XMStoreFloat3(&eyeVector, EyeVector);

		if(CameraReset(elapsedTime, PlayerForward, PlayerUp))
		{
			cameraReset = false;
		}
	}
	UpdateEye();
	AttitudeControl(elapsedTime);
	DebugGUI();
	UpdateViewProjection();
}

bool ClearCamera::CameraReset(float elapsedTime, DirectX::XMVECTOR PlayerForward, DirectX::XMVECTOR PlayerUp)
{
	static float rotateTimer{ 0 };
	using namespace DirectX;

	if (rotateTimer < 0.1f)
	{
		rotateTimer += elapsedTime;
		DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
		DirectX::XMVECTOR RotateEyeVector = -PlayerForward * 10 + PlayerUp;
		RotateEyeVector = DirectX::XMVector3Normalize(RotateEyeVector);

		const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EyeVector, RotateEyeVector);
		float dot{};
		DirectX::XMStoreFloat(&dot, Dot);
		const float angle = acosf(dot);

		const DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(EyeVector, RotateEyeVector);

		if (dot < 0.98f)
		{
			const DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, angle * elapsedTime / 0.1f);
			EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
		}
		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
	}
	else
	{
		DirectX::XMVECTOR EyeVector = -PlayerForward * 10 + PlayerUp;
		EyeVector = DirectX::XMVector3Normalize(EyeVector);
		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
		rotateTimer = 0;
		return true;
	}
	return false;
}

void ClearCamera::UpdateEye()
{
	using namespace DirectX;

	radius = 25;
	const DirectX::XMVECTOR EyeCenter = DirectX::XMLoadFloat3(&eyeCenter);
	const DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
	const DirectX::XMVECTOR Eye = EyeCenter + EyeVector * radius;
	DirectX::XMStoreFloat3(&eye, Eye);
}
