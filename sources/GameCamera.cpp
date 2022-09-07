#include "camera.h"

void GameCamera::Initialize(GraphicsPipeline& graphics)
{
	gameInitialize(graphics);
	UpdateViewProjection();
}

void GameCamera::Update(float elapsedTime)
{
	gameUpdate(elapsedTime);
	AttitudeControl(elapsedTime);
	DebugGUI();
	UpdateViewProjection();
}

void GameCamera::gameInitialize(GraphicsPipeline& graphics)
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
	radius = 20;
	EyeVector = DirectX::XMVector3Normalize(EyeVector);
	DirectX::XMStoreFloat3(&eyeVector, EyeVector);

	const DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * 12;
	DirectX::XMStoreFloat3(&target, Target);

	const DirectX::XMVECTOR Eye = Target + EyeVector * radius;
	DirectX::XMStoreFloat3(&eye, Eye);

	up = 12;
	up1 = 8.5f;
}

void GameCamera::gameUpdate(float elapsedTime)
{
#if 0
	using namespace DirectX;

	DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&player->GetVelocity());
	DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&player->GetPosition());
	DirectX::XMVECTOR PlayerTarget = DirectX::XMLoadFloat3(&player->GetTarget());

	const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&player->GetForward());
	const DirectX::XMVECTOR PlayerUp = DirectX::XMLoadFloat3(&player->GetUp());

	DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR CameraForward = DirectX::XMLoadFloat3(&forward);

    const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * 6;
	DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);

	if(player->GetEnemyLockOn())
	{


	}
	else
	{
		SetAngle(elapsedTime);
		if(horizonDegree >0.1f || horizonDegree < -0.1f || verticalDegree > 0.1f || verticalDegree < -0.1f)
		{
			DirectX::XMVECTOR EyeVector = CameraPosition - EyeCenter;
			//const DirectX::XMVECTOR Radius = DirectX::XMVector3Length(EyeVector);
			//DirectX::XMStoreFloat(&radius, Radius);
			EyeVector = DirectX::XMVector3Normalize(EyeVector);
			DirectX::XMStoreFloat3(&eyeVector, EyeVector);

			UpdateEyeVector(elapsedTime, PlayerUp);
			UpdateEye();
			UpdateTarget(PlayerPosition, PlayerUp);
		}
		else
		{
			DirectX::XMVECTOR EyeVector = EyeCenter - CameraPosition;
			EyeVector = DirectX::XMVector3Normalize(EyeVector);

			CameraForward = DirectX::XMVector3Normalize(CameraForward);
			DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EyeVector, CameraForward);
			float dot = DirectX::XMVectorGetX(Dot);

		    if (dot < 0.95f)
			{
				CameraPosition += Velocity * elapsedTime;
				DirectX::XMStoreFloat3(&eye, CameraPosition);
			}
			//CameraPosition += Velocity * elapsedTime;
			//DirectX::XMStoreFloat3(&eye, CameraPosition);

			DirectX::XMVECTOR Target = CameraPosition + CameraForward;
			DirectX::XMStoreFloat3(&target, Target);


		}
	}
