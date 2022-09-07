#include <assert.h>
#include <memory>
#include "SwordTrail.h"
#include "Player.h"

#include "BaseCamera.h"

bool Player::transit(float elapsed_time, int& index, DirectX::XMFLOAT3& position, float speed,
	const std::vector<DirectX::XMFLOAT3>& points, float play)
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

	rotate(elapsed_time, index, points);


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
			player_slash_hit_effec->play(effect_manager->get_effekseer_manager(), points.at(index), 0.8f);
			player_slash_hit_effec->set_quaternion(effect_manager->get_effekseer_manager(), orientation);

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

void Player::rotate(float elapsed_time, int index, const std::vector<DirectX::XMFLOAT3>& points)
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

void Player::lockon_post_effect(float elapsed_time, std::function<void(float, float)> effect_func, std::function<void()> effect_clear_func)
{
	if (during_search_time())
	{
		float rate = 1.0f;
		if (frame_time <= SEARCH_TIME)
		{
			frame_time += elapsed_time;
			if (frame_time >= SEARCH_TIME) { frame_time = SEARCH_TIME; }
			rate = frame_time / SEARCH_TIME;
			rate *= rate;
		}

		if (rate <= 1.0f)
		{
			frame_scope = 0.5f - rate * (0.5f - ROCKON_FRAME);
			frame_alpha += elapsed_time * 0.1f * 3.0f / SEARCH_TIME;
			frame_alpha = (std::min)(frame_alpha, 0.3f);
			if (rate > 0.98f) { effect_clear_func(); }
			else effect_func(frame_scope, frame_alpha);
		}
	}
	if (chain_cancel)
	{
		effect_clear_func();
		chain_cancel = false;
	}
}

void Player::ChainLockOn()
{


}


void Player::chain_parm_reset()
{
	chain_cancel = true;
	transition_chain_search(); /*リセット*/
	transition_normal_behavior();
}

//#define CHAIN_DEBUG

#ifdef CHAIN_DEBUG
bool debug_lockon = false;
bool debug_transition_chain_lockon_flg = false;
bool attack_start = false;
#endif // CHAIN_DEBUG
void Player::transition_chain_search()
{
	if (!chain_lockon_enemy_indexes.empty()) chain_lockon_enemy_indexes.clear();
	if (!lockon_suggests.empty()) lockon_suggests.clear();
	if (!reticles.empty()) { reticles.clear(); }

	// 初めの要素にはプレイヤーを入れる
	LockOnSuggest player_suggest;
	player_suggest.position = position;
	player_suggest.detection = true; // プレイヤーは検索には入れないので初めからtrue
	lockon_suggests.emplace_back(player_suggest);

	SEARCH_TIME = 0.5f;
	search_time = SEARCH_TIME;
	setup_search_time = false;

#ifdef CHAIN_DEBUG
	debug_lockon = false;
	debug_transition_chain_lockon_flg = false;
	attack_start = false;
#endif // CHAIN_DEBUG

	frame_time  = 0.0f;
	frame_scope = 0.5f;
	frame_alpha = 0.0f;
	player_chain_activity = &Player::chain_search_update;
}

