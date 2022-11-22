#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include <assert.h>
#include <memory>
#include "SwordTrail.h"
#include "Player.h"
#include"Correspondence.h"

#include "BaseCamera.h"

bool Player::transit(float elapsed_time, int& index, DirectX::XMFLOAT3& position, float speed,
	const std::vector<DirectX::XMFLOAT3>& points, float play)
{
	assert(!points.empty() && "�|�C���g�̃T�C�Y��0�ł�");
	if (index >= points.size() - 1)
	{
		// ���S�ɏd�˂�ƃo�O�邩���Ȃ̂ł��������炷
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
		// ���S�ɏd�˂�ƃo�O�邩���Ȃ̂ł��������炷
		position.x = points.at(index).x + 0.05f;
		position.y = 0;
		position.z = points.at(index).z + 0.05f;

		if (index != 0 && index % (STEPS * 2) == 0) // �G�̃|�C���g�ɂ���
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
	//�^�[�Q�b�g�Ɍ������ĉ�]
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
				q = XMQuaternionRotationAxis(up, angle);//���̕����ɓ����N�I�[�^�j�I��
			}
			else
			{
				q = XMQuaternionRotationAxis(up, -angle);//���̕����ɓ����N�I�[�^�j�I��
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
	//-----�G�̃z�X�g����Ԋ҂���-----//
	return_enemy_control = true;
	permit_chain_attack = false;
	chain_cancel = true;
	transition_chain_search(); /*���Z�b�g*/
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

	// ���߂̗v�f�ɂ̓v���C���[������
	LockOnSuggest player_suggest;
	player_suggest.position = position;
	player_suggest.detection = true; // �v���C���[�͌����ɂ͓���Ȃ��̂ŏ��߂���true
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

	SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());

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

	if(debug_lockon) // ���߂�ꂽ���ԓ��ɓG�����G�����b�N�I���X�e�[�g��
	{
		bool is_stun = false;
		// �X�^�����ꂽ�G�����Ȃ���Βʏ�s���ɖ߂�
		for (const auto& enemy : enemies)
		{
			if (enemy->fGetStun())
			{
				is_stun = true;
				break;
			}
		}
		if (!is_stun) { transition_chain_search(); /*���Z�b�g*/ transition_normal_behavior(); }
		else // ���߂�ꂽ���ԓ��ɓG�����G�����b�N�I���X�e�[�g��
		{
			search_time -= elapsed_time;
			if (search_time > 0)
			{
				for (int i = 0; i < enemies.size(); ++i)
				{
					bool registered = false;
					for (auto index : chain_lockon_enemy_indexes)
					{
						if (index == i) // ��x�o�^�����C���f�b�N�X�͓o�^���Ȃ�
						{
							registered = true;
							break;
						}
					}
					if (!registered && enemies.at(i)->fGetStun() && enemies.at(i)->fComputeAndGetIntoCamera()) // ���G���ԓ��Ɉ�x�ł�������ɉf��΃��b�N�I��
					{
						chain_lockon_enemy_indexes.emplace_back(i); // �o�^
						LockOnSuggest enemy_suggest; // �T�W�F�X�g�o�^
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

	if (is_awakening) // �o�����
	{
		if (!setup_search_time)
		{
			SEARCH_TIME = 0.5f;
			search_time = SEARCH_TIME;
			setup_search_time = true;
		}

		// �G�����Ȃ���Βʏ�s���ɖ߂�
		if (enemies.size() == 0)
		{
			SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());
			for (const auto& enemy : enemies)
			{
				if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
			}
			chain_parm_reset();
		}
		// �X�^�����ĂȂ��Ă����b�N�I��
		// ���߂�ꂽ���ԓ��ɓG�����G�����b�N�I���X�e�[�g��
		else
		{
			search_time -= elapsed_time;
			if (search_time > 0)
			{
				/*�L�����Z�������ꂪ������*/
				if (game_pad->get_button_up() & GamePad::BTN_LEFT_SHOULDER)
				{
					SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());

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

						if (enemies.at(i)->fComputeAndGetIntoCamera()) // ���G���ԓ��Ɉ�x�ł�������ɉf��΃��b�N�I��(�X�^���֌W�Ȃ�)
						{
							chain_lockon_enemy_indexes.emplace_back(i); // �o�^
							LockOnSuggest enemy_suggest; // �T�W�F�X�g�o�^
							enemy_suggest.position = enemies.at(i)->fGetPosition();
							lockon_suggests.emplace_back(enemy_suggest);

							enemies.at(i)->fSetIsLockOnOfChain(true);
							//enemies.at(i)->fSetStun(true);
							// reticle����
							reticles.insert(std::make_pair(std::make_unique<Reticle>(graphics_), enemies.at(i)));

							audio_manager->play_se(SE_INDEX::ROCK_ON);
						}
					}
					// reticle�̍X�V
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
				if (chain_lockon_enemy_indexes.empty()) /*�J�����ɃX�^�������G����̂��f��Ȃ�����*/
				{
					SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());

					for (const auto& enemy : enemies)
					{
						if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
					}
					chain_parm_reset();
				}
				else
				{
					//-----�}���`�v���C�����z�X�g����Ȃ��Ƃ��ɂ����ɓ���-----//
					//�}���`�v���C����Ȃ����͂��̂܂܃`�F�C���U���ɓ���B�}���`�v���C���ł��z�X�g�̎��̓`�F�C���U���ɂ��̂܂ܓ���-----//
					if (CorrespondenceManager::Instance().GetMultiPlay() && CorrespondenceManager::Instance().GetHost() == false)
					{
						//-----�G�̃z�X�g���̏��n������������J��-----//
						if (permit_chain_attack)transition_chain_lockon_begin();
						//-----�������炦�Ȃ�������I��-----//
						else
						{
							SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());

							for (const auto& enemy : enemies)
							{
								if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
							}
							chain_parm_reset();
						}
					}
					else transition_chain_lockon_begin();
				}
			}
		}
	}
	else // ��o�����
	{
		bool is_stun = false;
		int stun_enemy_count = 0;
		// �X�^�����ꂽ�G�����Ȃ���Βʏ�s���ɖ߂�
		// �X�^�������G�ɉ����ăT�[�`���Ԃ𑝂₷
		for (const auto& enemy : enemies)
		{
			if (enemy->fGetStun())
			{
				is_stun = true;
				++stun_enemy_count;
			}
		}

		if (is_stun && !setup_search_time) // �X�^�������G�����鎞�����v�Z
		{
			const int MAX_FLUCTUATION_COUNT = 6;
			const float MAX_SEARCH_TIME = 1.5f;
			const float MIN_SEARCH_TIME = 0.5f;

			stun_enemy_count = (std::min)(stun_enemy_count, MAX_FLUCTUATION_COUNT);
			// stun_enemy_count �� 1 �̎��Ɋ���0�ɂ������̂�-1
			SEARCH_TIME += (MAX_SEARCH_TIME - MIN_SEARCH_TIME) * ((float)(stun_enemy_count - 1) / (float)(MAX_FLUCTUATION_COUNT - 1));
			search_time = SEARCH_TIME;

			setup_search_time = true;
		}

		if (!is_stun)
		{
			SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());

			for (const auto& enemy : enemies)
			{
				if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
			}
			chain_parm_reset();
		}
		else // ���߂�ꂽ���ԓ��ɓG�����G�����b�N�I���X�e�[�g��
		{
			search_time -= elapsed_time;
			if (search_time > 0)
			{
				/*�L�����Z�������ꂪ������*/
				if (game_pad->get_button_up() & GamePad::BTN_LEFT_SHOULDER)
				{
					SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());
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
						if (enemies.at(i)->fGetStun() == false) continue;
						if (enemies.at(i)->fGetInnerCamera()) continue;

						if (enemies.at(i)->fComputeAndGetIntoCamera()) // ���G���ԓ��Ɉ�x�ł�������ɉf��΃��b�N�I��
						{
							chain_lockon_enemy_indexes.emplace_back(i); // �o�^
							LockOnSuggest enemy_suggest; // �T�W�F�X�g�o�^
							enemy_suggest.position = enemies.at(i)->fGetPosition();
							lockon_suggests.emplace_back(enemy_suggest);

							enemies.at(i)->fSetIsLockOnOfChain(true);
							// reticle����
							reticles.insert(std::make_pair(std::make_unique<Reticle>(graphics_), enemies.at(i)));

							audio_manager->play_se(SE_INDEX::ROCK_ON);
						}
					}
					// reticle�̍X�V
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
				if (chain_lockon_enemy_indexes.empty()) /*�J�����ɓG����̂��f��Ȃ�����*/
				{
					SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());

					for (const auto& enemy : enemies)
					{
						if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
					}
					chain_parm_reset();
				}
				else
				{
					//-----�}���`�v���C�����z�X�g����Ȃ��Ƃ��ɂ����ɓ���-----//
					//�}���`�v���C����Ȃ����͂��̂܂܃`�F�C���U���ɓ���B�}���`�v���C���ł��z�X�g�̎��̓`�F�C���U���ɂ��̂܂ܓ���-----//
					if (CorrespondenceManager::Instance().GetMultiPlay() && CorrespondenceManager::Instance().GetHost() == false)
					{
						//-----�G�̃z�X�g���̏��n������������J��-----//
						if (permit_chain_attack)transition_chain_lockon_begin();
						//-----�������炦�Ȃ�������I��-----//
						else
						{
							SendPlayerActionData(GamePad::BTN_LEFT_SHOULDER, GetInputMoveVec());

							for (const auto& enemy : enemies)
							{
								if (enemy->fIsLockOnOfChain()) { enemy->fSetIsLockOnOfChain(false); }
							}
							chain_parm_reset();

						}
					}
					else transition_chain_lockon_begin();
				}
			}
		}
	}
