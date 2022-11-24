#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "ClientPlayer.h"
#include"Correspondence.h"
#include"user.h"
#include"DebugConsole.h"


void ClientPlayer::ChainSearchUpdate(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& graphics_)
{
	for (int i = 0; i < enemies.size(); ++i)
	{
		for (const auto& id : receive_chain_lock_on_enemy_id)
		{
			//-----チェイン攻撃のロックオンしたIDでなければとばす-----//
			if (enemies.at(i)->fGetObjectId() != id) continue;

			//-----必要なデータを設定-----//
			chain_lockon_enemy_indexes.emplace_back(i); // 登録
			LockOnSuggest enemy_suggest; // サジェスト登録
			enemy_suggest.position = enemies.at(i)->fGetPosition();
			lockon_suggests.emplace_back(enemy_suggest);
			enemies.at(i)->fSetIsLockOnOfChain(true);
			DebugConsole::Instance().WriteDebugConsole("データ設定", TextColor::SkyBlue);
		}
	}

	DebugConsole::Instance().WriteDebugConsole("チェイン攻撃索敵完了");
	TransitionChainLockonBegin();
}

void ClientPlayer::TransitionChainSearch()
{

	if (!chain_lockon_enemy_indexes.empty()) chain_lockon_enemy_indexes.clear();
	if (!lockon_suggests.empty()) lockon_suggests.clear();
	if (!reticles.empty()) { reticles.clear(); }

	// 初めの要素にはプレイヤーを入れる
	LockOnSuggest player_suggest;
	player_suggest.position = position;
	player_suggest.detection = true; // プレイヤーは検索には入れないので初めからtrue
	lockon_suggests.emplace_back(player_suggest);


	player_chain_activity = &ClientPlayer::ChainSearchUpdate;
}

void ClientPlayer::ChainLockonBeginUpdate(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
	if (model->end_of_animation(anim_parm))
	{
		DebugConsole::Instance().WriteDebugConsole("ロックオン開始", TextColor::SkyBlue);
		TransitionChainLockon();
	}
}

void ClientPlayer::TransitionChainLockonBegin()
{
	is_chain_attack = true;
	is_chain_attack_aftertaste = true;
	is_chain_attack_aftertaste_timer = 0;

	if (is_awakening) { model->play_animation(anim_parm, AnimationClips::AwakingChargeInit, false, true, 0.1f, 3.0f); }
	else { model->play_animation(anim_parm, AnimationClips::ChargeInit, false, true, 0.1f, 3.0f); }
	player_chain_activity = &ClientPlayer::ChainLockonBeginUpdate;

	// velocityクリア
	velocity = {};

}

void ClientPlayer::ChainLockonUpdate(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
	static float base_speed = 100.0f;
	static float base_play = 1.0f;

	// speed増減
	const float MAX_LENGTH = 100.0f;
	float camera_to_player_length = Math::calc_vector_AtoB_length(camera_position, position);
	camera_to_player_length = (std::min)(camera_to_player_length, MAX_LENGTH);
	float magnification_speed = camera_to_player_length / MAX_LENGTH * 10.0f;
	float speed = magnification_speed * magnification_speed + base_speed;
	// play増減
	const float ADD_PLAY = 1.0f;
	float play = base_play + (camera_to_player_length / MAX_LENGTH) * ADD_PLAY;

	// 攻撃
	if (Transit(elapsed_time, transit_index, position, speed, interpolated_way_points, play))
	{
		assert(transit_index != 0 && "意図していない挙動になっています");
		audio_manager->play_se(SE_INDEX::ATTACK_SWORD);
		audio_manager->play_se(SE_INDEX::AVOIDANCE);
		if (is_awakening) combo_count -= COMBO_COUNT_SUB;
		TransitionChainAttack(); // 攻撃ステートへ
	}

	// 軌跡
	if (is_awakening)
	{
		mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
		mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
	}
	else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);

}