#else
    using namespace DirectX;

	const DirectX::XMFLOAT3 playerForward = player->GetForward();
	const DirectX::XMFLOAT3 playerUp = player->GetUp();
	const DirectX::XMFLOAT3 playerPosition = player->GetPosition();

	const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&playerForward);
	const DirectX::XMVECTOR PlayerUp = DirectX::XMLoadFloat3(&playerUp);
	const DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);

	DirectX::XMVECTOR PlayerTarget = DirectX::XMLoadFloat3(&player->GetTarget());

	const DirectX::XMVECTOR EyeCenter = PlayerPosition + PlayerUp * up1;
	DirectX::XMStoreFloat3(&eyeCenter, EyeCenter);

	if(boss_animation)
	{
		DirectX::XMVECTOR EyeVector = DirectX::XMVector3Normalize(PlayerPosition) * 10 + PlayerUp * 1;
		EyeVector = DirectX::XMVector3Normalize(EyeVector);
		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
		boss_animation = false;
	}

    switch (state)
    {
	case CameraState::Free:
		if(player->GetEnemyLockOn())
		{
			state = RockOn;
			break;
		}
		if(player->during_search_time())
		{
			state = AttackStart;
		}
		if (player->GetCameraReset())
		{
			if (CameraReset(elapsedTime, PlayerForward, PlayerUp))
			{
				player->FalseCameraReset();
			}
		}
		SetAngle(elapsedTime);
		UpdateEyeVector(elapsedTime, PlayerUp);
		UpdateTarget(PlayerPosition, PlayerUp);
		UpdateEye();
		break;
	case CameraState::RockOn:
		if(!player->GetEnemyLockOn())
		{
			state = Free;
			break;
		}
		if (player->during_search_time())
		{
			state = AttackStart;
		}
		if(player->GetBehindAvoidance())
		{
			state = Avoiding;
		}
		RockOnCalculateEyeVector(PlayerPosition, PlayerTarget, PlayerUp);
		RockOnUpdateEyeVector(elapsedTime, PlayerUp, player->GetCameraLockOn());
		UpdateTarget(PlayerPosition, PlayerUp);
		UpdateEye();
		break;
	case CameraState::AttackStart:
		if (player->during_search_time())
		{
			SetAngle(elapsedTime);
			UpdateEyeVector(elapsedTime, PlayerUp);
			UpdateTarget(PlayerPosition, PlayerUp);
			UpdateEye();
		}
		if(!player->during_search_time() && !player->during_chain_attack_end())
		{
			state = Free;
			break;
		}
		if(player->during_chain_attack_end())
		{
			state = Attacking;
			break;
		}
		break;
	case CameraState::Attacking:
		UpdateAttackingCamera(elapsedTime);
		if(!player->during_chain_attack_end())
		{
			state = CameraStopAttackEnd;
			timerStart = true;
			break;
		}
		break;
	case CameraState::Avoiding:
		if (player->GetEnemyLockOn())
		{
			avoidTargetPos = player->GetTarget();

			//avoidTargetPos.y = target.y;

			//target = avoidTargetPos;

			//avoidTargetPos = player->GetTarget();
		}

		if (!player->GetBehindAvoidance())
		{
			state = CameraStopEnd;
		}
		break;
	case CameraState::CameraStopEnd:
		if(cameraAvoidEnd)
		{
			cameraAvoidEnd = false;
			state = Free;
			break;
		}
		UpdateStopEndTarget();
		UpdateStopEndEye(DirectX::XMLoadFloat3(&avoidTargetPos), elapsedTime);
		break;
	case CameraState::CameraStopAttackEnd:
		if (cameraAvoidEnd)
		{
			cameraAvoidEnd = false;
			timer = 0.0f;
			state = Free;
			break;
		}
		//UpdateAttackEndTarget(PlayerPosition, PlayerUp, elapsedTime);
		if(timerStart)
		{
			timer += elapsedTime;
		}
		if (timer > AttackEndCameraTimer)
		{
			timerStart = false;
			UpdateTarget(PlayerPosition, PlayerUp);
			UpdateStopAttackEndEye(elapsedTime);
		}
		break;
    }

#endif

}

void GameCamera::SetAngle(float elapsedTime)
{
	float ax = game_pad->get_axis_RX();
	float ay = game_pad->get_axis_RY();

	if (ax > 0.1f || ax < 0.1f)
	{
		horizonDegree = 180 * ax * elapsedTime;
		if (player->during_search_time())
		{
			horizonDegree = 180 * ax * elapsedTime * 0.2f;
		}
	}
	//if (ay > 0.1f || ay < 0.1f)
	//{
	//	verticalDegree = 180 * -ay * elapsedTime;
	//}
}

void GameCamera::UpdateEyeVector(float elapsedTime, DirectX::XMVECTOR PlayerUp)
{
	//変化が無ければスルー
	if (horizonDegree < 0.1f && horizonDegree >-0.1f
		&& verticalDegree < 0.1f && verticalDegree > -0.1f)return;

	DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
	DirectX::XMFLOAT3 up = { 0,1,0 };
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

	DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Up, DirectX::XMVectorScale(EyeVector, -1));

	////プレイヤーに対して縦方向の回転
	////プレイヤーの真上と真下に近いときは回転しない
	//if (verticalDegree > 0.1f || verticalDegree < -0.1f)
	//{
	//	DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(Up), DirectX::XMVector3Normalize(EyeVector));
	//	float dot{};
	//	DirectX::XMStoreFloat(&dot, Dot);
	//	//真上に近い時
	//	if (dot > 0.9f)
	//	{
	//		if (verticalDegree < 0.1f)
	//		{
	//			DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
	//			DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
	//			EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
	//		}
	//		verticalDegree = 0;
	//	}
	//	//真下に近い時
	//	else if (dot < -0.9f)
	//	{
	//		if (verticalDegree > 0.1f)
	//		{
	//			DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
	//			DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
	//			EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
	//		}
	//		verticalDegree = 0;
	//	}
	//	//通常時
	//	else
	//	{
	//		DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
	//		DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
	//		EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
	//		verticalDegree = 0;
	//	}
	//}
	//プレイヤーに対して横の回転
	if (horizonDegree > 0.1f || horizonDegree < -0.1f)
	{
		DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Up);
		DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(horizonDegree));
		EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
		horizonDegree = 0;
	}


	//最後に一回だけ行う
	DirectX::XMStoreFloat3(&eyeVector, EyeVector);
}