#endif // CHAIN_DEBUG
}

void Player::transition_chain_lockon_begin()
{
	is_chain_attack = true;
	is_chain_attack_aftertaste = true;
	is_chain_attack_aftertaste_timer = 0;

	if (is_awakening) { model->play_animation(AwakingChargeInit, false, true, 0.1f, 3.0f); }
	else { model->play_animation(ChargeInit, false, true, 0.1f, 3.0f); }
	player_chain_activity = &Player::chain_lockon_begin_update;

	// velocity�N���A
	velocity = {};

	audio_manager->play_se(SE_INDEX::PLAYER_AWAKING);

	//-----�����̕ϐ���������-----//
	permit_chain_attack = false;

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

	sort_points.emplace_back(position); // �\�[�g���ꂽ�ʒu�̍ŏ��̓v���C���[�̈ʒu

	// �v���C���[�ƃ��b�N�I�����ꂽ�G���v���C���[�Ǝn�_�Ƃ��Ċ�����ȏ�̒��ň�ԋ߂��G�����̗v�f�ɓ���A���̓G�����̎n�_�Ƃ��ē��������Ń\�[�g����
	const float REFERENCE_DISTANCE = 10.0f; // �����
	for (int start = 0; start < lockon_suggests.size() - 1; ++start) // �n�_��
	{
		float champion_length{ FLT_MAX }; // ������ȏ�ōł��n�_�ɋ߂�����
		DirectX::XMFLOAT3 champion_pos{}; // ������ȏ�ōł��n�_�ɋ߂��ʒu
		int champion_index = -1;          // ������ȏ�ōł��n�_�ɋ߂��I�_���̃C���f�b�N�X
		for (int end = 0; end < lockon_suggests.size(); ++end) // �I�_��
		{
			float length = Math::calc_vector_AtoB_length(lockon_suggests.at(start).position, lockon_suggests.at(end).position);
			if (start != end && !lockon_suggests.at(end).detection && length >= REFERENCE_DISTANCE) // ���������F��������������ȏ�
			{
				if (champion_length > length)
				{
					champion_length = length;
					champion_pos    = lockon_suggests.at(end).position;
					champion_index  = end;
				}
			}
		}

		//-----< �n�_��start����I�_��end�̑�������I�� >-----//

		// ������ȏ�ōł��n�_�ɋ߂����������������̂œo�^
		if (champion_index > -1)
		{
			lockon_suggests.at(champion_index).detection = true; // �����ς�
			sort_points.emplace_back(champion_pos);
		}
		else // ������ȏ�ōł��n�_�ɋ߂�������������Ȃ�����
			 // �p�^�[���P�F�I�_�����S�Ċ�����ȓ�������(���������܂�����)���������0���傫��(�n�_�������������������΍�)���čČ���(�����ς݂͂͂���)
		 	 // �p�^�[���Q�F�I�_���̖��������Ȃ��������n�_����o�^����
		{
			bool all_searching = true; // �I�_�����S�Č�������Ă��邩�ǂ���
			for (int end = 0; end < lockon_suggests.size(); ++end) // �I�_��
			{
				if (start != end && !lockon_suggests.at(end).detection) // �I�_��( start != end )��������
				{
					all_searching = false; // ��ł�������������΃p�^�[��1�m��
					break;
				}
			}
			// �p�^�[��1
			if (!all_searching) // �������0���傫�����čČ���  ���������F��������������0���傫��
			{
				// ���Z�b�g
				champion_length = { FLT_MAX };
				champion_pos    = {};
				champion_index  = -1;
				for (int end = 0; end < lockon_suggests.size(); ++end) // �I�_��
				{
					float length = Math::calc_vector_AtoB_length(lockon_suggests.at(start).position, lockon_suggests.at(end).position);
					if (start != end && !lockon_suggests.at(end).detection && length > 0) // ���������F��������������0���傫��
					{
						if (champion_length > length)
						{
							champion_length = length;
							champion_pos    = lockon_suggests.at(end).position;
							champion_index  = end;
						}
					}
					//--------<!! �����ŋ�����0�ȉ��Ȃ�܂����������ʒu�Ƀ|�C���g������ !!>--------//
					else if (start != end && !lockon_suggests.at(end).detection && length <= 0)
					{
						champion_pos   = lockon_suggests.at(end).position;
						champion_index = end;
					}
				}
				assert(champion_index != -1 && "�p�^�[��1��champion_index���������Ă��܂���");
				assert(champion_index != start && "�p�^�[��1��champion_index���n�_���ɂȂ��Ă��܂�");
				lockon_suggests.at(champion_index).detection = true; // �����ς�
				sort_points.emplace_back(champion_pos); // �Č����I��
			}
			// �p�^�[��2
			else { sort_points.emplace_back(lockon_suggests.at(start).position); } // �n�_����o�^
		}
	}

	assert(lockon_suggests.size() == sort_points.size() && "�\�[�g�O�ƃ\�[�g��̗v�f�����قȂ��Ă��܂�");

	// �\�[�g���ꂽ�|�C���g���炻�ꂼ��̒��ԓ_���Z�o�A���̂��ׂĂ�ʂ�悤�ɔC�ӂ̕������ŕ��������|�C���g���Z�o
	way_points.emplace_back(sort_points.at(0)); // �v���C���[�̈ʒu
	for (int index = 0; index < sort_points.size() - 1; ++index)
	{
		DirectX::XMFLOAT3 start = sort_points.at(index);
		DirectX::XMFLOAT3 end   = sort_points.at(index + 1);
		// �E�����x�N�g���ƃ^�[�Q�b�g�ւ̃x�N�g���̊p�x���Z�o
		DirectX::XMVECTOR norm_vec = Math::calc_vector_AtoB_normalize(start, end);
		DirectX::XMFLOAT3 right = { 1,0,0 };
		DirectX::XMVECTOR right_vec = DirectX::XMLoadFloat3(&right);
		DirectX::XMVECTOR dot = DirectX::XMVector3Dot(norm_vec, right_vec);
		float cos_theta = DirectX::XMVectorGetX(dot);
		// �E�����x�N�g���ƃ^�[�Q�b�g�ւ̃x�N�g���̊p�x�ƔC�ӂ̊p�x�𑫂������̂����Ԓn�_�̃�
		const float theta = acosf(cos_theta) + DirectX::XMConvertToRadians(30.0f);
		// �^�[�Q�b�g�ւ̃x�N�g���̒������Z�o
		float length_vec = Math::calc_vector_AtoB_length(start, end);
		// ���Ԓn�_�̎Z�o
		DirectX::XMFLOAT3 midpoint;
		if (start.z <= end.z) { midpoint = { start.x + cosf(theta) * length_vec * 0.5f, start.y, start.z + sinf(theta) * length_vec * 0.5f }; }
		else { midpoint = { start.x + cosf(theta) * length_vec * 0.5f, start.y, start.z + -sinf(theta) * length_vec * 0.5f }; }
		// way_points�̐���
		way_points.emplace_back(midpoint); // �v���C���[�̈ʒu�ƓG�̈ʒu�̒��Ԓn�_
		way_points.emplace_back(end); // �G�̈ʒu
	}

	// way_points��ʂ�J�[�u���쐬
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
	case 0: // lockon_suggests�f�o�b�O�\��
		for (int i = 0; i < lockon_suggests.size(); ++i)
		{
			debug_figure->create_sphere(lockon_suggests.at(i).position, 1.5f, { 1,0,0,1 });
		}
		break;
	case 1: // sort_points�f�o�b�O�\��
		for (int i = 0; i < sort_points.size(); ++i)
		{
			debug_figure->create_sphere(sort_points.at(i), 1.5f, { 0,0,1,1 });
		}
		break;
	case 2: // way_points�f�o�b�O�\��
		for (const auto& point : way_points)
		{
			debug_figure->create_sphere(point, 1.5f, { 0,0,1,1 });
		}
		break;
	case 3: // interpolated_way_points�f�o�b�O�\��
		for (const auto& point : interpolated_way_points)
		{
			debug_figure->create_sphere(point, 1.0f, { 1,1,1,1 });
		}
		break;
	}