void ClientPlayer::TransitionChainLockon()
{
	if (!sort_points.empty()) sort_points.clear();
	if (!way_points.empty()) way_points.clear();
	if (!interpolated_way_points.empty()) interpolated_way_points.clear();

	sort_points.emplace_back(position); // ソートされた位置の最初はプレイヤーの位置

	// プレイヤーとロックオンされた敵をプレイヤーと始点として基準距離以上の中で一番近い敵を次の要素に入れ、その敵を次の始点として同じ条件でソートする
	const float REFERENCE_DISTANCE = 10.0f; // 基準距離
	for (int start = 0; start < lockon_suggests.size() - 1; ++start) // 始点側
	{
		float champion_length{ FLT_MAX }; // 基準距離以上で最も始点に近い距離
		DirectX::XMFLOAT3 champion_pos{}; // 基準距離以上で最も始点に近い位置
		int champion_index = -1;          // 基準距離以上で最も始点に近い終点側のインデックス
		for (int end = 0; end < lockon_suggests.size(); ++end) // 終点側
		{
			float length = Math::calc_vector_AtoB_length(lockon_suggests.at(start).position, lockon_suggests.at(end).position);
			if (start != end && !lockon_suggests.at(end).detection && length >= REFERENCE_DISTANCE) // 検索条件：未検索かつ基準距離以上
			{
				if (champion_length > length)
				{
					champion_length = length;
					champion_pos = lockon_suggests.at(end).position;
					champion_index = end;
				}
			}
		}

		//-----< 始点側startから終点側endの総当たり終了 >-----//

		// 基準距離以上で最も始点に近い距離が見つかったので登録
		if (champion_index > -1)
		{
			lockon_suggests.at(champion_index).detection = true; // 検索済み
			sort_points.emplace_back(champion_pos);
		}
		else // 基準距離以上で最も始点に近い距離が見つからなかった
			 // パターン１：終点側が全て基準距離以内だった(未検索がまだある)→基準距離を0より大きく(始点側が未検索だった時対策)して再検索(検索済みははじく)
			 // パターン２：終点側の未検索がなかった→始点側を登録する
		{
			bool all_searching = true; // 終点側が全て検索されているかどうか
			for (int end = 0; end < lockon_suggests.size(); ++end) // 終点側
			{
				if (start != end && !lockon_suggests.at(end).detection) // 終点側( start != end )が未検索
				{
					all_searching = false; // 一つでも未検索があればパターン1確定
					break;
				}
			}
			// パターン1
			if (!all_searching) // 基準距離を0より大きくして再検索  検索条件：未検索かつ距離が0より大きい
			{
				// リセット
				champion_length = { FLT_MAX };
				champion_pos = {};
				champion_index = -1;
				for (int end = 0; end < lockon_suggests.size(); ++end) // 終点側
				{
					float length = Math::calc_vector_AtoB_length(lockon_suggests.at(start).position, lockon_suggests.at(end).position);
					if (start != end && !lockon_suggests.at(end).detection && length > 0) // 検索条件：未検索かつ距離が0より大きい
					{
						if (champion_length > length)
						{
							champion_length = length;
							champion_pos = lockon_suggests.at(end).position;
							champion_index = end;
						}
					}
					//--------<!! ここで距離が0以下ならまったく同じ位置にポイントがある !!>--------//
					else if (start != end && !lockon_suggests.at(end).detection && length <= 0)
					{
						champion_pos = lockon_suggests.at(end).position;
						champion_index = end;
					}
				}
				assert(champion_index != -1 && "パターン1のchampion_indexが見つかっていません");
				assert(champion_index != start && "パターン1のchampion_indexが始点側になっています");
				lockon_suggests.at(champion_index).detection = true; // 検索済み
				sort_points.emplace_back(champion_pos); // 再検索終了
			}
			// パターン2
			else { sort_points.emplace_back(lockon_suggests.at(start).position); } // 始点側を登録
		}
	}

	assert(lockon_suggests.size() == sort_points.size() && "ソート前とソート後の要素数が異なっています");

	// ソートされたポイントからそれぞれの中間点を算出、そのすべてを通るように任意の分割数で分割したポイントを算出
	way_points.emplace_back(sort_points.at(0)); // プレイヤーの位置
	for (int index = 0; index < sort_points.size() - 1; ++index)
	{
		DirectX::XMFLOAT3 start = sort_points.at(index);
		DirectX::XMFLOAT3 end = sort_points.at(index + 1);
		// 右方向ベクトルとターゲットへのベクトルの角度を算出
		DirectX::XMVECTOR norm_vec = Math::calc_vector_AtoB_normalize(start, end);
		DirectX::XMFLOAT3 right = { 1,0,0 };
		DirectX::XMVECTOR right_vec = DirectX::XMLoadFloat3(&right);
		DirectX::XMVECTOR dot = DirectX::XMVector3Dot(norm_vec, right_vec);
		float cos_theta = DirectX::XMVectorGetX(dot);
		// 右方向ベクトルとターゲットへのベクトルの角度と任意の角度を足したものが中間地点のθ
		const float theta = acosf(cos_theta) + DirectX::XMConvertToRadians(30.0f);
		// ターゲットへのベクトルの長さを算出
		float length_vec = Math::calc_vector_AtoB_length(start, end);
		// 中間地点の算出
		DirectX::XMFLOAT3 midpoint;
		if (start.z <= end.z) { midpoint = { start.x + cosf(theta) * length_vec * 0.5f, start.y, start.z + sinf(theta) * length_vec * 0.5f }; }
		else { midpoint = { start.x + cosf(theta) * length_vec * 0.5f, start.y, start.z + -sinf(theta) * length_vec * 0.5f }; }
		// way_pointsの生成
		way_points.emplace_back(midpoint); // プレイヤーの位置と敵の位置の中間地点
		way_points.emplace_back(end); // 敵の位置
	}

	// way_pointsを通るカーブを作成
	CatmullRomSpline curve(way_points);
	curve.interpolate(interpolated_way_points, STEPS);

	transit_index = 0;
	if (is_awakening) { model->play_animation(anim_parm, AnimationClips::AwakingCharge, true); }
	else { model->play_animation(anim_parm, AnimationClips::Charge, true); }
	player_chain_activity = &ClientPlayer::ChainLockonUpdate;

}

