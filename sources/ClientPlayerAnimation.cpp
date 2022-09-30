#include"ClientPlayer.h"

void ClientPlayer::ExecFuncUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
    (this->*player_activity)(elapsed_time,sky_dome);
}

void ClientPlayer::IdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //移動に遷移
    //チェイン攻撃のロックオン完了から攻撃終了の時は操作は受け付けない
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
    {
        TransitionMove();
    }
    else if (during_chain_attack() && change_normal_timer > 0 && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
    {
        TransitionMove();
    }

    //チェイン攻撃から戻ってきて数秒間は移動しかできない
    //チェイン攻撃の状態では移動以外の操作は受け付けない
    if (change_normal_timer <= 0 && behavior_state == Behavior::Normal)
    {
        //回避に遷移
        float length{ Math::calc_vector_AtoB_length(position, target) };
        if (avoidance_buttun == false && (triggerR || button_down & GamePad::BTN_RIGHT_SHOULDER))
        {
            //ジャスト回避なら
            if (is_lock_on && is_just_avoidance_capsul)
            {
                TransitionJustBehindAvoidance();
            }
            else
            {
                //後ろに回り込める距離なら回り込みようのUpdate
                if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                {
                    TransitionBehindAvoidance();
                }
                //そうじゃなかったら普通の回避
                else TransitionAvoidance();
            }
        }
        //突進開始に遷移
        if (button_down & GamePad::BTN_ATTACK_B)
        {
            TransitionChargeInit();
        }

        Awaiking();
    }
}

void ClientPlayer::MoveUpdate(float elapsed_time, SkyDome* sky_dome)
{

    //移動入力がなくなったら待機に遷移
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionIdle();
    }
    else if (during_chain_attack() && change_normal_timer > 0 && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionIdle();
    }
    //チェイン攻撃から戻ってきて数秒間は移動しかできない
    //チェイン攻撃の状態では移動以外の操作は受け付けない
    if (change_normal_timer < 0 && behavior_state == Behavior::Normal)
    {
        //回避に遷移
        float length{ Math::calc_vector_AtoB_length(position, target) };
        if (avoidance_buttun == false && (triggerR || button_down & GamePad::BTN_RIGHT_SHOULDER))
        {
            //ジャスト回避なら
            if (is_lock_on && is_just_avoidance_capsul)
            {
                TransitionJustBehindAvoidance();
            }
            else
            {
                //後ろに回り込める距離なら回り込みようのUpdate
                if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                {
                    TransitionBehindAvoidance();
                }
                //そうじゃなかったら普通の回避
                else
                {
                    TransitionAvoidance();
                }
            }
        }
        //突進開始に遷移
        if (button_down & GamePad::BTN_ATTACK_B)
        {
            TransitionChargeInit();
        }

        Awaiking();
    }
}

void ClientPlayer::AvoidanceUpdate(float elapsed_time, SkyDome* sky_dome)
{
    avoidance_boost_time += 1.0f * elapsed_time;
    //回避の時の加速
    SetAccelerationVelocity();

    //-----攻撃ボタンを押したら攻撃に遷移-----//
    if (button_down & GamePad::BTN_ATTACK_B)
    {
        if (target_enemy != nullptr && target_enemy->fGetPercentHitPoint() != 0)
        {
            attack_time = 0;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            TransitionAttackType1(attack_animation_blends_speeds.z);
        }
    }

    if (avoidance_boost_time > 1.0f)
    {
        model->progress_animation();
        if (model->end_of_animation(anim_parm))
        {
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            //回避中かどうかの設定
            is_avoidance = false;
            is_behind_avoidance = false;
            //移動入力があったら移動に遷移
            if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
            {
                TransitionMove();
            }
            //移動入力がなかったら待機に遷移
            else
            {
                TransitionIdle();
            }
        }
    }
    else
    {
        if (model->get_anim_para().animation_tick > 0.2f)
        {
            model->pause_animation();
        }
        //連続回避の回数が0より大きいときに
        if (avoidance_direction_count > 0)
        {
            //回避ボタンを押したら入力方向に方向転換
            if (avoidance_buttun == false && (triggerR > 0.5f || button_down & GamePad::BTN_RIGHT_SHOULDER))
            {
                avoidance_direction_count--;
                avoidance_buttun = true;
                velocity = {};
                DirectX::XMFLOAT3 movevec = GetMoveVecter();
                if ((movevec.x * movevec.x) + (movevec.z * movevec.z) > 0)
                {
                    charge_point = Math::calc_designated_point(position, movevec, 200.0f);
                }
                else
                {
                    charge_point = Math::calc_designated_point(position, forward, 200.0f);
                }
                //覚醒状態の時の回避アニメーションの設定
                if (is_awakening)model->play_animation(anim_parm,AnimationClips::AwakingAvoidance, false, true);
                //通常状態の時のアニメーションの設定
                else model->play_animation(anim_parm, AnimationClips::Avoidance, false, true);
                avoidance_boost_time = 0.0f;
            }
        }
    }
}

