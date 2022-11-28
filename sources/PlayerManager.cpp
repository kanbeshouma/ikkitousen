#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "PlayerManager.h"
#include"Correspondence.h"

PlayerManager::PlayerManager()
{
}

PlayerManager::~PlayerManager()
{
}

void PlayerManager::RestartInitialize()
{
    //-----体力を再設定-----//
    multiplay_current_health = multiplay_max_health;
    //-----プレイヤーを復活-----//
    for (auto& player : players)
    {
        player->RestartInitialize(multiplay_current_health);
    }
}

void PlayerManager::Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (CorrespondenceManager::Instance().GetMultiPlay())
    {
        //-----無敵時間の更新-----//
        invincible_timer -= 1.0f * elapsed_time;
        //-----HPのクランプ-----//
        multiplay_current_health = Math::clamp(multiplay_current_health, 0, multiplay_max_health);
    }
    for (auto& player : players)
    {
        player->SetHealth(multiplay_current_health);
        //-----もしオプション中なら自分の処理をとばす-----//
        if (player->GetObjectId() == private_object_id && option) continue;
        player->Update(elapsed_time, graphics, sky_dome, enemies);
    }
#ifdef USE_IMGUI
    ImGui::Begin("PlayerManager");
    ImGui::DragInt("multiplay_current_health",&multiplay_current_health);
    ImGui::End();
#endif // USE_IMGUI
}

void PlayerManager::PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
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
    //-----プレイヤーとの距離を計算-----//
    for (auto& player : players)
    {
        if (player->GetObjectId() == private_object_id)
        {
            for (auto& client : players)
            {
                if (client->GetObjectId() == private_object_id) continue;
                //-----距離を計算して保存-----//
                client->SetPlayerToClientLength(Math::calc_vector_AtoB_length(player->GetPosition(), client->GetPosition()));
            }
            break;
        }
    }


    for (auto& player : players)
    {
        player->Render(graphics, elapsed_time);
    }
}

void PlayerManager::RenderOperationPlayer(GraphicsPipeline& graphics, float elapsed_time)
{
    //-----自分以外は飛ばして描画する-----//
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
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

void PlayerManager::SetPlayerMoveData(PlayerMoveData data)
{
    for (auto& player : players)
    {
        //-----プレイヤーIDと受信データのIDが同じならデータ設定-----//
        if (player->GetObjectId() == data.player_id)
        {
            player->SetReceiveData(data);
        }
    }
}
void PlayerManager::SetPlayerPositionData(PlayerPositionData data)
{
    for (auto& player : players)
    {
        //-----プレイヤーIDと受信データのIDが同じならデータ設定-----//
        if (player->GetObjectId() == data.player_id)
        {
            player->SetReceivePositionData(data);
        }
    }

}

void PlayerManager::SetPlayerActionData(PlayerActionData data)
{
    for (auto& player : players)
    {
        //-----プレイヤーIDと受信データのIDが同じならデータ設定-----//
        if (player->GetObjectId() == data.player_id)
        {
            player->SetPlayerActionData(data);
        }
    }

}

void PlayerManager::SendPlayerHealthData()
{
    //-----マルチプレイのときはデータを送信する-----//
    if (CorrespondenceManager::Instance().GetMultiPlay())
    {
        PlayerHealthData d;
        d.data[ComLocation::ComList] = CommandList::Update;
        d.data[ComLocation::UpdateCom] = UpdateCommand::PlayerHealthCommand;
        //-----ダメージを送信する-----//
        d.data[PlayerHealthEnum::Damage] = 0;
        //-----今の体力を設定する-----//
        d.health = multiplay_current_health;

        //-----全員に送信する-----//
        CorrespondenceManager::Instance().UdpSend((char*)&d, sizeof(PlayerHealthData));
    }

}

void PlayerManager::ReceivePlayerHealthData(PlayerHealthData d)
{
    //-----ホストはダメージの値を使用して体力を減らす-----//
    if(CorrespondenceManager::Instance().GetHost()) multiplay_current_health -= d.data[PlayerHealthEnum::Damage];
    //-----クライアント側は体力の総量を使用して同期をとる-----//
    else multiplay_current_health = d.health;
}

void PlayerManager::ReceiveLockOnChain(int id, std::vector<char> enemy_id_data)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != id) continue;
        //-----プレイヤーIDと受信データのIDが同じならデータ設定-----//
        player->SetChainLockOnId(enemy_id_data);
    }

}

void PlayerManager::AddPlayerMultiHealth()
{
    //-----現在の体力と最大値を増やす-----//
    multiplay_current_health += OnePersonMultiHealth;
    multiplay_max_health += OnePersonMultiHealth;
}

void PlayerManager::SubPlayerMultiHealth()
{
    //-----現在の体力と最大値をへらす-----//
    multiplay_max_health -= OnePersonMultiHealth;

}

void PlayerManager::PermitChainAttack()
{
    for (auto& player : players)
    {
        //-----操作しないプレイヤーはとばす-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----許可フラグを出す-----//
        player->SetPermitChainAttack(true);
    }
}