void ClientPlayer::ChainMoveUpdate(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
}

void ClientPlayer::TransitionChainMove()
{
	player_chain_activity = &ClientPlayer::ChainMoveUpdate;
}


void ClientPlayer::ChainAttackUpdate(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
	// 分割したポイントの最後なら通常行動へ
	if (transit_index >= interpolated_way_points.size() - 1)
	{
		// ロックオンされた敵にダメージ
		for (const auto& enemy : enemies)
		{
			if (!enemy->fIsLockOnOfChain()) continue; // ダメージを与えるのはロックオンされた敵のみ
			enemy->fDamaged(25, 0.3f, Graphics_, elapsed_time);
			enemy->fSetLaunchDissolve();
			enemy->fSetIsLockOnOfChain(false);
		}

		is_chain_attack = false;

		TransitionChainSearch(); /* リセット */
		TransitionNormalBehavior();
	}
	else // ロックオンステートの初期化を通らず更新処理へ
	{
		player_chain_activity = &ClientPlayer::ChainLockonUpdate;
		// 初期化通らないのでここでもアニメーション再生
		if (is_awakening) { model->play_animation(anim_parm, AnimationClips::AwakingCharge, true); }
		else { model->play_animation(anim_parm, AnimationClips::Charge, true); }
	}

}

void ClientPlayer::TransitionChainAttack()
{
	switch (attack_type)
	{
	case ATTACK_TYPE::FIRST:
		if (is_awakening) { model->play_animation(anim_parm, AnimationClips::AwakingAttackType1, false, true, 0.1f, 5.0f); }
		else { model->play_animation(anim_parm, AnimationClips::AttackType1, false, true, 0.1f, 5.0f); }
		attack_type = ATTACK_TYPE::SECOND;
		break;

	case ATTACK_TYPE::SECOND:
		if (is_awakening) { model->play_animation(anim_parm, AnimationClips::AwakingAttackType2, false, true, 0.1f, 5.0f); }
		else { model->play_animation(anim_parm, AnimationClips::AttackType2, false, true, 0.1f, 5.0f); }
		attack_type = ATTACK_TYPE::THIRD;
		break;

	case ATTACK_TYPE::THIRD:
		if (is_awakening) { model->play_animation(anim_parm, AnimationClips::AwakingAttackType3, false, true, 0.1f, 5.0f); }
		else { model->play_animation(anim_parm, AnimationClips::AttackType3, false, true, 0.1f, 5.0f); }
		attack_type = ATTACK_TYPE::FIRST;
		break;
	}

	player_chain_activity = &ClientPlayer::ChainAttackUpdate;
}

bool ClientPlayer::Transit(float elapsed_time, int& index, DirectX::XMFLOAT3& position, float speed, const std::vector<DirectX::XMFLOAT3>& points, float play)
{
	assert(!points.empty() && "ポイントのサイズが0です");
	if (index >= points.size() - 1)
	{
		// 完全に重ねるとバグるかもなのですこしずらす
		position.x = points.at(index).x + 0.05f;
		position.y = 0;
		position.z = points.at(index).z + 0.05f;
		return true;
	}

	using namespace DirectX;
	XMFLOAT3 velocity{};

	XMVECTOR vec = XMLoadFloat3(&points.at(index + 1)) - XMLoadFloat3(&position);
	XMVECTOR norm_vec = XMVector3Normalize(vec);
	XMStoreFloat3(&velocity, norm_vec);

	Rotate(elapsed_time, index, points);


	XMVECTOR length_vec = DirectX::XMVector3Length(vec);
	float length = DirectX::XMVectorGetX(length_vec);
	if (length <= play)
	{
		++index;
		// 完全に重ねるとバグるかもなのですこしずらす
		position.x = points.at(index).x + 0.05f;
		position.y = 0;
		position.z = points.at(index).z + 0.05f;

		if (index != 0 && index % (STEPS * 2) == 0) // 敵のポイントについた
		{
			// player_slash_hit_effec
			return true;
		}
	}
	else
	{
		position.x += velocity.x * speed * elapsed_time;
		position.y += velocity.y * speed * elapsed_time;
		position.z += velocity.z * speed * elapsed_time;
	}

	return false;
}

