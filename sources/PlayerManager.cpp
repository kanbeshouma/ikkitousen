#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

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
    //-----�̗͂��Đݒ�-----//
    multiplay_current_health = multiplay_max_health;
    //-----�v���C���[�𕜊�-----//
    for (auto& player : players)
    {
        player->RestartInitialize(multiplay_current_health);
    }
}

void PlayerManager::Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (CorrespondenceManager::Instance().GetMultiPlay())
    {
        //-----���G���Ԃ̍X�V-----//
        invincible_timer -= 1.0f * elapsed_time;
        //-----HP�̃N�����v-----//
        multiplay_current_health = Math::clamp(multiplay_current_health, 0, multiplay_max_health);
    }
    for (auto& player : players)
    {
        player->SetHealth(multiplay_current_health);
        //-----�����I�v�V�������Ȃ玩���̏������Ƃ΂�-----//
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
        //-----�������I�������for�����ʂ���-----//
        break;
    }
}

void PlayerManager::Render(GraphicsPipeline& graphics, float elapsed_time)
{
    //-----�v���C���[�Ƃ̋������v�Z-----//
    for (auto& player : players)
    {
        if (player->GetObjectId() == private_object_id)
        {
            for (auto& client : players)
            {
                if (client->GetObjectId() == private_object_id) continue;
                //-----�������v�Z���ĕۑ�-----//
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
    //-----�����ȊO�͔�΂��ĕ`�悷��-----//
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->Render(graphics, elapsed_time);
    }
}

void PlayerManager::RegisterPlayer(BasePlayer* player)
{
    //-----�v���C���[�̏��L����players�Ɉڂ��ēo�^����-----//
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
        //-----�v���C���[ID�Ǝ�M�f�[�^��ID�������Ȃ�f�[�^�ݒ�-----//
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
        //-----�v���C���[ID�Ǝ�M�f�[�^��ID�������Ȃ�f�[�^�ݒ�-----//
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
        //-----�v���C���[ID�Ǝ�M�f�[�^��ID�������Ȃ�f�[�^�ݒ�-----//
        if (player->GetObjectId() == data.player_id)
        {
            player->SetPlayerActionData(data);
        }
    }

}

void PlayerManager::SendPlayerHealthData()
{
    //-----�}���`�v���C�̂Ƃ��̓f�[�^�𑗐M����-----//
    if (CorrespondenceManager::Instance().GetMultiPlay())
    {
        PlayerHealthData d;
        d.data[ComLocation::ComList] = CommandList::Update;
        d.data[ComLocation::UpdateCom] = UpdateCommand::PlayerHealthCommand;
        //-----�_���[�W�𑗐M����-----//
        d.data[PlayerHealthEnum::Damage] = 0;
        //-----���̗̑͂�ݒ肷��-----//
        d.health = multiplay_current_health;

        //-----�S���ɑ��M����-----//
        CorrespondenceManager::Instance().UdpSend((char*)&d, sizeof(PlayerHealthData));
    }

}

void PlayerManager::ReceivePlayerHealthData(PlayerHealthData d)
{
    //-----�z�X�g�̓_���[�W�̒l���g�p���đ̗͂����炷-----//
    if(CorrespondenceManager::Instance().GetHost()) multiplay_current_health -= d.data[PlayerHealthEnum::Damage];
    //-----�N���C�A���g���̗͑͂̑��ʂ��g�p���ē������Ƃ�-----//
    else multiplay_current_health = d.health;
}

void PlayerManager::ReceiveLockOnChain(int id, std::vector<char> enemy_id_data)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != id) continue;
        //-----�v���C���[ID�Ǝ�M�f�[�^��ID�������Ȃ�f�[�^�ݒ�-----//
        player->SetChainLockOnId(enemy_id_data);
    }

}

void PlayerManager::AddPlayerMultiHealth()
{
    //-----���݂̗̑͂ƍő�l�𑝂₷-----//
    multiplay_current_health += OnePersonMultiHealth;
    multiplay_max_health += OnePersonMultiHealth;
}

void PlayerManager::SubPlayerMultiHealth()
{
    //-----���݂̗̑͂ƍő�l���ւ炷-----//
    multiplay_max_health -= OnePersonMultiHealth;

}

void PlayerManager::PermitChainAttack()
{
    for (auto& player : players)
    {
        //-----���삵�Ȃ��v���C���[�͂Ƃ΂�-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----���t���O���o��-----//
        player->SetPermitChainAttack(true);
    }
}

void PlayerManager::ProhibitionChainAttack()
{
    for (auto& player : players)
    {
        //-----���삵�Ȃ��v���C���[�͂Ƃ΂�-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----���t���O���o��-----//
        player->SetPermitChainAttack(false);
    }
}

void PlayerManager::SetDoChain(bool arg)
{
    for (auto& player : players)
    {
        //-----���삵�Ȃ��v���C���[�͂Ƃ΂�-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----���t���O���o��-----//
        player->SetDoChain(arg);
    }
}

bool PlayerManager::GetReturnEnemyControl()
{
    bool return_flg{ false };
    for (auto& player : players)
    {
        //-----���삵�Ȃ��v���C���[�͂Ƃ΂�-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----�G�̃z�X�g���̕Ԋ҃t���O���擾-----//
        return_flg = player->GetRetrunEnemyControl();
    }
    return return_flg;
}