void GameCamera::AttitudeControl(float elapsedTime)
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

void GameCamera::UpdateEye()
{
	using namespace DirectX;

	const DirectX::XMVECTOR EyeCenter = DirectX::XMLoadFloat3(&eyeCenter);
	const DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
	const DirectX::XMVECTOR Eye = EyeCenter + EyeVector * radius;
	DirectX::XMStoreFloat3(&eye, Eye);
}

void GameCamera::UpdateTarget(DirectX::XMVECTOR PlayerPosition, DirectX::XMVECTOR PlayerUp)
{
	using namespace DirectX;

	DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * up1;
	DirectX::XMStoreFloat3(&target, Target);

	//DirectX::XMVECTOR CameraForward = DirectX::XMLoadFloat3(&forward);
	//Target = CameraForward;

}

void GameCamera::UpdateRockOnTarget(DirectX::XMVECTOR PlayerTarget)
{
	DirectX::XMVECTOR Target = PlayerTarget;
	DirectX::XMStoreFloat3(&target, Target);
}

bool GameCamera::CameraReset(float elapsedTime, DirectX::XMVECTOR PlayerForward, DirectX::XMVECTOR PlayerUp)
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

bool GameCamera::RockOnCameraReset(float elapsedTime, DirectX::XMVECTOR PlayerForward, DirectX::XMVECTOR PlayerUp)
{
	static float rotateTimer{ 0 };
	using namespace DirectX;

	if (rotateTimer < 0.1f)
	{
		rotateTimer += elapsedTime;
		DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
		DirectX::XMVECTOR RotateEyeVector = DirectX::XMLoadFloat3(&rockOnEyeVector);
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

void GameCamera::UpdateStopEndTarget()
{
	if(player->GetEnemyLockOn())
	    avoidTargetPos = player->GetTarget();

	avoidTargetPos.y = target.y;

	target = avoidTargetPos;
}

void GameCamera::UpdateStopEndEye(DirectX::XMVECTOR PlayerTarget,float elapsedTime)
{
	using namespace DirectX;
	DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);

	DirectX::XMFLOAT3 playerPosition = player->GetPosition();
	playerPosition.y = DirectX::XMVectorGetY(PlayerTarget);
	DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);

	DirectX::XMVECTOR PlayerToTarget = PlayerTarget - PlayerPosition;
	PlayerToTarget = DirectX::XMVector3Normalize(PlayerToTarget);
	DirectX::XMVECTOR CameraGoalPositionVec = -PlayerToTarget * 10 + DirectX::XMLoadFloat3(&player->GetUp()) * 2;
	const DirectX::XMVECTOR CameraGoalPosition = DirectX::XMLoadFloat3(&eyeCenter) + CameraGoalPositionVec;

	DirectX::XMVECTOR CameraToGoal = CameraGoalPosition - CameraPosition;
	const DirectX::XMVECTOR CameraToGoalLength = DirectX::XMVector3Length(CameraToGoal);
	const float cameraToGoalLength = DirectX::XMVectorGetX(CameraToGoalLength);

	if (!attackEnd)
	{
		attackEndSpeed = cameraToGoalLength * lerpLate;
		attackEnd = true;
	}

	if (cameraToGoalLength > radius - 0.2f)
	{
		CameraPosition += CameraToGoal * attackEndSpeed * elapsedTime;
		DirectX::XMStoreFloat3(&eye, CameraPosition);
	}
	else
	{
		CameraGoalPositionVec = DirectX::XMVector3Normalize(CameraGoalPositionVec);
		DirectX::XMStoreFloat3(&eyeVector, CameraGoalPositionVec);
		cameraAvoidEnd = true;
		attackEnd = false;
	}
}

