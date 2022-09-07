#include"Player.h"
void Player::ExecFuncUpdate(float elapsed_time)
{
    (this->*player_title_activity)(elapsed_time);
}
void Player::UpdateTitleAnimationReadyIdle(float elaosed_time)
{
    if (start_title_animation)
    {
        model->play_animation(AnimationClips::TitleAnimationStart);
        player_title_activity = &Player::UpdateTitleAnimationStart;
    }
}
void Player::UpdateTitleAnimationStart(float elaosed_time)
{
    if (model->end_of_animation())
    {
        model->play_animation(AnimationClips::TitleAnimationStartIdle);
        player_title_activity = &Player::UpdateTitleAnimationStartIdle;
    }
}
void Player::UpdateTitleAnimationStartIdle(float elaosed_time)
{
    title_timer += 1.0f * elaosed_time;
    //モデルのアニメーションが終わった時かつ1秒たったら
    if (model->end_of_animation() && title_timer > 0.5f)
    {
        title_timer = 0;
        model->play_animation(AnimationClips::TitleAnimationEnd);
        player_title_activity = &Player::UpdateTitleAnimationEnd;
    }
}
void Player::UpdateTitleAnimationEnd(float elaosed_time)
{
    if (model->end_of_animation())
    {
        model->play_animation(AnimationClips::TitleAnimationEndIdle);
        player_title_activity = &Player::UpdateTitleAnimationEndIdle;
    }
}
void Player::UpdateTitleAnimationEndIdle(float elaosed_time)
{
    title_timer += 1.0f * elaosed_time;
    if(title_timer > 0.5) end_title_animation = true;

    if (model->end_of_animation())
    {
    }
}
void Player::TransitionTitleAnimationReadyIdle()
{
    model->play_animation(AnimationClips::TitleAnimationReadyIdle,true);
    //ここでupdateしておかないと1フレームだけ初期のアニメーションが映ってしまうから
    model->update_animation(1.0f);
    //タイトルはメッシュを隠さないから0にしておく
    threshold_mesh = 0.0f;
}
void Player::ExecFuncUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
    switch (behavior_state)
    {
    case Player::Behavior::Normal:
        //自分のクラスの関数ポインタを呼ぶ
        (this->*player_activity)(elapsed_time, sky_dome);
        break;
    case Player::Behavior::Chain:
        //自分のクラスの関数ポインタを呼ぶ
        if(during_chain_attack() == false)(this->*chain_activity)(elapsed_time, sky_dome);
        (this->*player_chain_activity)(elapsed_time, enemies,Graphics_);
        break;
    default:
        break;
    }
}

