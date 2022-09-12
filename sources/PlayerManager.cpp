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

void PlayerManager::PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    for (auto& player : players)
    {
        player->PlayerClearUpdate(elapsed_time, graphics, sky_dome, enemies);
    }
}

void PlayerManager::ConfigRender(GraphicsPipeline& graphics, float elapsed_time)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->ConfigRender(graphics, elapsed_time);
        //-----処理が終わったらfor文をぬける-----//
        break;
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

void PlayerManager::SetPlayerPosition(DirectX::XMFLOAT3 pos)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetPosition(pos);
    }
}

DirectX::XMFLOAT3 PlayerManager::GetMyTerminalPosition()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetPosition();
    }
    return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 PlayerManager::GetMyTerminalForward()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetForward();
    }
    return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 PlayerManager::GetMyTerminalJoint()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetEnentCameraJoint();
    }
    return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 PlayerManager::GetMyTerminalEye()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetEnentCameraEye();
    }
    return DirectX::XMFLOAT3();
}

bool PlayerManager::GetMyTerminalEndClearMotion()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetEndClearMotion();
    }
    return false;
}

void PlayerManager::ChangePlayerJustificationLength()
{
    for (auto& player : players)
    {
        player->ChangePlayerJustificationLength();
    }
}

void PlayerManager::TransitionStageMove()
{
    for (auto& player : players)
    {
        player->TransitionStageMove();
    }
}

void PlayerManager::TransitionStageMoveEnd()
{
    for (auto& player : players)
    {
        player->TransitionStageMoveEnd();
    }
}

bool PlayerManager::GetIsJustAvoidance()
{
    for (auto& player : players)
    {
        //-----操作できるプレイヤーではなかったらとばす-----//
        if (player->GetObjectId() != private_object_id) continue;

        //プレイヤーがジャスト回避したらslow
        return player->GetIsJustAvoidance();
    }

    return false;
}

std::vector<DirectX::XMFLOAT3> PlayerManager::GetPosition()
{
    std::vector<DirectX::XMFLOAT3> pos;
    //-----vector型配列を今のプレイヤーの数分リサイズする-----//
    pos.resize(players.size());

    for (auto& player : players)
    {
        //-----プレイヤーのオブジェクト番号番目に値を入れる-----//
        pos.at(player->GetObjectId()) = player->GetPosition();
    }
    return pos;
}

bool PlayerManager::DuringSearchTime()
{
    for (auto& player : players)
    {
        //-----操作できるプレイヤーではなかったらとばす-----//
        if (player->GetObjectId() != private_object_id) continue;

        //プレイヤーがジャスト回避したらslow
        return player->during_search_time();
    }
    return false;
}

bool PlayerManager::GetIsAlive()
{
    for (auto& player : players)
    {
        //-----プレイヤーが一人でも生きていたらtrueを返す-----//
        if (player->GetIsAlive()) return true;
    }

    return false;
}

void PlayerManager::PlayerAttackVsEnemy(EnemyManager* enemy_manager, GraphicsPipeline& graphics, float elapsed_time)
{
    bool block = false;
    for (auto& player : players)
    {
        if (player->GetIsPlayerAttack() == false) continue;
        block = false;
        if (player->GetIsAwakening())
        {
            player->AwakingAddCombo
            (
                enemy_manager->fCalcPlayerAttackVsEnemies
                (
                    player->GetSwordCapsuleParam(0).start,
                    player->GetSwordCapsuleParam(0).end,
                    player->GetSwordCapsuleParam(0).rasius,
                    player->GetPlayerPower(),
                    graphics,
                    elapsed_time,
                    block
                ),
                enemy_manager->fCalcPlayerAttackVsEnemies
                (
                    player->GetSwordCapsuleParam(1).start,
                    player->GetSwordCapsuleParam(1).end,
                    player->GetSwordCapsuleParam(1).rasius,
                    player->GetPlayerPower(),
                    graphics,
                    elapsed_time,
                    block
                )
                , block
            );

        }
        else
        {
            //-----攻撃に当たった回数-----//
            int hit_count{ enemy_manager->fCalcPlayerAttackVsEnemies(
                player->GetSwordCapsuleParam(0).start,player->GetSwordCapsuleParam(0).end,player->GetSwordCapsuleParam(0).rasius,player->GetPlayerPower(),graphics,elapsed_time,block) };

            //-----結果を入れる-----//
            player->AddCombo(hit_count, block);
        }
    }
}