void GameCamera::UpdateAttackEndTarget(DirectX::XMVECTOR PlayerPosition, DirectX::XMVECTOR PlayerUp,float elapsedTime)
{
	using namespace DirectX;
	static float rotateTimer{ 0 };

	if (rotateTimer < 0.1f)
	{
		DirectX::XMVECTOR Target = DirectX::XMLoadFloat3(&target);
		DirectX::XMVECTOR Target2 = PlayerPosition + PlayerUp * up;

		DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);

		Target = DirectX::XMVector3Normalize(Target - CameraPosition);
		Target2 = DirectX::XMVector3Normalize(Target2 - CameraPosition);

		const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Target, Target2);
		float dot = DirectX::XMVectorGetX(Dot);

		if (dot < 1.0f)
		{
			dot = acosf(dot);
			const DirectX::XMFLOAT3 up{ 0.0f,1.0f,0.0f };
			DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
			DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(Up, dot * elapsedTime / 0.1f);
			Target = DirectX::XMVector3Rotate(Target, Q);
		}
		DirectX::XMStoreFloat3(&target, Target);
	}
	else
	{
		DirectX::XMVECTOR Target = PlayerPosition + PlayerUp * up;
		DirectX::XMStoreFloat3(&target, Target);
	}
	//DirectX::XMVECTOR CameraForward = DirectX::XMLoadFloat3(&forward);
	//Target = CameraForward;

}

void GameCamera::UpdateStopAttackEndEye(float elapsedTime)
{
	using namespace DirectX;
	DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&player->GetPosition());

	DirectX::XMVECTOR CameraToPlayer = PlayerPosition - CameraPosition;
	CameraToPlayer = DirectX::XMVector3Normalize(CameraToPlayer);

	DirectX::XMVECTOR CameraGoalPositionVec = -CameraToPlayer * 10 + DirectX::XMLoadFloat3(&player->GetUp()) * 2;
	CameraGoalPositionVec = DirectX::XMVector3Normalize(CameraGoalPositionVec);
	const DirectX::XMVECTOR CameraGoalPosition = DirectX::XMLoadFloat3(&eyeCenter) + CameraGoalPositionVec * radius;

	DirectX::XMVECTOR CameraToGoal = CameraGoalPosition - CameraPosition;
	const DirectX::XMVECTOR CameraToGoalLength = DirectX::XMVector3Length(CameraToGoal);
	const float cameraToGoalLength = DirectX::XMVectorGetX(CameraToGoalLength);

	if(!attackEnd)
	{
		attackEndSpeed = cameraToGoalLength * lerpLate;
		attackEnd = true;
	}

    //CameraToGoal = DirectX::XMVector3Normalize(CameraToGoal);

	if (cameraToGoalLength > radius - 0.2f)
	{
		CameraPosition += CameraToGoal * attackEndSpeed * elapsedTime;
		DirectX::XMStoreFloat3(&eye, CameraPosition);
	}
	else
	{
		cameraAvoidEnd = true;
		attackEnd = false;
	}
}

void GameCamera::UpdateAttackingCamera(float elapsedTime)
{
	using namespace DirectX;
	DirectX::XMFLOAT3 forward = GetForward();
	forward.y = 0.0f;
	DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
	Forward = DirectX::XMVector3Normalize(Forward);

	DirectX::XMFLOAT3 playerPosition = player->GetPosition();
	playerPosition.y = eye.y;
	const DirectX::XMVECTOR PlayerPosition = DirectX::XMLoadFloat3(&playerPosition);

	const DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);

	DirectX::XMVECTOR CameraToPlayer = PlayerPosition - CameraPosition;
	CameraToPlayer = DirectX::XMVector3Normalize(CameraToPlayer);

	const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Forward, CameraToPlayer);
	float angle = DirectX::XMVectorGetX(Dot);

	if(angle < 0.9f)
	{
    	angle = acosf(angle);
		AttackingUpdateTarget(angle, CameraToPlayer);
	}

}

