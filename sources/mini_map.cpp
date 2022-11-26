#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include "mini_map.h"
#include "Operators.h"
#include "user.h"

MiniMap::MiniMap(GraphicsPipeline& graphics)
{
	//�X�v���C�g�̎��̐���
	mini_map_icon = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\minimap\\minimap.png",1);
	player_icon = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\minimap\\minimap_player.png",1);

	//�p�����[�^�[�����l�ݒ�
	//�~�j�}�b�v���n
	minimap_icon_param =
	{ {1018,7},{1,1} };
	//�v���C���[
	player_icon_param =
	{ {128,128},{1,1} };
	//�G���G
	enemy_icon_param =
	{ {128,128},{1,1} };
	//�{�X
	boss_icon_param =
	{ {128,128},{1,1} };
	//���X�{�X
	last_boss_icon_param =
	{ {128,128},{1,1} };

}
void MiniMap::render(GraphicsPipeline& graphics,const DirectX::XMFLOAT2& player_pos, const DirectX::XMFLOAT2& player_forward,const DirectX::XMFLOAT2& camera_forward, std::vector<BaseEnemy*> enemy_list)
{
	//���[�_�[��������

	DirectX::XMFLOAT2 center_pos = { 1147.0f,136.0f };//�A�C�R���̊�ʒu
	mini_map_icon->begin(graphics.get_dc().Get());
	mini_map_icon->render(graphics.get_dc().Get(), {  minimap_icon_param.position.x, minimap_icon_param.position.y }, { minimap_icon_param.scale });
	mini_map_icon->end(graphics.get_dc().Get());

	player_icon_param.position = { center_pos.x,center_pos.y   };

#if 0
#if USE_IMGUI
	ImGui::Begin("minimapp");
	ImGui::DragFloat2("minimap", &player_icon_param.position.x);
	ImGui::End();
#endif

#endif // 0
	{
		//�v���C���[�̃A�C�R���̌�������]�i�J�����̑O�𐳖ʂƂ���j
		DirectX::XMVECTOR Dot = DirectX::XMVector2Dot(XMLoadFloat2(&player_forward), XMLoadFloat2(&camera_forward));
		float dot = DirectX::XMVectorGetX(Dot);
		dot = acosf(dot);

		const float cross{ (player_forward.x * camera_forward.y) - (player_forward.y * camera_forward.x) };
		dot = cross < 0 ? -dot : dot;

		//�v���C���[�A�C�R��
		player_icon->begin(graphics.get_dc().Get());
		player_icon->render(graphics.get_dc().Get(), { player_icon_param.position }, { player_icon_param.scale },
			{ 16,16 }, { 1,1,1,1 }, { DirectX::XMConvertToDegrees(dot) });
		player_icon->end(graphics.get_dc().Get());
	}
	DirectX::XMVECTOR P_Normal = DirectX::XMVector2Normalize(DirectX::XMLoadFloat2(&camera_forward));
	//�G�l�~�[�A�C�R��
	for(auto& enemy : enemy_list)
	{
		if(enemy->mpIcon==nullptr)
		{
		    continue;
		}

		const DirectX::XMFLOAT2 e_pos = { enemy->fGetPosition().x,enemy->fGetPosition().z };
		//�v���C���[����G�ւ̃x�N�g��
		const DirectX::XMVECTOR Normal_P_To_E_Vec = Math::calc_vector_AtoB_normalize(player_pos, e_pos);
		//�v���C���[�̐��ʃx�N�g���ƓG�܂ł̃x�N�g���Ƃ̓��ςŊԂ̊p���o��
		 DirectX::XMVECTOR Dot = DirectX::XMVector2Dot(P_Normal, Normal_P_To_E_Vec);
		float dot = DirectX::XMVectorGetX(Dot);
		dot = acosf(dot);
		//���K��
		DirectX::XMFLOAT2 normal_p_to_e_vec{};
		DirectX::XMStoreFloat2(&normal_p_to_e_vec, Normal_P_To_E_Vec);
		DirectX::XMFLOAT2 enemy_icon_pos;
		 float cross{ (normal_p_to_e_vec.x * camera_forward.y) - (normal_p_to_e_vec.y * camera_forward.x) };
		//��]�������w��
		dot = cross < 0 ? -dot : dot;

		const float length_p_to_e_vec = Math::calc_vector_AtoB_length(player_pos, e_pos);
		enemy_icon_pos.x =  center_pos.x + length_p_to_e_vec /2 * sinf(dot);
		enemy_icon_pos.y =  center_pos.y - length_p_to_e_vec /2 * cosf(dot);
		enemy_icon_param.position = enemy_icon_pos;

		if(length_p_to_e_vec < 205.0f)
		{
			float angle = 0.0f;
			if(enemy->fGetIsBoss())
			{
				DirectX::XMFLOAT2 boss_front = { enemy->fGetForward().x,enemy->fGetForward().z };
				DirectX::XMVECTOR Boss_Dot = DirectX::XMVector2Dot(XMLoadFloat2(&boss_front), XMLoadFloat2(&camera_forward));
				float boss_dot = DirectX::XMVectorGetX(Boss_Dot);
				boss_dot = acosf(boss_dot);

				const float cross{ (boss_front.x * camera_forward.y) - (boss_front.y * camera_forward.x) };
				boss_dot = cross < 0 ? -boss_dot : boss_dot;
				angle = DirectX::XMConvertToDegrees(boss_dot);
			}

			enemy->mpIcon->begin(graphics.get_dc().Get());
			if(enemy->fGetStun()) // �X�^�����Ă�G�̃A�C�R���̐F�ς���
			{
				enemy->mpIcon->render(graphics.get_dc().Get(), { enemy_icon_param.position }, { enemy_icon_param.scale }, { 32,32 }, { 1,1,0,1 }, angle);
			}
			else
			{
				enemy->mpIcon->render(graphics.get_dc().Get(), { enemy_icon_param.position }, { enemy_icon_param.scale }, { 32,32 }, { 1,1,1,1 }, angle);
			}
			enemy->mpIcon->end(graphics.get_dc().Get());

		}
	}
}