#endif // CHAIN_DEBUG

	// speed����
	const float MAX_LENGTH = 100.0f;
	float camera_to_player_length = Math::calc_vector_AtoB_length(camera_position, position);
	camera_to_player_length       = (std::min)(camera_to_player_length, MAX_LENGTH);
	float magnification_speed     = camera_to_player_length / MAX_LENGTH * 10.0f;
	float speed = magnification_speed * magnification_speed + base_speed;
	// play����
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
	// �U��
	if (attack_start && transit(elapsed_time, transit_index, position, speed, interpolated_way_points, play))
	{
		assert(transit_index != 0 && "�Ӑ}���Ă��Ȃ������ɂȂ��Ă��܂�");
		transition_chain_attack(); // �U���X�e�[�g��
	}
#else
	// �U��
	if (transit(elapsed_time, transit_index, position, speed, interpolated_way_points, play))
	{
		assert(transit_index != 0 && "�Ӑ}���Ă��Ȃ������ɂȂ��Ă��܂�");
		audio_manager->play_se(SE_INDEX::ATTACK_SWORD);
		audio_manager->play_se(SE_INDEX::AVOIDANCE);
		if (is_awakening) combo_count -= COMBO_COUNT_SUB;
		transition_chain_attack(); // �U���X�e�[�g��
	}