void PlayerManager::ProhibitionChainAttack()
{
    for (auto& player : players)
    {
        //-----操作しないプレイヤーはとばす-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----許可フラグを出す-----//
        player->SetPermitChainAttack(false);
    }
}

void PlayerManager::SetDoChain(bool arg)
{
    for (auto& player : players)
    {
        //-----操作しないプレイヤーはとばす-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----許可フラグを出す-----//
        player->SetDoChain(arg);
    }
}

bool PlayerManager::GetReturnEnemyControl()
{
    bool return_flg{ false };
    for (auto& player : players)
    {
        //-----操作しないプレイヤーはとばす-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----敵のホスト権の返還フラグを取得-----//
        return_flg = player->GetRetrunEnemyControl();
    }
    return return_flg;
}

void PlayerManager::SetReturnEnemyControl(bool arg)
{
    for (auto& player : players)
    {
        //-----操作しないプレイヤーはとばす-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----敵のホスト権の返還フラグを初期化-----//
        player->SetReturnEnemyControl(false);
    }
}

void PlayerManager::DeletePlayer(int id)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() == id)
        {
            //-----オブジェクト番号と削除するプレイヤーのIDが同じなら削除変数に入れる-----//
            remove_players.emplace_back(player);
        }
    }

    //-----削除変数に入っているプレイヤーを削除する-----//
    for (auto& p : remove_players)
    {
        auto e = std::find(players.begin(), players.end(), p);
        if (e != players.end())
        {
            players.erase(e);
        }
    }
    remove_players.clear();
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

std::vector<std::tuple<int, DirectX::XMFLOAT3>> PlayerManager::GetPosition()
{
    //std::tuple<object_id,position>
    std::vector<std::tuple<int, DirectX::XMFLOAT3>> pos;

    for (auto& player : players)
    {
        //-----値を入れる-----//
        auto tuple = std::make_tuple(player->GetObjectId(), player->GetPosition());
        pos.emplace_back(tuple);
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
            //-----覚醒中は武器が二つあるから二つ分当たり判定をとる-----//
            int hit_count_1{ enemy_manager->fCalcPlayerAttackVsEnemies(player->GetSwordCapsuleParam(0).start,player->GetSwordCapsuleParam(0).end,player->GetSwordCapsuleParam(0).rasius,player->GetPlayerPower(),graphics,elapsed_time,block)};
            int hit_count_2{ enemy_manager->fCalcPlayerAttackVsEnemies(player->GetSwordCapsuleParam(1).start,player->GetSwordCapsuleParam(1).end,player->GetSwordCapsuleParam(1).rasius,player->GetPlayerPower(),graphics,elapsed_time,block)};

            player->AwakingAddCombo(hit_count_1,hit_count_2,block);
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
        //-----操作キャラクターじゃなかったらとばす-----//

        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
            //-----プレイヤーの体力をマルチ用のデータに設定(同期をとる)-----//
            player->SetHealth(multiplay_current_health);

            if (enemy_manager->fCalcEnemiesAttackVsPlayer(player->GetBodyCapsuleParam().start,
                player->GetBodyCapsuleParam().end,
                player->GetBodyCapsuleParam().rasius, player->GetDamagedFunc()))
            {
                //-----ホストが今の体力の総量を送信する-----//
                if (CorrespondenceManager::Instance().GetHost())
                {
                    multiplay_current_health = player->GetHealth();
                    //=====プレイヤーの体力のデータを送信する=====//
                    SendPlayerHealthData();
                }
            }
        }
        else
        {
            //-----シングルプレイの時のダメージ処理-----//
            enemy_manager->fCalcEnemiesAttackVsPlayer(player->GetBodyCapsuleParam().start,
                player->GetBodyCapsuleParam().end,
                player->GetBodyCapsuleParam().rasius, player->GetDamagedFunc());
        }
    }


}

void PlayerManager::SearchClientPlayerLockOnEnemy(EnemyManager* enemy_manager)
{
    for (auto& player : players)
    {
        //-----自分(操作しているプレイヤー)ならとばす-----//
        if (player->GetObjectId() == private_object_id) continue;
        for (const auto enemy : enemy_manager->fGetEnemies())
        {
            //-----同じ番号でなかったらとばす-----//
            if (player->GetLockPnEnemyId() != enemy->fGetObjectId()) continue;
            player->SetTarget(enemy);
            break;
        }
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

void PlayerManager::SetCameraView(const DirectX::XMFLOAT4X4 view)
{
    for (auto& player : players)
    {
        player->SetCameraView(view);
    }

}

void PlayerManager::SetCameraProjection(const DirectX::XMFLOAT4X4 projection)
{
    for (auto& player : players)
    {
        player->SetCameraProjection(projection);
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

bool PlayerManager::GetDoChainAttack()
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        if (player->GetBehaviorState() == BasePlayer::Behavior::Chain) return true;
        else return false;
    }
    return false;
}