void ClientPlayer::BehindAvoidanceUpdate(float elapsed_time, SkyDome* sky_dome)
{
    behind_test_timer += 1.0f * elapsed_time;

    if (BehindAvoidanceMove(elapsed_time, behind_transit_index, position, behind_speed, behind_interpolated_way_points, 1.5f))
    {
        if (is_just_avoidance)
        {
            behaind_avoidance_recharge = false;
            behaind_avoidance_cool_time = 0.0f;
        }
        else
        {
            behaind_avoidance_recharge = true;
            behaind_avoidance_cool_time = 1.0f;
        }
        //回避中かどうかの設定
        is_avoidance = false;
        is_behind_avoidance = false;
        //ジャスト回避のフラグを初期化
        is_just_avoidance = false;
        TransitionIdle();
    }
    else
    {
        is_lock_on = true;
    }
}

void ClientPlayer::ChargeInitUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {
        TransitionCharge(attack_animation_blends_speeds.x);
    }
    //ChargeAcceleration(elapsed_time);
}

void ClientPlayer::ChargeUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //ブロックされていたら剣をふって怯む
    if (is_block)
    {
        TransitionAttackType1(attack_animation_blends_speeds.y);
    }
    charge_time += charge_add_time * elapsed_time;
    //攻撃の加速の設定
    SetAccelerationVelocity();

    //突進時間を超えたらそれぞれの遷移にとぶ
    if (charge_time > CHARGE_MAX_TIME)
    {

        velocity.x *= 0.2f;
        velocity.y *= 0.2f;
        velocity.z *= 0.2f;

        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
        {
            charge_time = 0;
            is_charge = false;
            TransitionMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            charge_time = 0;
            is_charge = false;
            charge_change_direction_count = CHARGE_DIRECTION_COUNT;
            TransitionIdle();
        }
        Awaiking();
    }
    else
    {
        if (is_enemy_hit)
        {
            //敵に当たって攻撃ボタン(突進ボタン)を押したら一撃目
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            charge_change_direction_count = CHARGE_DIRECTION_COUNT;
            is_enemy_hit = false;
            is_attack = false;
            TransitionAttackType1(attack_animation_blends_speeds.y);
        }
        if (is_lock_on == false && charge_change_direction_count > 0)
        {
            if (button_down & GamePad::BTN_ATTACK_B)
            {
                charge_change_direction_count--;
                velocity = {};
                DirectX::XMFLOAT3 movevec = GetMoveVecter();
                if ((movevec.x * movevec.x) + (movevec.z * movevec.z) > 0)
                {
                    charge_point = Math::calc_designated_point(position, movevec, 200.0f);
                }
                else
                {
                    charge_point = Math::calc_designated_point(position, forward, 200.0f);
                }
                //SetAccelerationVelocity();
                charge_time = 0;
                //TransitionCharge();
            }
        }
    }
    if (is_awakening)
    {
        mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
        mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
    }
    else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);

}

