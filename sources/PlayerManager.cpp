#include "PlayerManager.h"

PlayerManager::PlayerManager()
{
}

PlayerManager::~PlayerManager()
{
}

void PlayerManager::Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    for (auto& player : players)
    {
        player->Update(elapsed_time, graphics, sky_dome, enemies);
    }
}

void PlayerManager::Render(GraphicsPipeline& graphics, float elapsed_time)
{
    for (auto& player : players)
    {
        player->Render(graphics, elapsed_time);
    }
}

void PlayerManager::RegisterPlayer(BasePlayer* player)
{
    //-----プレイヤーの所有権をplayersに移して登録する-----//
    players.emplace_back(std::move(player));
}

void PlayerManager::TransitionStageMove()
{
    for (auto& player : players)
    {

    }
}

void PlayerManager::TransitionStageMoveEnd()
{
}

bool PlayerManager::GetIsJustAvoidance()
{
    return false;
}

std::vector<DirectX::XMFLOAT3> PlayerManager::GetPosition()
{
    return std::vector<DirectX::XMFLOAT3>();
}