void Player::IdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //移動に遷移
    //チェイン攻撃のロックオン完了から攻撃終了の時は操作は受け付けない
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
    {
        TransitionMove();
    }
    else if (during_chain_attack() && change_normal_timer > 0 && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
    {
        TransitionMove();
    }

    //チェイン攻撃から戻ってきて数秒間は移動しかできない
    //チェイン攻撃の状態では移動以外の操作は受け付けない
    if (change_normal_timer < 0 && behavior_state == Behavior::Normal)
    {
        //回避に遷移
        float length{ Math::calc_vector_AtoB_length(position, target) };
        if (avoidance_buttun == false && (game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER))
        {
            //ジャスト回避なら
            if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
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
        if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
        {
            TransitionChargeInit();
        }

        Awaiking();
    }
    UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::MoveUpdate(float elapsed_time, SkyDome* sky_dome)
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
        if (avoidance_buttun == false && (game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER))
        {
            //ジャスト回避なら
            if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
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
        if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
        {
            TransitionChargeInit();
        }

        Awaiking();
    }
    UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::AvoidanceUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //エフェクトの位置，回転設定
    player_air_registance_effec->set_position(effect_manager->get_effekseer_manager(),position);
    player_air_registance_effec->set_quaternion(effect_manager->get_effekseer_manager(), orientation);
    avoidance_boost_time += 1.0f * elapsed_time;
    //回避の時の加速
    SetAccelerationVelocity();
    //ロックオンしている敵と一定距離近くなったら
    float length{ Math::calc_vector_AtoB_length(position, target) };
    if (is_lock_on && length < 15.0f)
    {
        player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
        //攻撃に遷移
        velocity.x *= 0.2f;
        velocity.y *= 0.2f;
        velocity.z *= 0.2f;
        TransitionAttackType1();
    }
    if (avoidance_boost_time > 1.0f)
    {
        model->progress_animation();
        if (model->end_of_animation())
        {
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            //回避中かどうかの設定
            is_avoidance = false;
            is_behind_avoidance = false;
            //移動入力があったら移動に遷移
            if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
            {
                player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
                TransitionMove();
            }
            //移動入力がなかったら待機に遷移
            else
            {
                player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
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
            if (avoidance_buttun == false && (game_pad->get_trigger_R() > 0.5f || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER))
            {
                player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
                player_air_registance_effec->play(effect_manager->get_effekseer_manager(), position, 0.3f);
                avoidance_direction_count--;
                avoidance_buttun = true;
                velocity = {};
                DirectX::XMFLOAT3 movevec = SetMoveVec(camera_forward, camera_right);
                if ((movevec.x * movevec.x) + (movevec.z * movevec.z) > 0)
                {
                    ChargeTurn(elapsed_time, movevec, turn_speed, position, orientation);
                    charge_point = Math::calc_designated_point(position, movevec, 200.0f);
                }
                else
                {
                    ChargeTurn(elapsed_time, forward, turn_speed, position, orientation);
                    charge_point = Math::calc_designated_point(position, forward, 200.0f);
                }
                audio_manager->play_se(SE_INDEX::AVOIDANCE);
                //覚醒状態の時の回避アニメーションの設定
                if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
                //通常状態の時のアニメーションの設定
                else model->play_animation(AnimationClips::Avoidance, false, true);
                avoidance_boost_time = 0.0f;
            }
        }
    }
    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::BehindAvoidanceUpdate(float elapsed_time, SkyDome* sky_dome)
{
    player_behaind_effec_2->set_position(effect_manager->get_effekseer_manager(), position);
    player_behaind_effec_2->set_quaternion(effect_manager->get_effekseer_manager(), orientation);

    just_stun->set_position(effect_manager->get_effekseer_manager(), target);
    behind_test_timer += 1.0f * elapsed_time;
    //behind_timer += 2.0f * elapsed_time;
    player_behind_effec->set_position(effect_manager->get_effekseer_manager(), { position.x,position.y + air_registance_offset_y ,position.z });
    //BehindAvoidanceMove(elapsed_time);
    if (BehindAvoidanceMove(elapsed_time, behind_transit_index,position, behind_speed, behind_interpolated_way_points,1.5f))
    {
        just_stun->stop(effect_manager->get_effekseer_manager());
        player_behaind_effec_2->stop(effect_manager->get_effekseer_manager());
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
        player_behind_effec->stop(effect_manager->get_effekseer_manager());
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
    UpdateBehindAvoidanceVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::ChargeInitUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        TransitionCharge(attack_animation_blends_speeds.x);
    }
    //ChargeAcceleration(elapsed_time);
    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::ChargeUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //エフェクトの位置，回転設定
    player_air_registance_effec->set_position(effect_manager->get_effekseer_manager(), position);
    player_air_registance_effec->set_quaternion(effect_manager->get_effekseer_manager(), orientation);
    start_dash_effect = false;
    charge_time += charge_add_time * elapsed_time;
    //ChargeAcceleration(elapsed_time);
    //攻撃の加速の設定
    SetAccelerationVelocity();
    //ブロックされていたら剣をふって怯む
    if (is_block)
    {
        TransitionAttackType1(attack_animation_blends_speeds.y);
    }
    //突進時間を超えたらそれぞれの遷移にとぶ
    if (charge_time > CHARGE_MAX_TIME)
    {

        audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
        PostEffect::clear_post_effect();
        velocity.x *= 0.2f;
        velocity.y *= 0.2f;
        velocity.z *= 0.2f;

        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
        {
            charge_time = 0;
            is_charge = false;
            player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
            TransitionMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            charge_time = 0;
            is_charge = false;
            charge_change_direction_count = CHARGE_DIRECTION_COUNT;
            player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
            TransitionIdle();
        }
        Awaiking();
    }
    else
    {
        if (is_enemy_hit)
        {
            player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
            PostEffect::clear_post_effect();
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
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
                //エフェクト再生
                player_air_registance_effec->play(effect_manager->get_effekseer_manager(), position, 0.3f);
                audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
                audio_manager->play_se(SE_INDEX::PLAYER_RUSH);

                charge_change_direction_count--;
                velocity = {};
                DirectX::XMFLOAT3 movevec = SetMoveVec(camera_forward, camera_right);
                if ((movevec.x * movevec.x) + (movevec.z * movevec.z) > 0)
                {
                    ChargeTurn(elapsed_time, movevec, turn_speed, position, orientation);
                    charge_point = Math::calc_designated_point(position, movevec, 200.0f);
                }
                else
                {
                    ChargeTurn(elapsed_time, forward, turn_speed, position, orientation);
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

    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::AttackType1Update(float elapsed_time, SkyDome* sky_dome)
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

    if (model->end_of_animation())
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
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
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

    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::AttackType2Update(float elapsed_time, SkyDome* sky_dome)
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

    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    if (is_update_animation == false)
    {
        attack_time += attack_add_time * elapsed_time;
        SetAccelerationVelocity();
        if (is_enemy_hit)
        {
            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
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
            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            attack_time = 0;
            TransitionIdle();
        }

    }
    if (model->end_of_animation())
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
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
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

    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::AttackType3Update(float elapsed_time, SkyDome* sky_dome)
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

    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //敵に当たったか時間が2秒たったら加速を終わる

    if (is_update_animation == false)
    {
        attack_time += attack_add_time * elapsed_time;
        SetAccelerationVelocity();
        if (is_enemy_hit)
        {

            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
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
            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            attack_time = 0;
            TransitionIdle();
        }
    }
    if (model->end_of_animation())
    {
        attack_time += attack_add_time * elapsed_time;
        if (attack_time > ATTACK_TYPE3_MAX_TIME)
        {
            //移動入力があったら移動に遷移
            if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
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
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
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

    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::SpecialSurgeUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //special_surge_timer += 1.0f * elapsed_time;

    //if (special_surge_timer > 1.0f)
    //{
    //    velocity = {};
    //    TransitionOpportunity();
    //}
    UpdateSpecialSurgeVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::OpportunityUpdate(float elapsed_time, SkyDome* sky_dome)
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
        //UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::DamageUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        TransitionIdle();
    }
}

void Player::TransformHumUpdate(float elapsed_time, SkyDome* sky_dome)
{
    position.y = Math::lerp(position.y, 0.0f, 1.0f * elapsed_time);
    if (model->end_of_animation())
    {
        //クリア演出中なら解除する
        if (during_clear) during_clear = false;
        TransitionIdle();
    }
}

void Player::TransformWingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        TransitionSpecialSurge();
    }
    //ロックオンしている敵の方を見る
    UpdateRotateToTarget(elapsed_time, position, orientation, camera_forward, camera_position);
   //UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::AwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        audio_manager->play_se(SE_INDEX::PLAYER_AWAKING);
        player_awaiking_effec->stop(effect_manager->get_effekseer_manager());

        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
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

void Player::InvAwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
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

void Player::StageMoveUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        TransitionWingDashStart();
    }
}

void Player::WingDashStartUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        TransitionWingDashIdle();
    }

}
void Player::WingDashIdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    position.y = Math::lerp(position.y, 2.0f, 1.0f * elapsed_time);
}

void Player::WingDashEndUpdate(float elapsed_time, SkyDome* sky_dome)
{
    position.y = Math::lerp(position.y, 0.0f, 1.0f * elapsed_time);

    if (model->end_of_animation())
    {
        TransitionTransformHum();
    }
}

void Player::DieUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        TransitionDying();
    }
}