void Player::chain_search_update(float elapsed_time, std::vector<BaseEnemy*> enemies,
	GraphicsPipeline& graphics_)
{
#ifdef USE_IMGUI
	ImGui::Begin("chain");
	ImGui::Text("search");
#ifdef CHAIN_DEBUG
	ImGui::Text("search_time:%f", search_time);
	if (ImGui::Button("lockon start")) { debug_lockon = true; }
	if (ImGui::Button("transition lockon")) { debug_transition_chain_lockon_flg = true; }
	if (ImGui::Button("reset"))
	{
		position = {};
		if (!lockon_suggests.empty()) lockon_suggests.clear();
		search_time = SEARCH_TIME;
	}
#endif // CHAIN_DEBUG
	ImGui::End();
#endif // USE_IMGUI

#ifdef CHAIN_DEBUG

	if(debug_lockon) // 決められた時間内に敵を索敵しロックオンステートへ
	{
		bool is_stun = false;
		// スタンされた敵がいなければ通常行動に戻る
		for (const auto& enemy : enemies)
		{
			if (enemy->fGetStun())
			{
				is_stun = true;
				break;
			}
		}
		if (!is_stun) { transition_chain_search(); /*リセット*/ transition_normal_behavior(); }
		else // 決められた時間内に敵を索敵しロックオンステートへ
		{
			search_time -= elapsed_time;
			if (search_time > 0)
			{
				for (int i = 0; i < enemies.size(); ++i)
				{
					bool registered = false;
					for (auto index : chain_lockon_enemy_indexes)
					{
						if (index == i) // 一度登録したインデックスは登録しない
						{
							registered = true;
							break;
						}
					}
					if (!registered && enemies.at(i)->fGetStun() && enemies.at(i)->fComputeAndGetIntoCamera()) // 索敵時間内に一度でも視錐台に映ればロックオン
					{
						chain_lockon_enemy_indexes.emplace_back(i); // 登録
						LockOnSuggest enemy_suggest; // サジェスト登録
						enemy_suggest.position = enemies.at(i)->fGetPosition();
						lockon_suggests.emplace_back(enemy_suggest);

						enemies.at(i)->fSetIsLockOnOfChain(true);
					}
				}
			}
			else
			{
				transition_chain_lockon_begin();
			}
		}
	}
#else

	if (is_awakening) // 覚醒状態
	{
		if (!setup_search_time)
		{
			SEARCH_TIME = 0.5f;
			search_time = SEARCH_TIME;
			setup_search_time = true;
		}

		// 敵がいなければ通常行動に戻る
		if (enemies.size() == 0)
		{
			for (const auto& enemy : enemies)
			{
				if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
			}
			chain_parm_reset();
		}
		// スタンしてなくてもロックオン
		// 決められた時間内に敵を索敵しロックオンステートへ
		else
		{
			search_time -= elapsed_time;
			if (search_time > 0)
			{
				/*キャンセルがあれがここへ*/
				if (game_pad->get_button_up() & GamePad::BTN_LEFT_SHOULDER)
				{
					for (const auto& enemy : enemies)
					{
						if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
					}
					chain_parm_reset();
				}
				else
				{
					for (int i = 0; i < enemies.size(); ++i)
					{
						if (enemies.at(i)->fIsLockOnOfChain()) continue;
						if (enemies.at(i)->fGetPosition().y > 5.0f) continue;
						if (enemies.at(i)->fGetInnerCamera()) continue;

						if (enemies.at(i)->fComputeAndGetIntoCamera()) // 索敵時間内に一度でも視錐台に映ればロックオン(スタン関係なし)
						{
							chain_lockon_enemy_indexes.emplace_back(i); // 登録
							LockOnSuggest enemy_suggest; // サジェスト登録
							enemy_suggest.position = enemies.at(i)->fGetPosition();
							lockon_suggests.emplace_back(enemy_suggest);

							enemies.at(i)->fSetIsLockOnOfChain(true);
							// reticle生成
							reticles.insert(std::make_pair(std::make_unique<Reticle>(graphics_), enemies.at(i)));

							audio_manager->play_se(SE_INDEX::ROCK_ON);
						}
					}
					// reticleの更新
					for (const auto& reticle : reticles)
					{
						if (reticle.first == nullptr || reticle.second == nullptr) continue;
						reticle.first->update(graphics_, elapsed_time, 1.0f / SEARCH_TIME);
						reticle.first->focus(reticle.second, true);
					}
				}
			}
			else
			{
				if (chain_lockon_enemy_indexes.empty()) /*カメラにスタンした敵が一体も映らなかった*/
				{
					for (const auto& enemy : enemies)
					{
						if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
					}
					chain_parm_reset();
				}
				else { transition_chain_lockon_begin(); }
			}
		}
	}
	else // 非覚醒状態
	{
		bool is_stun = false;
		int stun_enemy_count = 0;
		// スタンされた敵がいなければ通常行動に戻る
		// スタンした敵に応じてサーチ時間を増やす
		for (const auto& enemy : enemies)
		{
			if (enemy->fGetStun())
			{
				is_stun = true;
				++stun_enemy_count;
			}
		}

		if (is_stun && !setup_search_time) // スタンした敵がいる時だけ計算
		{
			const int MAX_FLUCTUATION_COUNT = 6;
			const float MAX_SEARCH_TIME = 1.5f;
			const float MIN_SEARCH_TIME = 0.5f;

			stun_enemy_count = (std::min)(stun_enemy_count, MAX_FLUCTUATION_COUNT);
			// stun_enemy_count が 1 の時に割合0にしたいので-1
			SEARCH_TIME += (MAX_SEARCH_TIME - MIN_SEARCH_TIME) * ((float)(stun_enemy_count - 1) / (float)(MAX_FLUCTUATION_COUNT - 1));
			search_time = SEARCH_TIME;

			setup_search_time = true;
		}

		if (!is_stun)
		{
			for (const auto& enemy : enemies)
			{
				if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
			}
			chain_parm_reset();
		}
		else // 決められた時間内に敵を索敵しロックオンステートへ
		{
			search_time -= elapsed_time;
			if (search_time > 0)
			{
				/*キャンセルがあれがここへ*/
				if (game_pad->get_button_up() & GamePad::BTN_LEFT_SHOULDER)
				{
					for (const auto& enemy : enemies)
					{
						if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
					}
					chain_parm_reset();
				}
				else
				{
					for (int i = 0; i < enemies.size(); ++i)
					{
						if (enemies.at(i)->fIsLockOnOfChain()) continue;
						if (enemies.at(i)->fGetPosition().y > 5.0f) continue;
						if (!enemies.at(i)->fGetStun()) continue;
						if (enemies.at(i)->fGetInnerCamera()) continue;

						if (enemies.at(i)->fComputeAndGetIntoCamera()) // 索敵時間内に一度でも視錐台に映ればロックオン
						{
							chain_lockon_enemy_indexes.emplace_back(i); // 登録
							LockOnSuggest enemy_suggest; // サジェスト登録
							enemy_suggest.position = enemies.at(i)->fGetPosition();
							lockon_suggests.emplace_back(enemy_suggest);

							enemies.at(i)->fSetIsLockOnOfChain(true);
							// reticle生成
							reticles.insert(std::make_pair(std::make_unique<Reticle>(graphics_), enemies.at(i)));

							audio_manager->play_se(SE_INDEX::ROCK_ON);
						}
					}
					// reticleの更新
					for (const auto& reticle : reticles)
					{
						if (reticle.first == nullptr || reticle.second == nullptr) continue;
						reticle.first->update(graphics_, elapsed_time, 1.0f / SEARCH_TIME);
						reticle.first->focus(reticle.second, true);
					}
				}
			}
			else
			{
				if (chain_lockon_enemy_indexes.empty()) /*カメラに敵が一体も映らなかった*/
				{
					for (const auto& enemy : enemies)
					{
						if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
					}
					chain_parm_reset();
				}
				else { transition_chain_lockon_begin(); }
			}
		}
	}
#endif // CHAIN_DEBUG

	// 旋回処理
	RollTurn(position, orientation, elapsed_time);
}