void ClientPlayer::AttackType1Update(float elapsed_time, SkyDome* sky_dome)
{
    if (is_awakening)
    {
        if (model->get_anim_para().animation_tick > 0.16f)
        {
            //ブロックされたかどうか
            if (is_block)
            {
                is_block = false;
                TransitionDamage();
            }
        }
    }
    else
    {
        if (model->get_anim_para().animation_tick > 0.11f)
        {
            //ブロックされたかどうか
            if (is_block)
            {
                is_block = false;
                TransitionDamage();
            }
        }
    }

    if (model->end_of_animation(anim_parm))
    {

        is_attack = false;
        attack_time += attack_add_time * elapsed_time;
        //猶予時間を超えたら待機に遷移
        if (attack_time > ATTACK_TYPE1_MAX_TIME)
        {
            attack_time = 0;
            TransitionIdle();
        }
        else
        {
            //猶予時間よりも早く押したら攻撃2撃目に遷移
            if (button_down & GamePad::BTN_ATTACK_B)
            {
                if (target_enemy != nullptr && target_enemy->fGetPercentHitPoint() != 0)
                {
                    attack_time = 0;
                    velocity.x *= 0.2f;
                    velocity.y *= 0.2f;
                    velocity.z *= 0.2f;
                    TransitionAttackType2(attack_animation_blends_speeds.z);
                }
            }
        }

    }
    if (is_awakening)
    {
        mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
        mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
    }
    else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);

}

void ClientPlayer::AttackType2Update(float elapsed_time, SkyDome* sky_dome)
{
    if (is_awakening)
    {
        if (model->get_anim_para().animation_tick > 0.25f)
        {
            //ブロックされたかどうか
            if (is_block)
            {
                is_block = false;
                TransitionDamage();
            }
        }
    }
    else
    {
        if (model->get_anim_para().animation_tick > 0.26f)
        {
            //ブロックされたかどうか
            if (is_block)
            {
                is_block = false;
                TransitionDamage();
            }
        }
    }

    if (is_update_animation == false)
    {
        attack_time += attack_add_time * elapsed_time;
        if (is_enemy_hit)
        {
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            is_charge = false;
            is_attack = false;
            attack_time = 0;
            is_update_animation = true;
            is_enemy_hit = false;
        }
        if (attack_time >= 0.6f)
        {
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            attack_time = 0;
            TransitionIdle();
        }

    }
    if (model->end_of_animation(anim_parm))
    {
        attack_time += attack_add_time * elapsed_time;
        //猶予時間を超えたら待機に遷移
        if (attack_time > ATTACK_TYPE2_MAX_TIME)
        {
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            attack_time = 0;
            TransitionIdle();
        }
        else
        {
            //猶予時間よりも早く押したら攻撃3撃目に遷移
            if (button_down & GamePad::BTN_ATTACK_B)
            {
                if (target_enemy != nullptr && target_enemy->fGetPercentHitPoint() != 0)
                {
                    velocity.x *= 0.2f;
                    velocity.y *= 0.2f;
                    velocity.z *= 0.2f;
                    attack_time = 0;
                    TransitionAttackType3(attack_animation_blends_speeds.w);
                }
            }
        }
    }
    if (is_awakening)
    {
        mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
        mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
    }
    else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);

}

void ClientPlayer::AttackType3Update(float elapsed_time, SkyDome* sky_dome)
{
    if (is_awakening)
    {
        if (model->get_anim_para().animation_tick > 0.37f)
        {
            //ブロックされたかどうか
            if (is_block)
            {
                is_block = false;
                TransitionDamage();
            }
        }
    }
    else
    {
        if (model->get_anim_para().animation_tick > 0.35f)
        {
            //ブロックされたかどうか
            if (is_block)
            {
                is_block = false;
                TransitionDamage();
            }
        }
    }

    //敵に当たったか時間が2秒たったら加速を終わる

    if (is_update_animation == false)
    {
        attack_time += attack_add_time * elapsed_time;
        if (is_enemy_hit)
        {
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            is_charge = false;
            is_attack = false;
            attack_time = 0;
            is_update_animation = true;
            is_enemy_hit = false;
        }
        if (attack_time >= 0.6f)
        {
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            attack_time = 0;
            TransitionIdle();
        }
    }
    if (model->end_of_animation(anim_parm))
    {
        attack_time += attack_add_time * elapsed_time;
        if (attack_time > ATTACK_TYPE3_MAX_TIME)
        {
            //移動入力があったら移動に遷移
            if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
            {
                velocity.x *= 0.2f;
                velocity.y *= 0.2f;
                velocity.z *= 0.2f;
                charge_time = 0;
                TransitionMove();
            }
            //移動入力がなかったら待機に遷移
            else
            {
                velocity.x *= 0.2f;
                velocity.y *= 0.2f;
                velocity.z *= 0.2f;
                charge_time = 0;
                TransitionIdle();
            }

        }
        else
        {
            if (button_down & GamePad::BTN_ATTACK_B)
            {
                if (target_enemy != nullptr && target_enemy->fGetPercentHitPoint() != 0)
                {
                    attack_time = 0;
                    velocity.x *= 0.2f;
                    velocity.y *= 0.2f;
                    velocity.z *= 0.2f;
                    TransitionCharge(attack_animation_blends_speeds.z);
                }
            }
        }
    }
    if (is_awakening)
    {
        mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
        mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
    }
    else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);

}