void PlayerManager::PlayerCounterVsEnemyAttack(EnemyManager* enemy_manager)
{
    for (auto& player : players)
    {
        player->PlayerJustAvoidance(enemy_manager->fCalcEnemiesAttackVsPlayerCounter(
            player->GetJustAvoidanceCapsuleParam().start,
            player->GetJustAvoidanceCapsuleParam().end,
            player->GetJustAvoidanceCapsuleParam().rasius));
    }
}

void PlayerManager::EnemyAttackVsPlayer(EnemyManager* enemy_manager)
{
    for (auto& player : players)
    {
        enemy_manager->fCalcEnemiesAttackVsPlayer(player->GetBodyCapsuleParam().start,
            player->GetBodyCapsuleParam().end,
            player->GetBodyCapsuleParam().rasius, player->GetDamagedFunc());
    }
}

void PlayerManager::BulletVsPlayer(BulletManager& bullet_manager)
{
    for (auto& player : players)
    {
        bullet_manager.fCalcBulletsVsPlayer(player->GetBodyCapsuleParam().start,
            player->GetBodyCapsuleParam().end,
            player->GetBodyCapsuleParam().rasius, player->GetDamagedFunc());
    }
}

void PlayerManager::PlayerStunVsEnemy(EnemyManager* enemy_manager)
{
    for (auto& player : players)
    {
        enemy_manager->fCalcPlayerStunVsEnemyBody(player->GetPosition(), player->GetStunRadius());
    }
}

void PlayerManager::SetPlayerChainTime(EnemyManager* enemy_manager)
{
    for (auto& player : players)
    {
        enemy_manager->fSetIsPlayerChainTime(player->during_chain_attack());
    }
}

void PlayerManager::SetBossCamera(bool flag)
{
    for (auto& player : players)
    {
        player->SetBossCamera(flag);
    }
}

void PlayerManager::SetCameraDirection(DirectX::XMFLOAT3 f, DirectX::XMFLOAT3 r)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetCameraDirection(f, r);
        //-----設定できたらfor文をぬける-----//
        break;
    }
}

void PlayerManager::LockOnPostEffect(float elapsed_time, PostEffect* post_efc)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->lockon_post_effect(elapsed_time, [=](float scope, float alpha) { post_efc->lockon_post_effect(scope, alpha); },
            [=]() { post_efc->clear_post_effect(); });
        //-----設定できたらfor文をぬける-----//
        break;
    }
}

void PlayerManager::SetCameraPosition(DirectX::XMFLOAT3 eye)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetCameraPosition(eye);
        //-----設定できたらfor文をぬける-----//
        break;
    }
}

void PlayerManager::SetTarget(BaseEnemy* enemy)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetTarget(enemy);
        //-----設定できたらfor文をぬける-----//
        break;
    }
}

void PlayerManager::SetCameraTarget(DirectX::XMFLOAT3 target)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetCameraTarget(target);
        //-----設定できたらfor文をぬける-----//
        break;
    }
}

void PlayerManager::DashPostEffect(GraphicsPipeline& graphics, PostEffect* post_efc)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        if (player->GetStartDashEffect()) post_efc->dash_post_effect(graphics.get_dc().Get(), player->GetPosition());
        //-----設定できたらfor文をぬける-----//
        break;
    }
}

BaseEnemy* PlayerManager::GetTargetEnemy()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetPlayerTargetEnemy();
    }

    return nullptr;
}

bool PlayerManager::GetEnemyLockOn()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetEnemyLockOn();
    }
    return false;
}

bool PlayerManager::GetBehaindCharge()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        return player->GetBehaindCharge();
    }
    return false;
}