void Player::transition_chain_lockon_begin()
{
	is_chain_attack = true;
	is_chain_attack_aftertaste = true;
	is_chain_attack_aftertaste_timer = 0;

	if (is_awakening) { model->play_animation(AwakingChargeInit, false, true, 0.1f, 3.0f); }
	else { model->play_animation(ChargeInit, false, true, 0.1f, 3.0f); }
	player_chain_activity = &Player::chain_lockon_begin_update;

	// velocityクリア
	velocity = {};

	audio_manager->play_se(SE_INDEX::PLAYER_AWAKING);

	if (GameFile::get_instance().get_vibration())game_pad->set_vibration(0.0f, 1.0f, 0.3f);

}

void Player::chain_lockon_begin_update(float elapsed_time, std::vector<BaseEnemy*> enemies,
	GraphicsPipeline& Graphics_)
{
#ifdef USE_IMGUI
	ImGui::Begin("chain"); ImGui::Text("lockon_begin"); ImGui::End();
#endif // USE_IMGUI

	if (model->end_of_animation())
	{
		transition_chain_lockon();
	}
}

void Player::transition_chain_lockon()
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
					champion_pos    = lockon_suggests.at(end).position;
					champion_index  = end;
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
				champion_pos    = {};
				champion_index  = -1;
				for (int end = 0; end < lockon_suggests.size(); ++end) // 終点側
				{
					float length = Math::calc_vector_AtoB_length(lockon_suggests.at(start).position, lockon_suggests.at(end).position);
					if (start != end && !lockon_suggests.at(end).detection && length > 0) // 検索条件：未検索かつ距離が0より大きい
					{
						if (champion_length > length)
						{
							champion_length = length;
							champion_pos    = lockon_suggests.at(end).position;
							champion_index  = end;
						}
					}
					//--------<!! ここで距離が0以下ならまったく同じ位置にポイントがある !!>--------//
					else if (start != end && !lockon_suggests.at(end).detection && length <= 0)
					{
						champion_pos   = lockon_suggests.at(end).position;
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
		DirectX::XMFLOAT3 end   = sort_points.at(index + 1);
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
	if (is_awakening) { model->play_animation(AwakingCharge, true); }
	else { model->play_animation(Charge, true); }
	player_chain_activity = &Player::chain_lockon_update;
}

void Player::chain_lockon_update(float elapsed_time, std::vector<BaseEnemy*> enemies,GraphicsPipeline& Graphics_)
{
#ifdef USE_IMGUI
	ImGui::Begin("chain"); ImGui::Text("lockon"); ImGui::End();
#endif // USE_IMGUI

	static float base_speed = 100.0f;
	static float base_play  = 1.0f;
#ifdef CHAIN_DEBUG
	static int elem = 0;
#ifdef USE_IMGUI
	ImGui::Begin("transit");
	ImGui::Text("position x : %.1f  y : %.1f  z : %.1f", position.x, position.y, position.z);
	ImGui::DragFloat("base_speed", &base_speed, 0.1f);
	ImGui::DragFloat("base_play", &base_play, 0.01f);
	if (ImGui::Button("start")) { attack_start = true; }
	if (ImGui::Button("restart")) { transit_index = 0; }
	ImGui::Separator();
	constexpr int count = 4;
	const char* elems_names[count] = { "suggests","sort_points","way_points","interpolated" };

	const char* elem_name = (elem >= 0 && elem < count) ? elems_names[elem] : "Unknown";
	ImGui::SliderInt("sphere type", &elem, 0, count - 1, elem_name);

	ImGui::End();
#endif  USE_IMGUI
	switch (elem)
	{
	case 0: // lockon_suggestsデバッグ表示
		for (int i = 0; i < lockon_suggests.size(); ++i)
		{
			debug_figure->create_sphere(lockon_suggests.at(i).position, 1.5f, { 1,0,0,1 });
		}
		break;
	case 1: // sort_pointsデバッグ表示
		for (int i = 0; i < sort_points.size(); ++i)
		{
			debug_figure->create_sphere(sort_points.at(i), 1.5f, { 0,0,1,1 });
		}
		break;
	case 2: // way_pointsデバッグ表示
		for (const auto& point : way_points)
		{
			debug_figure->create_sphere(point, 1.5f, { 0,0,1,1 });
		}
		break;
	case 3: // interpolated_way_pointsデバッグ表示
		for (const auto& point : interpolated_way_points)
		{
			debug_figure->create_sphere(point, 1.0f, { 1,1,1,1 });
		}
		break;
	}
#endif // CHAIN_DEBUG

	// speed増減
	const float MAX_LENGTH = 100.0f;
	float camera_to_player_length = Math::calc_vector_AtoB_length(camera_position, position);
	camera_to_player_length       = (std::min)(camera_to_player_length, MAX_LENGTH);
	float magnification_speed     = camera_to_player_length / MAX_LENGTH * 10.0f;
	float speed = magnification_speed * magnification_speed + base_speed;
	// play増減
	const float ADD_PLAY = 1.0f;
	float play = base_play + (camera_to_player_length / MAX_LENGTH) * ADD_PLAY;


#ifdef USE_IMGUI
	ImGui::Begin("magnification");
	ImGui::Text("length:%f", camera_to_player_length);
	ImGui::Text("magnification_speed:%f", magnification_speed * magnification_speed);
	ImGui::Text("speed:%f", speed);
	ImGui::Text("play:%f", play);
	ImGui::End();
#endif // USE_IMGUI

#ifdef CHAIN_DEBUG
	// 攻撃
	if (attack_start && transit(elapsed_time, transit_index, position, speed, interpolated_way_points, play))
	{
		assert(transit_index != 0 && "意図していない挙動になっています");
		transition_chain_attack(); // 攻撃ステートへ
	}
#else
	// 攻撃
	if (transit(elapsed_time, transit_index, position, speed, interpolated_way_points, play))
	{
		assert(transit_index != 0 && "意図していない挙動になっています");
		audio_manager->play_se(SE_INDEX::ATTACK_SWORD);
		audio_manager->play_se(SE_INDEX::AVOIDANCE);
		if (is_awakening) combo_count -= COMBO_COUNT_SUB;
		transition_chain_attack(); // 攻撃ステートへ
	}
#endif // CHAIN_DEBUG

	// reticleの更新
	for (const auto& reticle : reticles)
	{
		if (reticle.first == nullptr || reticle.second == nullptr) continue;
		reticle.first->update(Graphics_, elapsed_time, 1.0f / SEARCH_TIME);
		reticle.first->focus(reticle.second, true);
	}

	// 軌跡
	if (is_awakening)
	{
		mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
		mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
	}
	else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
}

void Player::transition_chain_move()
{

	player_chain_activity = &Player::chain_move_update;
}

void Player::chain_move_update(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
#ifdef USE_IMGUI
	ImGui::Begin("chain"); ImGui::Text("move"); ImGui::End();
#endif // USE_IMGUI

}

void Player::transition_chain_attack()
{
	switch (attack_type)
	{
	case ATTACK_TYPE::FIRST:
		if (is_awakening) { model->play_animation(AwakingAttackType1, false, true, 0.1f, 5.0f); }
		else { model->play_animation(AttackType1, false, true, 0.1f, 5.0f); }
		attack_type = ATTACK_TYPE::SECOND;
		break;

	case ATTACK_TYPE::SECOND:
		if (is_awakening) { model->play_animation(AwakingAttackType2, false, true, 0.1f, 5.0f); }
		else { model->play_animation(AttackType2, false, true, 0.1f, 5.0f); }
		attack_type = ATTACK_TYPE::THIRD;
		break;

	case ATTACK_TYPE::THIRD:
		if (is_awakening) { model->play_animation(AwakingAttackType3, false, true, 0.1f, 5.0f); }
		else { model->play_animation(AttackType3, false, true, 0.1f, 5.0f); }
		attack_type = ATTACK_TYPE::FIRST;
		break;
	}

	player_chain_activity = &Player::chain_attack_update;
}

void Player::chain_attack_update(float elapsed_time, std::vector<BaseEnemy*> enemies,GraphicsPipeline& Graphics_)
{
#ifdef USE_IMGUI
	ImGui::Begin("chain"); ImGui::Text("attack"); ImGui::End();
#endif // USE_IMGUI



	// 攻撃が終わった
	//if (model->end_of_animation())
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

			if (tutorial_state == TutorialState::ChainAttackTutorial)
			{
				tutorial_action_count--;
				if (tutorial_action_count <= 0)is_next_tutorial = true;
			}

			is_chain_attack = false;
			change_normal_timer = BaseCamera::AttackEndCameraTimer + AddAttackEndCameraTimer;

			transition_chain_search(); /* リセット */
			transition_normal_behavior();

			if (GameFile::get_instance().get_vibration())game_pad->set_vibration(1.0f, 0.0f, 0.3f);

		}
		else // ロックオンステートの初期化を通らず更新処理へ
		{
			player_chain_activity = &Player::chain_lockon_update;
			// 初期化通らないのでここでもアニメーション再生
			if (is_awakening) { model->play_animation(AwakingCharge, true); }
			else { model->play_animation(Charge, true); }
		}
	}
}