#endif // CHAIN_DEBUG

	// reticle�̍X�V
	for (const auto& reticle : reticles)
	{
		if (reticle.first == nullptr || reticle.second == nullptr) continue;
		reticle.first->update(Graphics_, elapsed_time, 1.0f / SEARCH_TIME);
		reticle.first->focus(reticle.second, true);
	}

	// �O��
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



	// �U�����I�����
	//if (model->end_of_animation())
	{
		// ���������|�C���g�̍Ō�Ȃ�ʏ�s����
		if (transit_index >= interpolated_way_points.size() - 1)
		{
			// ���b�N�I�����ꂽ�G�Ƀ_���[�W
			for (const auto& enemy : enemies)
			{
				if (!enemy->fIsLockOnOfChain()) continue; // �_���[�W��^����̂̓��b�N�I�����ꂽ�G�̂�
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

			transition_chain_search(); /* ���Z�b�g */
			transition_normal_behavior();

			if (GameFile::get_instance().get_vibration())game_pad->set_vibration(1.0f, 0.0f, 0.3f);

			if (CorrespondenceManager::Instance().GetMultiPlay() && CorrespondenceManager::Instance().GetHost() == false)
			{
				//-----�G�̃z�X�g����Ԋ҂���-----//
				return_enemy_control = true;
				//-----�`�F�C���U���̋��̕ϐ���������-----//
				permit_chain_attack = false;
			}

		}
		else // ���b�N�I���X�e�[�g�̏�������ʂ炸�X�V������
		{
			player_chain_activity = &Player::chain_lockon_update;
			// �������ʂ�Ȃ��̂ł����ł��A�j���[�V�����Đ�
			if (is_awakening) { model->play_animation(AwakingCharge, true); }
			else { model->play_animation(Charge, true); }
		}
	}
}