void GameCamera::AttackingUpdateTarget(float angle, DirectX::XMVECTOR CameraToPlayer)
{
	using namespace DirectX;
	const DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&target) - CameraPosition;
	DirectX::XMFLOAT3 forward{};
	DirectX::XMStoreFloat3(&forward, Forward);
	forward.y = 0.0f;
	Forward = DirectX::XMLoadFloat3(&forward);

	const DirectX::XMFLOAT3 up{ 0.0f,1.0f,0.0f };
	const DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

	DirectX::XMFLOAT3 right = GetRight();
	right.y = 0.0f;
	DirectX::XMVECTOR Right = DirectX::XMLoadFloat3(&right);

	DirectX::XMVECTOR D = DirectX::XMVector3Dot(CameraToPlayer, Right);
	float d = DirectX::XMVectorGetX(D);

	float rightOrLeft{}; //1 : 右 , -1 : 左
	if (d > 0)rightOrLeft = 1;
	else rightOrLeft = -1;

	float r{};
	r = 0.45f;
	DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(Up, r * rightOrLeft);
	DirectX::XMVECTOR RotateVec = DirectX::XMVector3Rotate(Forward, Q);
	RotateVec = DirectX::XMVector3Normalize(RotateVec);

	const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(RotateVec, CameraToPlayer);
	float dot = DirectX::XMVectorGetX(Dot);

	dot = acosf(dot);
	Q = DirectX::XMQuaternionRotationAxis(Up, dot * rightOrLeft);
	Forward = DirectX::XMLoadFloat3(&target) - CameraPosition;
	DirectX::XMVECTOR RotateForward = DirectX::XMVector3Rotate(Forward, Q);

	const DirectX::XMVECTOR Target = CameraPosition + RotateForward;
	DirectX::XMStoreFloat3(&target, Target);
}

bool GameCamera::RockOnUpdateEyeVector(float elapsedTime, DirectX::XMVECTOR PlayerUp, bool rockOnStart)
{
	if (rockOnStart)
	{
		if (rockOnTimer < 0.1f)
		{
			rockOnTimer += elapsedTime;
			DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
			DirectX::XMVECTOR RotateEyeVector = DirectX::XMLoadFloat3(&rockOnEyeVector);
			RotateEyeVector = DirectX::XMVector3Normalize(RotateEyeVector);

			const DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EyeVector, RotateEyeVector);
			float dot{};
			DirectX::XMStoreFloat(&dot, Dot);
			const float angle = acosf(dot);

			const DirectX::XMFLOAT3 worldUp{ 0.0f,1.0f,0.0f };
			const DirectX::XMVECTOR Axis = DirectX::XMLoadFloat3(&worldUp);
			//if(DirectX::XMVector3Equal(DirectX::XMVectorZero(), Axis))
			//{
			//	DirectX::XMStoreFloat3(&eyeVector, EyeVector);
			//	return false;
			//}
			if (dot < 0.98f)
			{
				DirectX::XMFLOAT3 right = GetRight();
				right.y = 0.0f;
				DirectX::XMVECTOR Right = DirectX::XMLoadFloat3(&right);

				DirectX::XMVECTOR D = DirectX::XMVector3Dot(EyeVector, Right);
				float d = DirectX::XMVectorGetX(D);

				float rightOrLeft{}; //1 : 右 , -1 : 左
				if (d > 0)rightOrLeft = -1;
				else rightOrLeft = 1;

				using namespace DirectX;
				const DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis , rightOrLeft * angle * elapsedTime / 0.1f);
				EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
			}
			DirectX::XMStoreFloat3(&eyeVector, EyeVector);
			return false;
		}
		else
		{
			eyeVector = rockOnEyeVector;
			rockOnTimer = 0;
			player->FalseCameraLockOn();
			return true;
		}
	}
	else
	{
		eyeVector = rockOnEyeVector;
		//using namespace DirectX;

		////左右のずれ修正
		DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat3(&eyeVector);
		//DirectX::XMVECTOR RockOnEyeVector = DirectX::XMLoadFloat3(&rockOnEyeVector);

		//DirectX::XMFLOAT3 worldUp{ 0.0f,1.0f,0.0f };
		//DirectX::XMVECTOR WorldUp = DirectX::XMLoadFloat3(&worldUp);

		//DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(EyeVector, RockOnEyeVector);
		//float dot{};
		//dot = DirectX::XMVectorGetX(Dot);
		//float radian = acosf(dot);

		//if(fabsf(radian) > 1e-8)
		//{
		//	DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationAxis(WorldUp, DirectX::XMConvertToDegrees(radian));
		//	EyeVector = DirectX::XMVector3Rotate(EyeVector, Q);
		//}

#if 0
		//変化が無ければスルー
		if (horizonDegree < 0.1f && horizonDegree >-0.1f
			&& verticalDegree < 0.1f && verticalDegree > -0.1f) {
		}
		else
		{
			DirectX::XMFLOAT3 up = { 0,1,0 };
			DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);

			DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Up, DirectX::XMVectorScale(EyeVector, -1));

			//プレイヤーに対して縦方向の回転
			//プレイヤーの真上と真下に近いときは回転しない
			if (verticalDegree > 0.1f || verticalDegree < -0.1f)
			{
				DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(DirectX::XMVector3Normalize(Up), DirectX::XMVector3Normalize(EyeVector));
				float dot{};
				DirectX::XMStoreFloat(&dot, Dot);
				//真上に近い時
				if (dot > 0.9f)
				{
					if (verticalDegree < 0.1f)
					{
						DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
						DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
						EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
					}
					verticalDegree = 0;
				}
				//真下に近い時
				else if (dot < -0.9f)
				{
					if (verticalDegree > 0.1f)
					{
						DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
						DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
						EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
					}
					verticalDegree = 0;
				}
				//通常時
				else
				{
					DirectX::XMVECTOR Axis = DirectX::XMVector3Normalize(Right);
					DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, DirectX::XMConvertToRadians(verticalDegree));
					EyeVector = DirectX::XMVector3Rotate(EyeVector, Quaternion);
					verticalDegree = 0;
				}
			}
		}