void Player::DyingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    is_dying_update = true;
    threshold += 1.0f * elapsed_time;
    threshold_mesh += 1.0f * elapsed_time;
    if (threshold > 1.0f && threshold_mesh > 1.0f)
    {
        is_alive = false;
    }
}

void Player::StartMothinUpdate(float elapsed_time, SkyDome* sky_dome)
{
}

void Player::NamelessMotionUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (wipe_parm < 0.15f)
    {
        wipe_parm += 0.2f * elapsed_time;
        PostEffect::wipe_effect(wipe_parm);
    }

    if (model->get_anim_para().animation_tick > 0.3f && nameless_motion_se_state == 0)
    {
        audio_manager->play_se(SE_INDEX::SWING_SWORD1);
        nameless_motion_se_state = 1;
    }
    if (model->get_anim_para().animation_tick > 0.56f && nameless_motion_se_state == 1)
    {
        audio_manager->play_se(SE_INDEX::SWING_SWORD2);
        nameless_motion_se_state = 2;
    }
    if (model->get_anim_para().animation_tick > 4.0f && nameless_motion_se_state == 2)
    {
        audio_manager->play_se(SE_INDEX::RETURN_SWORD);
        nameless_motion_se_state = 3;
    }



    if (model->end_of_animation())
    {
        TransitionNamelessMotionIdle();
    }
}