void PlayerManager::SetReturnEnemyControl(bool arg)
{
    for (auto& player : players)
    {
        //-----���삵�Ȃ��v���C���[�͂Ƃ΂�-----//
        if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
        //-----�G�̃z�X�g���̕Ԋ҃t���O��������-----//
        player->SetReturnEnemyControl(false);
    }
}

void PlayerManager::DeletePlayer(int id)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() == id)
        {
            //-----�I�u�W�F�N�g�ԍ��ƍ폜����v���C���[��ID�������Ȃ�폜�ϐ��ɓ����-----//
            remove_players.emplace_back(player);
        }
    }

    //-----�폜�ϐ��ɓ����Ă���v���C���[���폜����-----//
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
        //-----����ł���v���C���[�ł͂Ȃ�������Ƃ΂�-----//
        if (player->GetObjectId() != private_object_id) continue;

        //�v���C���[���W���X�g���������slow
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
        //-----�l������-----//
        auto tuple = std::make_tuple(player->GetObjectId(), player->GetPosition());
        pos.emplace_back(tuple);
    }
    return pos;
}

bool PlayerManager::DuringSearchTime()
{
    for (auto& player : players)
    {
        //-----����ł���v���C���[�ł͂Ȃ�������Ƃ΂�-----//
        if (player->GetObjectId() != private_object_id) continue;

        //�v���C���[���W���X�g���������slow
        return player->during_search_time();
    }
    return false;
}

bool PlayerManager::GetIsAlive()
{
    for (auto& player : players)
    {
        //-----�v���C���[����l�ł������Ă�����true��Ԃ�-----//
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
            //-----�o�����͕��킪����邩���������蔻����Ƃ�-----//
            int hit_count_1{ enemy_manager->fCalcPlayerAttackVsEnemies(player->GetSwordCapsuleParam(0).start,player->GetSwordCapsuleParam(0).end,player->GetSwordCapsuleParam(0).rasius,player->GetPlayerPower(),graphics,elapsed_time,block)};
            int hit_count_2{ enemy_manager->fCalcPlayerAttackVsEnemies(player->GetSwordCapsuleParam(1).start,player->GetSwordCapsuleParam(1).end,player->GetSwordCapsuleParam(1).rasius,player->GetPlayerPower(),graphics,elapsed_time,block)};

            player->AwakingAddCombo(hit_count_1,hit_count_2,block);
        }
        else
        {
            //-----�U���ɓ���������-----//
            int hit_count{ enemy_manager->fCalcPlayerAttackVsEnemies(
                player->GetSwordCapsuleParam(0).start,player->GetSwordCapsuleParam(0).end,player->GetSwordCapsuleParam(0).rasius,player->GetPlayerPower(),graphics,elapsed_time,block) };

            //-----���ʂ�����-----//
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
        //-----����L�����N�^�[����Ȃ�������Ƃ΂�-----//

        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            if (player->GetObjectId() != CorrespondenceManager::Instance().GetOperationPrivateId()) continue;
            //-----�v���C���[�̗̑͂��}���`�p�̃f�[�^�ɐݒ�(�������Ƃ�)-----//
            player->SetHealth(multiplay_current_health);

            if (enemy_manager->fCalcEnemiesAttackVsPlayer(player->GetBodyCapsuleParam().start,
                player->GetBodyCapsuleParam().end,
                player->GetBodyCapsuleParam().rasius, player->GetDamagedFunc()))
            {
                //-----�z�X�g�����̗̑͂̑��ʂ𑗐M����-----//
                if (CorrespondenceManager::Instance().GetHost())
                {
                    multiplay_current_health = player->GetHealth();
                    //=====�v���C���[�̗̑͂̃f�[�^�𑗐M����=====//
                    SendPlayerHealthData();
                }
            }
        }
        else
        {
            //-----�V���O���v���C�̎��̃_���[�W����-----//
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
        //-----����(���삵�Ă���v���C���[)�Ȃ�Ƃ΂�-----//
        if (player->GetObjectId() == private_object_id) continue;
        for (const auto enemy : enemy_manager->fGetEnemies())
        {
            //-----�����ԍ��łȂ�������Ƃ΂�-----//
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
        //-----�ݒ�ł�����for�����ʂ���-----//
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
        //-----�ݒ�ł�����for�����ʂ���-----//
        break;
    }
}

void PlayerManager::SetCameraPosition(DirectX::XMFLOAT3 eye)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetCameraPosition(eye);
        //-----�ݒ�ł�����for�����ʂ���-----//
        break;
    }
}

void PlayerManager::SetTarget(BaseEnemy* enemy)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetTarget(enemy);
        //-----�ݒ�ł�����for�����ʂ���-----//
        break;
    }
}

void PlayerManager::SetCameraTarget(DirectX::XMFLOAT3 target)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        player->SetCameraTarget(target);
        //-----�ݒ�ł�����for�����ʂ���-----//
        break;
    }
}

void PlayerManager::DashPostEffect(GraphicsPipeline& graphics, PostEffect* post_efc)
{
    for (auto& player : players)
    {
        if (player->GetObjectId() != private_object_id) continue;
        if (player->GetStartDashEffect()) post_efc->dash_post_effect(graphics.get_dc().Get(), player->GetPosition());
        //-----�ݒ�ł�����for�����ʂ���-----//
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