#endif

		//最後に一回だけ行う
		DirectX::XMStoreFloat3(&eyeVector, EyeVector);
		return false;
	}
}

void GameCamera::RockOnCalculateEyeVector(DirectX::XMVECTOR PlayerPosition, DirectX::XMVECTOR RockOnPosition, DirectX::XMVECTOR PlayerUp)
{
	using namespace DirectX;

#if 1
	DirectX::XMVECTOR PlayerToRockOn = RockOnPosition - PlayerPosition;
	const DirectX::XMVECTOR Distance = DirectX::XMVector3Length(PlayerToRockOn);
	float distance{};
	DirectX::XMStoreFloat(&distance, Distance);

	const DirectX::XMVECTOR CameraRockOnPosition = PlayerToRockOn * 1.0f;

	PlayerToRockOn = DirectX::XMVector3Normalize(PlayerToRockOn);
	//const DirectX::XMFLOAT3 playerForward = player->GetForward();
	//const DirectX::XMVECTOR PlayerForward = DirectX::XMLoadFloat3(&playerForward);


	//DirectX::XMVECTOR RockOnEyeVector = -PlayerToRockOn * 10 + PlayerUp;
	DirectX::XMVECTOR RockOnEyeVector = -PlayerToRockOn * 10 + PlayerUp * 1;
	RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);

#else
	DirectX::XMVECTOR PlayerToRockOn = RockOnPosition - PlayerPosition;
	PlayerToRockOn = DirectX::XMVector3Normalize(PlayerToRockOn);


	//DirectX::XMVECTOR RockOnEyeVector = -PlayerToRockOn * 10 + Up;
	DirectX::XMVECTOR RockOnEyeVector = RockOnPosition - DirectX::XMLoadFloat3(&eyeCenter);
	RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);
#endif

	////真上、真下に近いとき修正
	//DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(RockOnEyeVector, DirectX::XMVector3Normalize(Up));
	//float dot{};
	//DirectX::XMStoreFloat(&dot, Dot);
	//if (dot > 0.8f)
	//{
	//	dot = 0.8f;
	//	float radius = acosf(dot);
	//	DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(DirectX::XMVector3Normalize(Up), RockOnEyeVector);
	//	DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, radius);
	//	RockOnEyeVector = DirectX::XMVector3Rotate(Up, Quaternion);
	//	RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);
	//}
	//else if (dot < -0.8f)
	//{
	//	dot = -0.8f;
	//	float radius = acosf(dot);
	//	DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(DirectX::XMVector3Normalize(Up), RockOnEyeVector);
	//	DirectX::XMVECTOR Quaternion = DirectX::XMQuaternionRotationAxis(Axis, radius);
	//	RockOnEyeVector = DirectX::XMVector3Rotate(Up, Quaternion);
	//	RockOnEyeVector = DirectX::XMVector3Normalize(RockOnEyeVector);
	//}


	//DirectX::XMStoreFloat3(&playerToRockOn, PlayerToRockOn);
	DirectX::XMStoreFloat3(&rockOnEyeVector, RockOnEyeVector);
}