void Player::NamelessMotionIdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        if (is_end_clear_motion == false)
        {
            wipe_parm -= 0.2f * elapsed_time;
            PostEffect::wipe_effect(wipe_parm);
        }
        //クリアモーションが終わったことを伝える
        if (wipe_parm < 0)
        {
            PostEffect::clear_post_effect();
            wipe_parm = 0.0f;
            is_end_clear_motion = true;
        }
    }
}

void Player::Awaiking()
{
    //チェイン攻撃中は覚醒状態の各遷移にはとばない
    if (behavior_state == Behavior::Normal)
    {
        //ボタン入力
        if (game_pad->get_button() & GamePad::BTN_A)
        {
            if (combo_count >= MAX_COMBO_COUNT&& is_awakening == false)
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

void Player::TransitionIdle(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //ダッシュエフェクトの終了
    //end_dash_effect = true;
    //覚醒状態の時の待機アニメーションにセット
    if(is_awakening)model->play_animation(AnimationClips::AwakingIdle, true,true,blend_second);
    //通常状態の待機アニメーションにセット
    else model->play_animation(AnimationClips::Idle, true,true,blend_second);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //待機状態の時の更新関数に切り替える
    player_activity = &Player::IdleUpdate;
}

void Player::TransitionMove(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    //エフェクト再生
    player_move_effec_r->play(effect_manager->get_effekseer_manager(), step_pos_r);
    player_move_effec_l->play(effect_manager->get_effekseer_manager(), step_pos_l);
    //ダッシュエフェクトの終了
    //end_dash_effect = true;
    //覚醒状態の時の移動アニメーションの設定
    if(is_awakening)model->play_animation(AnimationClips::AwakingMove, true,true, blend_second);
    //通常状態の時に移動アニメーションの設定
    else model->play_animation(AnimationClips::Move, true, true,blend_second);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //移動状態の時の更新関数に切り替える
    player_activity = &Player::MoveUpdate;
}

void Player::TransitionAvoidance()
{
    //player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    audio_manager->play_se(SE_INDEX::AVOIDANCE);
    //エフェクト再生
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
    player_air_registance_effec->play(effect_manager->get_effekseer_manager(), position,0.3f);
    player_air_registance_effec->set_speed(effect_manager->get_effekseer_manager(), AVOIDANCE_ANIMATION_SPEED);
    //回避中かどうかの設定
    is_avoidance = true;
    //回り込み回避かどうか
    is_behind_avoidance = false;
    //--------------------------イージング加速の変数初期化---------------------------------//
#if 0
    avoidance_boost_time = 0;
    avoidance_start = velocity;
    if (is_lock_on)
    {
        leverage = 15.0f;
        DirectX::XMFLOAT3 movevec = SetMoveVec(camera_forward, camera_right);
        if (sqrtf((movevec.x * movevec.x) + (movevec.y * movevec.y) + (movevec.z * movevec.z)) <= 0.0f)
        {
            avoidance_end = { forward.x * leverage ,forward.y * leverage,forward.z * leverage };
        }
        else
        {
            avoidance_end = { movevec.x * leverage ,movevec.y * leverage,movevec.z * leverage };
        }
    }
    else
    {
        leverage = 30.0f;
        avoidance_end = { forward.x * leverage ,forward.y * leverage,forward.z * leverage };
    }

#endif // 0
    avoidance_boost_time = 0.0f;
    //方向転換の回数
    avoidance_direction_count = 3;
    //ロックオンしてない場合のターゲットの設定
    charge_point = Math::calc_designated_point(position, forward, 200.0f);

    //-----------------------------------------------------------------------------------------//
    //覚醒状態の時の回避アニメーションの設定
    if(is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false,true);
    //通常状態の時のアニメーションの設定
    else model->play_animation(AnimationClips::Avoidance, false,true);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = AVOIDANCE_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //回避状態の時の更新関数に切り替える
    player_activity = &Player::AvoidanceUpdate;
}

void Player::TransitionBehindAvoidance()
{
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_behaind_effec_2->play(effect_manager->get_effekseer_manager(), position,2.0f);

    behind_test_timer = 0.0f;
    audio_manager->play_se(SE_INDEX::WRAPAROUND_AVOIDANCE);
    player_behind_effec->play(effect_manager->get_effekseer_manager(), {position.x,position.y + air_registance_offset_y ,position.z});
    if (target_enemy != nullptr)
    {
        //ロックオンしている敵をスタンさせる
        target_enemy->fSetStun(true);
    }
#if 0
    if (is_just_avoidance_capsul)
    {
        //ロックオンしている敵をスタンさせる
        if (target_enemy != nullptr)
        {
            target_enemy->fSetStun(true, true);
        }
        is_just_avoidance = true;
    }
    else
    {
        if (target_enemy != nullptr)
        {
            //ロックオンしている敵をスタンさせる
            target_enemy->fSetStun(true);
        }
    }

#endif // 0
    velocity = {};
    //回避中かどうかの設定
    is_avoidance = true;
    //回り込み回避かどうか
    is_behind_avoidance = true;
    //覚醒状態の時の回避アニメーションの設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
    //通常状態の時のアニメーションの設定
    else model->play_animation(AnimationClips::Avoidance, false, true);
    //後ろに回り込む座標の取得
    BehindAvoidancePosition();
    //回り込むときのタイマー
    behind_timer = 0;
    //回り込みの補完レート
    behind_late = 0;
    //移動速度の初期化
    velocity = {};
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //背後に回り込むときの関数に切り替える
    player_activity = &Player::BehindAvoidanceUpdate;

}

void Player::TransitionJustBehindAvoidance()
{
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());

    player_behaind_effec_2->play(effect_manager->get_effekseer_manager(), position,2.0f);
    audio_manager->play_se(SE_INDEX::WRAPAROUND_AVOIDANCE);
        //ロックオンしている敵をスタンさせる
    if (target_enemy != nullptr)
    {
        target_enemy->fSetStun(true, true);
    }
    if (is_awakening)
    {
        just_stun->play(effect_manager->get_effekseer_manager(), target, 6.0f);
    }
    else
    {
        just_stun->play(effect_manager->get_effekseer_manager(), target, 3.0f);
    }
    //HP回復する
    player_health += JUST_AVOIDANCE_HEALTH;
    //コンボゲージ増やす
    combo_count += JUST_AVOIDANCE_COMBO;
    is_just_avoidance = true;
    velocity = {};
    //回避中かどうかの設定
    is_avoidance = true;
    //回り込み回避かどうか
    is_behind_avoidance = true;
    //覚醒状態の時の回避アニメーションの設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
    //通常状態の時のアニメーションの設定
    else model->play_animation(AnimationClips::Avoidance, false, true);
    //後ろに回り込む座標の取得
    BehindAvoidancePosition();
    //回り込むときのタイマー
    behind_timer = 0;
    //回り込みの補完レート
    behind_late = 0;
    //移動速度の初期化
    velocity = {};
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //背後に回り込むときの関数に切り替える
    player_activity = &Player::BehindAvoidanceUpdate;
}

void Player::TransitionChargeInit()
{
    //player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //覚醒状態の時の突進の始まりのアニメーションに設定
   if(is_awakening)model->play_animation(AnimationClips::AwakingChargeInit, false,true);
   //通常状態の時の突進の始まりのアニメーションに設定
   else model->play_animation(AnimationClips::ChargeInit, false,true);
   //攻撃中かどうかの設定
   is_attack = true;
   //突進中かどうかの設定
   is_charge = true;
   //アニメーション速度の設定
    animation_speed = CHARGEINIT_ANIMATION_SPEED;
    //ロックオンしてない場合のターゲットの設定
    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //加速のレート
    lerp_rate = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //突進の始まりの時の更新関数に切り替える
    player_activity = &Player::ChargeInitUpdate;
}

void Player::TransitionCharge(float blend_seconds)
{
    //player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
    //エフェクト再生
    player_air_registance_effec->play(effect_manager->get_effekseer_manager(), position, 0.3f);
    //ダッシュポストエフェクトをかける
    start_dash_effect = true;
    //覚醒状態の時の突進アニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = CHARGE_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingCharge, false, true, blend_seconds);
    }
    //通常状態の時の突進アニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = CHARGE_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::Charge, false, true, blend_seconds);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //突進中かどうかの設定
    is_charge = true;
    //アニメーションスピードの設定
#if 1
    animation_speed = CHARGE_ANIMATION_SPEED;
#else
    //デバッグ用
    animation_speed = attack_animation_speeds.x;
#endif // 0
    //charge_point = Math::calc_designated_point(position, forward, 60.0f);
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //加速のレート
    lerp_rate = 4.0f;
    //攻撃の加速の設定
    //SetAccelerationVelocity();
    //突進中の更新関数に切り替える
    player_activity = &Player::ChargeUpdate;

}
void Player::TransitionAttackType1(float blend_seconds)
{
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());

    //覚醒状態の時の１撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType1, false, true, blend_seconds);
    }
    //通常状態の時の１撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType1, false, true, blend_seconds);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーションスピードの設定
    animation_speed = ATTACK1_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //加速のレート
    lerp_rate = 4.0f;
    //１撃目の更新関数に切り替える
    player_activity = &Player::AttackType1Update;
}