void ClientPlayer::Rotate(float elapsed_time, int index, const std::vector<DirectX::XMFLOAT3>& points)
{
	using namespace DirectX;
	//ターゲットに向かって回転
	XMVECTOR orientation_vec = DirectX::XMLoadFloat4(&orientation);
	DirectX::XMVECTOR forward, up;
	DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientation_vec);
	DirectX::XMFLOAT4X4 m4x4 = {};
	DirectX::XMStoreFloat4x4(&m4x4, m);
	up = { 0, 1, 0 };
	forward = { m4x4._31, m4x4._32, m4x4._33 };
	DirectX::XMFLOAT3 front{};
	DirectX::XMStoreFloat3(&front, forward);

	XMFLOAT3 d{};
	XMVECTOR d_vec = XMLoadFloat3(&points.at(index + 1)) - XMLoadFloat3(&points.at(index));
	XMVECTOR norm_vec = XMVector3Normalize(d_vec);
	XMStoreFloat3(&d, norm_vec);
	{
		XMVECTOR dot = XMVector3Dot(forward, norm_vec);
		float angle = acosf(DirectX::XMVectorGetX(dot));
		if (fabs(angle + FLT_EPSILON) > FLT_EPSILON)
		{
			XMVECTOR q;
			float cross{ (d.x * front.z) - (d.z * front.x) };
			if (cross > 0)
			{
				q = XMQuaternionRotationAxis(up, angle);//正の方向に動くクオータニオン
			}
			else
			{
				q = XMQuaternionRotationAxis(up, -angle);//正の方向に動くクオータニオン
			}
#if 0
			XMVECTOR Q = XMQuaternionMultiply(orientation_vec, q);
			orientation_vec = XMQuaternionSlerp(orientation_vec, Q, 10.0f * elapsed_time);
#else
			orientation_vec = XMQuaternionMultiply(orientation_vec, q);
#endif
		}
	}

	DirectX::XMStoreFloat4(&orientation, orientation_vec);
}

void ClientPlayer::TransitionChainBehavior()
{
	behavior_state = Behavior::Chain;
	TransitionChainSearch();
}

void ClientPlayer::TransitionNormalBehavior()
{
	behavior_state = Behavior::Normal;
	//-----チェイン攻撃のデータを削除-----//
	receive_chain_lock_on_enemy_id.clear();

	DebugConsole::Instance().WriteDebugConsole("チェイン攻撃終了", TextColor::SkyBlue);
}

void ClientPlayer::ChainParmReset()
{
	chain_cancel = true;
	TransitionChainSearch(); /*リセット*/
	TransitionNormalBehavior();
	DebugConsole::Instance().WriteDebugConsole("チェイン攻撃終了", TextColor::SkyBlue);
}

void ClientPlayer::ChainIdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
	if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
	{
		TransitionChainMoveAnim(0.3f);
	}
	UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, sky_dome);
}

void ClientPlayer::ChainMoveAnimUpdate(float elapsed_time, SkyDome* sky_dome)
{
	//移動入力がなくなったら待機に遷移
	if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
	{
		TransitionChainIdle();
	}
	UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, sky_dome);
}

void ClientPlayer::TransitionChainIdle(float blend_second)
{
	if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingIdle, true, true, blend_second);
	//通常状態の待機アニメーションにセット
	else model->play_animation(anim_parm, AnimationClips::Idle, true, true, blend_second);
	//攻撃中かどうかの設定
	is_attack = false;
	//アニメーション速度
	animation_speed = 1.0f;
	//アニメーションをしていいかどうか
	is_update_animation = true;
	//待機状態の時の更新関数に切り替える
	chain_activity = &ClientPlayer::ChainIdleUpdate;

}

void ClientPlayer::TransitionChainMoveAnim(float blend_second)
{
	if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingMove, true, true, blend_second);
	//通常状態の時に移動アニメーションの設定
	else model->play_animation(anim_parm, AnimationClips::Move, true, true, blend_second);
	//攻撃中かどうかの設定
	is_attack = false;
	//アニメーション速度
	animation_speed = 1.0f;
	//アニメーションをしていいかどうか
	is_update_animation = true;
	//移動状態の時の更新関数に切り替える
	chain_activity = &ClientPlayer::ChainMoveAnimUpdate;

}
