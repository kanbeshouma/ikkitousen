#include "framework.h"
#include "camera.h"
#include "user.h"


#include "CameraManager.h"

//void GameCamera::Initialize(GraphicsPipeline& graphics)
//{
//	gameInitialize(graphics);
//}
//
//void GameCamera::Update(float elapsedTime)
//{
//	gameUpdate(elapsedTime);
//	AttitudeControl(elapsedTime);
//	DebugGUI();
//	UpdateViewProjection();
//}
//
//void GameCamera::gameInitialize(GraphicsPipeline& graphics)
//{
//	HRESULT hr{ S_OK };
//	//----定数バッファ----//
//	// カメラ関連
//	scene_constants = std::make_unique<Constants<SceneConstants>>(graphics.get_device().Get());
//	// orientationの初期化
//	{
//		DirectX::XMFLOAT3 n(0, 1, 0); // 軸（正規化）
//		constexpr float angle = DirectX::XMConvertToRadians(0); //角度（ラジアン）
//		orientation = {
//			sinf(angle / 2) * n.x,
//			sinf(angle / 2) * n.y,
//			sinf(angle / 2) * n.z,
//			cosf(angle / 2)
//		};
//	}
//	using namespace DirectX;
//	const DirectX::XMFLOAT3 playerPosition = player->GetPosition();
//	const DirectX::XMFLOAT3 playerForward = player->GetForward();
//	const DirectX::XMFLOAT3 playerUp = player->GetUp();
//
//	const DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);
//	const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&playerForward);
//	const DirectX::XMVECTOR PlayerUp = DirectX::XMLoadFloat3(&playerUp);
//
//	//DirectX::XMVECTOR EyeVector = -PlayerForward * 10 + PlayerUp;
//	////const DirectX::XMVECTOR Radius = DirectX::XMVector3Length(EyeVector);
//	////DirectX::XMStoreFloat(&radius, Radius);
//	//radius = 15;
//	//EyeVector = DirectX::XMVector3Normalize(EyeVector);
//	//DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//
//	//const DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * 3;
//	//DirectX::XMStoreFloat3(&target, Target);
//
//	//const DirectX::XMVECTOR Eye = Target + EyeVector * radius;
//	//DirectX::XMStoreFloat3(&eye, Eye);
//
//	const DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * 6;
//	DirectX::XMStoreFloat3(&target, Target);
//
//	DirectX::XMVECTOR EyeVector = -PlayerForward * 10 + PlayerUp;
//	EyeVector = DirectX::XMVector3Normalize(EyeVector);
//	DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//
//	const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * 6;
//	DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);
//
//	radius = 15;
//	const DirectX::XMVECTOR Eye = EyeCenter + EyeVector * radius;
//	DirectX::XMStoreFloat3(&eye, Eye);
//
//}
//
//void GameCamera::gameUpdate(float elapsedTime)
//{
//	using namespace DirectX;
//
//	const DirectX::XMFLOAT3 playerForward = player->GetForward();
//	const DirectX::XMFLOAT3 playerUp = player->GetUp();
//	const DirectX::XMFLOAT3 playerPosition = player->GetPosition();
//
//	const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&playerForward);
//	const DirectX::XMVECTOR PlayerUp = DirectX::XMLoadFloat3(&playerUp);
//	const DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);
//
//	const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * 6;
//	DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);
//
//	if (player->GetEnemyLockOn())
//	{
//		if (player->GetCameraReset())
//		{
//			const DirectX::XMFLOAT3 playerTarget = player->GetTarget();
//			const DirectX::XMVECTOR PlayerTarget = DirectX::XMLoadFloat3(&playerTarget);
//			RockOnCalculateEyeVector(PlayerPosition, PlayerTarget,PlayerUp);
//			if (RockOnCameraReset(elapsedTime, PlayerForward, PlayerUp))
//			{
//				player->FalseCameraReset();
//			}
//		}
//		SetAngle(elapsedTime);
//		const DirectX::XMFLOAT3 playerTarget = player->GetTarget();
//		const DirectX::XMVECTOR PlayerTarget = DirectX::XMLoadFloat3(&playerTarget);
//		RockOnCalculateEyeVector(PlayerPosition, PlayerTarget,PlayerUp);
//		if(RockOnUpdateEyeVector(elapsedTime, PlayerUp, player->GetCameraLockOn()))
//		{
//		    player->FalseCameraLockOn();
//		}
//		//UpdateEyeVector(elapsedTime, PlayerUp);
//		UpdateRockOnTarget(PlayerTarget);
//	}
//	else
//	{
//		if (player->GetCameraReset())
//		{
//			if (CameraReset(elapsedTime, PlayerForward, PlayerUp))
//			{
//				player->FalseCameraReset();
//			}
//		}
//		SetAngle(elapsedTime);
//		UpdateEyeVector(elapsedTime, PlayerUp);
//   	    UpdateTarget(PlayerPosition, PlayerUp);
//	}
//	UpdateEye();
//
//}
//
//void GameCamera::SetAngle(float elapsedTime)
//{
//	float ax = game_pad->get_axis_RX();
//	float ay = game_pad->get_axis_RY();
//
//	if (ax > 0.1f || ax < 0.1f)
//	{
//		horizonDegree = 180 * ax * elapsedTime;
//	}
//	if (ay > 0.1f || ay < 0.1f)
//	{
//		verticalDegree = 180 * -ay * elapsedTime;
//	}
//}
//
//void GameCamera::UpdateEyeVector(float elapsedTime, DirectX::XMVECTOR PlayerUp)
//{
//	//変化が無ければスルー
//	if (horizonDegree < 0.1f && horizonDegree >-0.1f
//		&& verticalDegree < 0.1f && verticalDegree > -0.1f)return;
//
//	DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
//	DirectX::XMFLOAT3 up = { 0,1,0 };
//	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
//
//	DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Up, DirectX::XMVectorScale(EyeVector, -1));
//
//	//プレイヤーに対して縦方向の回転
//	//プレイヤーの真上と真下に近いときは回転しない
//	if (verticalDegree > 0.1f || verticalDegree < -0.1f)
//	{
//		DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(Up), DirectX::XMVector3Normalize(EyeVector));
//		float dot{};
//		DirectX::XMStoreFloat(&dot, Dot);
//		//真上に近い時
//		if (dot > 0.9f)
//		{
//			if (verticalDegree < 0.1f)
//			{
//				DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
//				DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
//				EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//			}
//			verticalDegree = 0;
//		}
//		//真下に近い時
//		else if (dot < -0.9f)
//		{
//			if (verticalDegree > 0.1f)
//			{
//				DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
//				DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
//				EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//			}
//			verticalDegree = 0;
//		}
//		//通常時
//		else
//		{
//			DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
//			DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
//			EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//			verticalDegree = 0;
//		}
//	}
//	//プレイヤーに対して横の回転
//	if (horizonDegree > 0.1f || horizonDegree < -0.1f)
//	{
//		DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Up);
//		DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(horizonDegree));
//		EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//		horizonDegree = 0;
//	}
//
//
//	//最後に一回だけ行う
//	DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//}
//
//void GameCamera::AttitudeControl(float elapsedTime)
//{
//	using namespace DirectX;
//	DirectX::XMVECTOR Orientation = DirectX::XMLoadFloat4(&orientation);
//	DirectX::XMVECTOR Forward, Right, Up;
//	DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(Orientation);
//	DirectX::XMFLOAT4X4 m4x4 = {};
//	DirectX::XMStoreFloat4x4(&m4x4, m);
//	Right = { m4x4._11, m4x4._12, m4x4._13 };
//	Up = { 0,1,0 };
//	Forward = { m4x4._31, m4x4._32, m4x4._33 };
//
//	DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
//	EyeVector *= -1;
//
//	DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(EyeVector), DirectX::XMVector3Normalize(Forward));
//	float dot{};
//	DirectX::XMStoreFloat(&dot, Dot);
//	const float angle = acosf(dot);
//
//	if (dot > 0.98f)
//	{
//		//DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(EyeVector, Forward);
//		DirectX::XMVECTOR Axis = Up;
//		DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, angle);
//		Orientation = DirectX::XMQuaternionMultiply(Orientation, Quaternion);
//	}
//
//
//
//	DirectX::XMStoreFloat4(&orientation, Orientation);
//}
//
//void GameCamera::UpdateEye()
//{
//	using namespace DirectX;
//
//	const DirectX::XMVECTOR EyeCenter = DirectX::XMLoadFloat3(&eyeCenter);
//	const DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
//	const DirectX::XMVECTOR Eye = EyeCenter + EyeVector * radius;
//	DirectX::XMStoreFloat3(&eye, Eye);
//}
//
//void GameCamera::UpdateTarget(DirectX::XMVECTOR PlayerPosition, DirectX::XMVECTOR PlayerUp)
//{
//	using namespace DirectX;
//
//	DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * 6;
//	DirectX::XMStoreFloat3(&target, Target);
//
//	//DirectX::XMVECTOR CameraForward = DirectX::XMLoadFloat3(&forward);
//	//Target = CameraForward;
//
//}
//
//void GameCamera::UpdateRockOnTarget(DirectX::XMVECTOR PlayerTarget)
//{
//	DirectX::XMVECTOR Target = PlayerTarget;
//	DirectX::XMStoreFloat3(&target, Target);
//}
//
//bool GameCamera::CameraReset(float elapsedTime, DirectX::XMVECTOR PlayerForward, DirectX::XMVECTOR PlayerUp)
//{
//	static float rotateTimer{ 0 };
//	using namespace DirectX;
//
//	if (rotateTimer < 0.1f)
//	{
//		rotateTimer += elapsedTime;
//		DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
//		DirectX::XMVECTOR RotateEyeVector = -PlayerForward * 10 + PlayerUp;
//		RotateEyeVector = DirectX::XMVector3Normalize(RotateEyeVector);
//
//		const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EyeVector, RotateEyeVector);
//		float dot{};
//		DirectX::XMStoreFloat(&dot, Dot);
//		const float angle = acosf(dot);
//
//		const DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(EyeVector, RotateEyeVector);
//
//		if (dot < 0.98f)
//		{
//			const DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, angle * elapsedTime / 0.1f);
//			EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//		}
//		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//	}
//	else
//	{
//		DirectX::XMVECTOR EyeVector = -PlayerForward * 10 + PlayerUp;
//		EyeVector = DirectX::XMVector3Normalize(EyeVector);
//		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//		rotateTimer = 0;
//		return true;
//	}
//	return false;
//}
//
//bool GameCamera::RockOnCameraReset(float elapsedTime, DirectX::XMVECTOR PlayerForward, DirectX::XMVECTOR PlayerUp)
//{
//	static float rotateTimer{ 0 };
//	using namespace DirectX;
//
//	if (rotateTimer < 0.1f)
//	{
//		rotateTimer += elapsedTime;
//		DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
//		DirectX::XMVECTOR RotateEyeVector = DirectX::XMLoadFloat3(&rockOnEyeVector);
//		RotateEyeVector = DirectX::XMVector3Normalize(RotateEyeVector);
//
//		const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EyeVector, RotateEyeVector);
//		float dot{};
//		DirectX::XMStoreFloat(&dot, Dot);
//		const float angle = acosf(dot);
//
//		const DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(EyeVector, RotateEyeVector);
//		if (dot < 0.98f)
//		{
//			const DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, angle * elapsedTime / 0.1f);
//			EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//		}
//		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//
//	}
//	else
//	{
//		DirectX::XMVECTOR EyeVector = -PlayerForward * 10 + PlayerUp;
//		EyeVector = DirectX::XMVector3Normalize(EyeVector);
//		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//		rotateTimer = 0;
//		return true;
//	}
//	return false;
//
//}
//
//bool GameCamera::RockOnUpdateEyeVector(float elapsedTime, DirectX::XMVECTOR PlayerUp, bool rockOnStart)
//{
//	if (rockOnStart)
//	{
//		if (rockOnTimer < 0.1f)
//		{
//			rockOnTimer += elapsedTime;
//			DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
//			DirectX::XMVECTOR RotateEyeVector = DirectX::XMLoadFloat3(&rockOnEyeVector);
//			RotateEyeVector = DirectX::XMVector3Normalize(RotateEyeVector);
//
//			const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EyeVector, RotateEyeVector);
//			float dot{};
//			DirectX::XMStoreFloat(&dot, Dot);
//			const float angle = acosf(dot);
//
//			const DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(EyeVector, RotateEyeVector);
//			//if(DirectX::XMVector3Equal(DirectX::XMVectorZero(), Axis))
//			//{
//			//	DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//			//	return false;
//			//}
//			if (dot < 0.98f)
//			{
//				const DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, angle * elapsedTime / 0.1f);
//				EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//			}
//			DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//			return false;
//		}
//		else
//		{
//			eyeVector = rockOnEyeVector;
//			rockOnTimer = 0;
//			return true;
//		}
//	}
//	else
//	{
//		eyeVector = rockOnEyeVector;
//
//		//左右のずれ修正
//		DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
//
//		const DirectX::XMVECTOR PlayerToRockOn = DirectX::XMLoadFloat3(&playerToRockOn);
//		DirectX::XMVECTOR Cross = DirectX::XMVector3Cross(PlayerUp, PlayerToRockOn);
//		Cross = DirectX::XMVector3Normalize(Cross);
//		const DirectX::XMVECTOR ProjectionLength = DirectX::XMVector3Dot(Cross, EyeVector);
//		float projectionLength{};
//		DirectX::XMStoreFloat(&projectionLength, ProjectionLength);
//
//		EyeVector = DirectX::XMVectorSubtract(EyeVector, DirectX::XMVectorScale(Cross, projectionLength));
//		EyeVector = DirectX::XMVector3Normalize(EyeVector);
//
//#if 0
//		//変化が無ければスルー
//		if (horizonDegree < 0.1f && horizonDegree >-0.1f
//			&& verticalDegree < 0.1f && verticalDegree > -0.1f) {
//		}
//		else
//		{
//			DirectX::XMFLOAT3 up = { 0,1,0 };
//			DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
//
//			DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Up, DirectX::XMVectorScale(EyeVector, -1));
//
//			//プレイヤーに対して縦方向の回転
//			//プレイヤーの真上と真下に近いときは回転しない
//			if (verticalDegree > 0.1f || verticalDegree < -0.1f)
//			{
//				DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(Up), DirectX::XMVector3Normalize(EyeVector));
//				float dot{};
//				DirectX::XMStoreFloat(&dot, Dot);
//				//真上に近い時
//				if (dot > 0.9f)
//				{
//					if (verticalDegree < 0.1f)
//					{
//						DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
//						DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
//						EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//					}
//					verticalDegree = 0;
//				}
//				//真下に近い時
//				else if (dot < -0.9f)
//				{
//					if (verticalDegree > 0.1f)
//					{
//						DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
//						DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
//						EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//					}
//					verticalDegree = 0;
//				}
//				//通常時
//				else
//				{
//					DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
//					DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
//					EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
//					verticalDegree = 0;
//				}
//			}
//		}
//#endif
//
//		//最後に一回だけ行う
//		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
//		return false;
//	}
//}
//
//void GameCamera::RockOnCalculateEyeVector(DirectX::XMVECTOR PlayerPosition, DirectX::XMVECTOR RockOnPosition,DirectX::XMVECTOR PlayerUp)
//{
//	using namespace DirectX;
//
//	DirectX::XMFLOAT3 up{ 0,1,0 };
//	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
//
//#if 1
//	DirectX::XMVECTOR Target = DirectX::XMLoadFloat3(&target);
//	Target -= PlayerPosition;
//	DirectX::XMVECTOR PlayerToRockOn = RockOnPosition - PlayerPosition;
//	const DirectX::XMVECTOR Distance = DirectX::XMVector3Length(PlayerToRockOn);
//	float distance{};
//	DirectX::XMStoreFloat(&distance, Distance);
//
//	const DirectX::XMVECTOR CameraRockOnPosition = PlayerToRockOn * 1.0f;
//
//	PlayerToRockOn = DirectX::XMVector3Normalize(PlayerToRockOn);
//
//
//	//DirectX::XMVECTOR RockOnEyeVector = -PlayerToRockOn * 10 + PlayerUp;
//	DirectX::XMVECTOR RockOnEyeVector = RockOnPosition - DirectX::XMLoadFloat3(&eyeCenter);
//
//	RockOnEyeVector *= -1;
//	RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);
//
//#else
//	DirectX::XMVECTOR PlayerToRockOn = RockOnPosition - PlayerPosition;
//	PlayerToRockOn = DirectX::XMVector3Normalize(PlayerToRockOn);
//
//
//    //DirectX::XMVECTOR RockOnEyeVector = -PlayerToRockOn * 10 + Up;
//	DirectX::XMVECTOR RockOnEyeVector = RockOnPosition - DirectX::XMLoadFloat3(&eyeCenter);
//	RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);
//#endif
//
//    //真上、真下に近いとき修正
//	DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(RockOnEyeVector, DirectX::XMVector3Normalize(Up));
//	float dot{};
//	DirectX::XMStoreFloat(&dot, Dot);
//	if (dot > 0.8f)
//	{
//		dot = 0.8f;
//		float radius = acosf(dot);
//		DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(DirectX::XMVector3Normalize(Up), RockOnEyeVector);
//		DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, radius);
//		RockOnEyeVector = DirectX::XMVector3Rotate(Up, Quaternion);
//		RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);
//	}
//	else if (dot < -0.8f)
//	{
//		dot = -0.8f;
//		float radius = acosf(dot);
//		DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(DirectX::XMVector3Normalize(Up), RockOnEyeVector);
//		DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, radius);
//		RockOnEyeVector = DirectX::XMVector3Rotate(Up, Quaternion);
//		RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);
//	}
//
//
//	DirectX::XMStoreFloat3(&playerToRockOn, PlayerToRockOn);
//	DirectX::XMStoreFloat3(&rockOnEyeVector, RockOnEyeVector);
//}
//