void ClientPlayer::SpecialSurgeUpdate(float elapsed_time, SkyDome* sky_dome)
{
}

void ClientPlayer::OpportunityUpdate(float elapsed_time, SkyDome* sky_dome)
{
    opportunity_timer += 1.0f * elapsed_time;
    if (special_surge_combo_count > 0)special_surge_opportunity = 2.0f / special_surge_combo_count;
    else special_surge_opportunity = 2.0f;

    //設定した隙よりも時間がたったらそれぞれの行動に遷移する
    if (opportunity_timer > special_surge_opportunity)
    {
        special_surge_combo_count = 0;
        TransitionTransformHum();
    }
}

void ClientPlayer::DamageUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {
        TransitionIdle();
    }
}

void ClientPlayer::TransformHumUpdate(float elapsed_time, SkyDome* sky_dome)
{
    position.y = Math::lerp(position.y, 0.0f, 1.0f * elapsed_time);
    if (model->end_of_animation(anim_parm))
    {
        //クリア演出中なら解除する
        if (during_clear) during_clear = false;
        TransitionIdle();
    }
}

void ClientPlayer::TransformWingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {
        TransitionSpecialSurge();
    }
}

void ClientPlayer::AwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {

        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
        {
            TransitionMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            TransitionIdle();
        }
    }
}

void ClientPlayer::InvAwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {
        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
        {
            TransitionMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            TransitionIdle();
        }
    }
}

void ClientPlayer::StageMoveUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {
        TransitionWingDashStart();
    }
}

void ClientPlayer::WingDashStartUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {
        TransitionWingDashIdle();
    }

}
void ClientPlayer::WingDashIdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    position.y = Math::lerp(position.y, 2.0f, 1.0f * elapsed_time);
}

void ClientPlayer::WingDashEndUpdate(float elapsed_time, SkyDome* sky_dome)
{
    position.y = Math::lerp(position.y, 0.0f, 1.0f * elapsed_time);

    if (model->end_of_animation(anim_parm))
    {
        TransitionTransformHum();
    }
}

void ClientPlayer::DieUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
    {
        TransitionDying();
    }
}

void ClientPlayer::DyingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    is_dying_update = true;
    threshold += 1.0f * elapsed_time;
    threshold_mesh += 1.0f * elapsed_time;
    if (threshold > 1.0f && threshold_mesh > 1.0f)
    {
        is_alive = false;
    }
}

void ClientPlayer::StartMothinUpdate(float elapsed_time, SkyDome* sky_dome)
{
}


void ClientPlayer::Awaiking()
{
    //チェイン攻撃中は覚醒状態の各遷移にはとばない
    if (behavior_state == Behavior::Normal)
    {
        //ボタン入力
        if (game_pad->get_button() & GamePad::BTN_A)
        {
            if (combo_count >= MAX_COMBO_COUNT && is_awakening == false)
            {
                TransitionAwaking();//コンボカウントが最大のときは覚醒状態になる
            }
        }
        if (is_awakening && combo_count <= 0)
        {
            //覚醒状態かどうかの設定
            is_awakening = false;
            TransitionInvAwaking();//覚醒状態のときにカウントが0になったら通常状態になる
        }
    }
}

