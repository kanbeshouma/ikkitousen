#pragma once
#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "EnemyManager.h"
class MiniMap
{
	struct Element
	{
		DirectX::XMFLOAT2 position{};
		DirectX::XMFLOAT2 scale{ 1, 1 };
	};
public:
	MiniMap(GraphicsPipeline& graphics);
	~MiniMap() {}

	void render(GraphicsPipeline& graphics, const DirectX::XMFLOAT2& player_pos, const DirectX::XMFLOAT2& player_forward, const DirectX::XMFLOAT2& camera_forward, std::vector<BaseEnemy*> enemy_list , std::vector<std::shared_ptr<BasePlayer>> player_list, int private_object_id);
	void render(GraphicsPipeline& graphics, const DirectX::XMFLOAT2& player_pos, const DirectX::XMFLOAT2& player_forward, const DirectX::XMFLOAT2& camera_forward, std::vector<BaseEnemy*> enemy_list);
	std::unique_ptr<SpriteBatch> mini_map_icon;
	std::unique_ptr<SpriteBatch> player_icon;
	Element minimap_icon_param;
	Element player_icon_param;
	Element enemy_icon_param;
	Element boss_icon_param;
	Element last_boss_icon_param;
};