void Player::TransitionAttackType2(float blend_seconds)
{
    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);

    //覚醒状態の時の２撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType2, false, true, blend_seconds);
    }
    //通常状態の時の２撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType2, false, true, blend_seconds);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーション速度の設定
    animation_speed = ATTACK2_ANIMATION_SPEED;
    //攻撃の加速の設定
    //SetAccelerationVelocity();
        //ロックオンしてない場合のターゲットの設定
    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //加速のレート
    lerp_rate = 2.0f;
    //攻撃の時間
    attack_time = 0;
    //アニメーションをしていいかどうか
    is_update_animation = false;
    //突進中かどうかの設定
    is_charge = true;
    //２撃目の更新関数に切り替える
    player_activity = &Player::AttackType2Update;
}
void Player::TransitionAttackType3(float blend_seconds)
{
    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);

    //覚醒状態の時の３撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType3, false, true, blend_seconds);
    }
    //通常状態の時の３撃目ののアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType3, false, true, blend_seconds);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーション速度の設定
    animation_speed = ATTACK3_ANIMATION_SPEED;
    //攻撃の加速の設定
    //SetAccelerationVelocity();
    //加速のレート
    lerp_rate =2.0f;
    //攻撃の時間
    attack_time = 0;
    //アニメーションをしていいかどうか
    is_update_animation = false;
    //突進中かどうかの設定
    is_charge = true;
    //３撃目の更新関数に切り替える
    player_activity = &Player::AttackType3Update;
}

