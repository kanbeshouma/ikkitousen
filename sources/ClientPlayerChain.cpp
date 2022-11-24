#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

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
			//-----�`�F�C���U���̃��b�N�I������ID�łȂ���΂Ƃ΂�-----//
			if (enemies.at(i)->fGetObjectId() != id) continue;

			//-----�K�v�ȃf�[�^��ݒ�-----//
			chain_lockon_enemy_indexes.emplace_back(i); // �o�^
			LockOnSuggest enemy_suggest; // �T�W�F�X�g�o�^
			enemy_suggest.position = enemies.at(i)->fGetPosition();
			lockon_suggests.emplace_back(enemy_suggest);
			enemies.at(i)->fSetIsLockOnOfChain(true);
			DebugConsole::Instance().WriteDebugConsole("�f�[�^�ݒ�", TextColor::SkyBlue);
		}
	}

	DebugConsole::Instance().WriteDebugConsole("�`�F�C���U�����G����");
	TransitionChainLockonBegin();
}

void ClientPlayer::TransitionChainSearch()
{

	if (!chain_lockon_enemy_indexes.empty()) chain_lockon_enemy_indexes.clear();
	if (!lockon_suggests.empty()) lockon_suggests.clear();
	if (!reticles.empty()) { reticles.clear(); }

	// ���߂̗v�f�ɂ̓v���C���[������
	LockOnSuggest player_suggest;
	player_suggest.position = position;
	player_suggest.detection = true; // �v���C���[�͌����ɂ͓���Ȃ��̂ŏ��߂���true
	lockon_suggests.emplace_back(player_suggest);


	player_chain_activity = &ClientPlayer::ChainSearchUpdate;
}

void ClientPlayer::ChainLockonBeginUpdate(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
	if (model->end_of_animation(anim_parm))
	{
		DebugConsole::Instance().WriteDebugConsole("���b�N�I���J�n", TextColor::SkyBlue);
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

	// velocity�N���A
	velocity = {};

}

void ClientPlayer::ChainLockonUpdate(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
	static float base_speed = 100.0f;
	static float base_play = 1.0f;

	// speed����
	const float MAX_LENGTH = 100.0f;
	float camera_to_player_length = Math::calc_vector_AtoB_length(camera_position, position);
	camera_to_player_length = (std::min)(camera_to_player_length, MAX_LENGTH);
	float magnification_speed = camera_to_player_length / MAX_LENGTH * 10.0f;
	float speed = magnification_speed * magnification_speed + base_speed;
	// play����
	const float ADD_PLAY = 1.0f;
	float play = base_play + (camera_to_player_length / MAX_LENGTH) * ADD_PLAY;

	// �U��
	if (Transit(elapsed_time, transit_index, position, speed, interpolated_way_points, play))
	{
		assert(transit_index != 0 && "�Ӑ}���Ă��Ȃ������ɂȂ��Ă��܂�");
		audio_manager->play_se(SE_INDEX::ATTACK_SWORD);
		audio_manager->play_se(SE_INDEX::AVOIDANCE);
		if (is_awakening) combo_count -= COMBO_COUNT_SUB;
		TransitionChainAttack(); // �U���X�e�[�g��
	}

	// �O��
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
					champion_pos = lockon_suggests.at(end).position;
					champion_index = end;
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
				champion_pos = {};
				champion_index = -1;
				for (int end = 0; end < lockon_suggests.size(); ++end) // �I�_��
				{
					float length = Math::calc_vector_AtoB_length(lockon_suggests.at(start).position, lockon_suggests.at(end).position);
					if (start != end && !lockon_suggests.at(end).detection && length > 0) // ���������F��������������0���傫��
					{
						if (champion_length > length)
						{
							champion_length = length;
							champion_pos = lockon_suggests.at(end).position;
							champion_index = end;
						}
					}
					//--------<!! �����ŋ�����0�ȉ��Ȃ�܂����������ʒu�Ƀ|�C���g������ !!>--------//
					else if (start != end && !lockon_suggests.at(end).detection && length <= 0)
					{
						champion_pos = lockon_suggests.at(end).position;
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
		DirectX::XMFLOAT3 end = sort_points.at(index + 1);
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

		is_chain_attack = false;

		TransitionChainSearch(); /* ���Z�b�g */
		TransitionNormalBehavior();
	}
	else // ���b�N�I���X�e�[�g�̏�������ʂ炸�X�V������
	{
		player_chain_activity = &ClientPlayer::ChainLockonUpdate;
		// �������ʂ�Ȃ��̂ł����ł��A�j���[�V�����Đ�
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

	Rotate(elapsed_time, index, points);


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

void ClientPlayer::TransitionChainBehavior()
{
	behavior_state = Behavior::Chain;
	TransitionChainSearch();
}

void ClientPlayer::TransitionNormalBehavior()
{
	behavior_state = Behavior::Normal;
	//-----�`�F�C���U���̃f�[�^���폜-----//
	receive_chain_lock_on_enemy_id.clear();

	DebugConsole::Instance().WriteDebugConsole("�`�F�C���U���I��", TextColor::SkyBlue);
}

void ClientPlayer::ChainParmReset()
{
	chain_cancel = true;
	TransitionChainSearch(); /*���Z�b�g*/
	TransitionNormalBehavior();
	DebugConsole::Instance().WriteDebugConsole("�`�F�C���U���I��", TextColor::SkyBlue);
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
	//�ړ����͂��Ȃ��Ȃ�����ҋ@�ɑJ��
	if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
	{
		TransitionChainIdle();
	}
	UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, sky_dome);
}

void ClientPlayer::TransitionChainIdle(float blend_second)
{
	if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingIdle, true, true, blend_second);
	//�ʏ��Ԃ̑ҋ@�A�j���[�V�����ɃZ�b�g
	else model->play_animation(anim_parm, AnimationClips::Idle, true, true, blend_second);
	//�U�������ǂ����̐ݒ�
	is_attack = false;
	//�A�j���[�V�������x
	animation_speed = 1.0f;
	//�A�j���[�V���������Ă������ǂ���
	is_update_animation = true;
	//�ҋ@��Ԃ̎��̍X�V�֐��ɐ؂�ւ���
	chain_activity = &ClientPlayer::ChainIdleUpdate;

}

void ClientPlayer::TransitionChainMoveAnim(float blend_second)
{
	if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingMove, true, true, blend_second);
	//�ʏ��Ԃ̎��Ɉړ��A�j���[�V�����̐ݒ�
	else model->play_animation(anim_parm, AnimationClips::Move, true, true, blend_second);
	//�U�������ǂ����̐ݒ�
	is_attack = false;
	//�A�j���[�V�������x
	animation_speed = 1.0f;
	//�A�j���[�V���������Ă������ǂ���
	is_update_animation = true;
	//�ړ���Ԃ̎��̍X�V�֐��ɐ؂�ւ���
	chain_activity = &ClientPlayer::ChainMoveAnimUpdate;

}