void Player::TransitionSpecialSurge()
{
    //飛行機モードになるアニメーションに設定
    model->play_animation(AnimationClips::IdleWing, true,true);
    //ゲージ消費の突進中に当たった敵の数を初期化
    special_surge_combo_count = 0;
    //ゲージ消費の突進かどうかの設定
    is_special_surge = true;
    //攻撃中かどうかの設定
    is_attack = true;
    //ロックオンしてない場合のターゲットの設定
    charge_point = Math::calc_designated_point(position, forward, 100.0f);
    //ゲージ消費の突進の移動速度を設定
    SpecialSurgeAcceleration();
    //コンボカウントの消費
    combo_count -= 10.0f;
    //コンボカウントの制限
    combo_count = Math::clamp(combo_count, 0.0f, MAX_COMBO_COUNT);
    //ゲージ消費の突進のタイマー
    //special_surge_timer = 0.0f;
    //アニメーションスピードの設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //ゲージ消費の突進の更新関数に切り替える
    player_activity = &Player::SpecialSurgeUpdate;
}

void Player::TransitionOpportunity()
{
    //ゲージ消費の突進攻撃終了
    is_special_surge = false;
    //攻撃中かどうかの設定
    is_attack = false;
    //隙が生じた時の経過時間をリセット
    //special_surge_timer = 0;
    //アニメーションスピードの設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //ゲージ消費の突進の隙の更新関数に切り替える
    player_activity = &Player::OpportunityUpdate;
}

void Player::TransitionDamage()
{
    PostEffect::clear_post_effect();
    velocity = {};
    //ダッシュエフェクトの終了
    start_dash_effect = false;
    //攻撃中かどうかの設定
    is_attack = false;
    //覚醒状態の時のダメージアニメーションに設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingDamage, false, true);
    //通常状態の時のアニメーションに設定
    else model->play_animation(AnimationClips::Damage, false, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //ダメージ受けたときの更新関数に切り替える
    player_activity = &Player::DamageUpdate;
}

void Player::TransitionTransformHum()
{
    //人型になるアニメーションに設定
    model->play_animation(AnimationClips::TransformHum, false,true);
    //アニメーション速度の設定
    animation_speed = TRANSFORM_HUM_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //人型になってるときの更新関数に切り替える
    player_activity = &Player::TransformHumUpdate;
}

void Player::TransitionTransformWing()
{
    velocity = {};
    //飛行機モードになるアニメーションに設定
    model->play_animation(AnimationClips::TransformWing, false,true);
    //アニメーション速度の設定
    animation_speed = TRANSFORM_WING_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //飛行機モード中の更新関数に切り替える
    player_activity = &Player::TransformWingUpdate;
}

void Player::TransitionAwaking()
{
    invincible_timer = 2.0f;
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    player_awaiking_effec->play(effect_manager->get_effekseer_manager(), position,2.0f);
    invincible_timer = 2.0f;
    //覚醒状態になるアニメーションに設定
    model->play_animation(AnimationClips::Awaking, false,true);
    //覚醒状態かどうかの設定
    is_awakening = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //覚醒状態になる途中の更新関数に切り替える
    player_activity = &Player::AwakingUpdate;
}

void Player::TransitionInvAwaking()
{
    invincible_timer = 2.0f;
    //通常状態に戻るアニメーションに設定
    model->play_animation(AnimationClips::InvAwaking, false,true);
    //覚醒状態かどうかの設定
    is_awakening = false;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //通常状態に戻ってるときの更新関数に切り替える
    player_activity = &Player::InvAwakingUpdate;

}

void Player::TransitionWingDashStart()
{
    model->play_animation(AnimationClips::WingDashStart, false, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    player_activity = &Player::WingDashStartUpdate;

}

void Player::TransitionWingDashIdle()
{
    model->play_animation(AnimationClips::WingDashIdle, true, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    player_activity = &Player::WingDashIdleUpdate;

}

void Player::TransitionWingDashEnd()
{
}
void Player::TransitionDie()
{
    //攻撃中かどうかの設定
    is_attack = false;
    velocity = {};
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //覚醒状態の時のダメージアニメーションに設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingDie, false, true);
    //通常状態の時のアニメーションに設定
    else model->play_animation(AnimationClips::Die, false, true);
    //更新関数に切り替え
    player_activity = &Player::DieUpdate;
    if (GameFile::get_instance().get_vibration())game_pad->set_vibration(1.0f, 1.0f, 1.0f);
}

void Player::TransitionDying()
{
    //攻撃中かどうかの設定
    is_attack = false;
    velocity = {};
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //覚醒状態の時のダメージアニメーションに設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingDying, true, true);
    //通常状態の時のアニメーションに設定
    else model->play_animation(AnimationClips::Dying, true, true);
    //更新関数に切り替え
    player_activity = &Player::DyingUpdate;

}

void Player::TransitionNamelessMotionIdle()
{
    //攻撃中かどうかの設定
    is_attack = false;
    velocity = {};
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションに設定
    model->play_animation(AnimationClips::NamelessMotionIdle, false, true);
    //更新関数に切り替え
    player_activity = &Player::NamelessMotionIdleUpdate;

}

void Player::TransitionStartMothin()
{
    //攻撃中かどうかの設定
    is_attack = false;
    velocity = {};
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションに設定
    //model->play_animation(AnimationClips::StartMothin, false, true);
    //更新関数に切り替え
    player_activity = &Player::StartMothinUpdate;

}

void Player::TransitionNamelessMotion()
{
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //クリア用モーションが始まったらからtrueにする
    is_start_cleear_motion = true;
    //攻撃中かどうかの設定
    is_attack = false;
    velocity = {};
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションに設定
    model->play_animation(AnimationClips::NamelessMotion, false, true);
    //更新関数に切り替え
    player_activity = &Player::NamelessMotionUpdate;
}

void Player::TransitionStageMove()
{
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //ステージ遷移の時に回復する
    const float health = static_cast<float>(player_health) /static_cast<float>(MAX_HEALTH);
    if (health < 0.7f) player_health = static_cast<int>(MAX_HEALTH * 0.7);
    //player_health += RECOVERY_HEALTH;
    velocity = {};
    //移動のアニメーションにする()
    model->play_animation(AnimationClips::TransformWing, false);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //通常状態に戻ってるときの更新関数に切り替える
    player_activity = &Player::StageMoveUpdate;
    during_clear = true;
}

void Player::TransitionStageMoveEnd()
{
    model->play_animation(AnimationClips::WingDashEnd, false, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    player_activity = &Player::WingDashEndUpdate;

}

void Player::ChainIdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
    {
        TransitionChainMove();
    }
    UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);

}
void Player::ChainMoveUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //移動入力がなくなったら待機に遷移
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionChainIdle();
    }
    UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}
void Player::TransitionChainIdle(float blend_second)
{
    if (is_awakening)model->play_animation(AnimationClips::AwakingIdle, true, true, blend_second);
    //通常状態の待機アニメーションにセット
    else model->play_animation(AnimationClips::Idle, true, true, blend_second);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //待機状態の時の更新関数に切り替える
    player_activity = &Player::ChainIdleUpdate;
}
void Player::TransitionChainMove(float blend_second)
{
    if (is_awakening)model->play_animation(AnimationClips::AwakingMove, true, true, blend_second);
    //通常状態の時に移動アニメーションの設定
    else model->play_animation(AnimationClips::Move, true, true, blend_second);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //移動状態の時の更新関数に切り替える
    player_activity = &Player::ChainMoveUpdate;
}

