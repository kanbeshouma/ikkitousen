#include"Player.h"
#include"imgui_include.h"
#include"user.h"
#include"easing.h"

#include "BaseCamera.h"

void Player::UpdateTutorial(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{

    //チェイン攻撃から戻ってきたときにカメラが戻ってくるまでは止めておく
    change_normal_timer -= 1.0f * elapsed_time;

    ExecFuncTutorialUpdate(elapsed_time, sky_dome, enemies,graphics);
    //イベントシーンの黒の枠
    wipe_parm = Math::clamp(wipe_parm, 0.0f, 0.15f);
    if (awaiking_event == false)
    {
        player_move_effec_r->set_position(effect_manager->get_effekseer_manager(), step_pos_r);
        player_move_effec_l->set_position(effect_manager->get_effekseer_manager(), step_pos_l);

        if (during_chain_attack())
        {
            if (is_chain_attack_aftertaste_timer < CHRONOSTASIS_TIME)
            {
                chronostasis_scope = Math::lerp(chronostasis_scope, 0.3f, 1 / CHRONOSTASIS_TIME * elapsed_time);
                chronostasis_saturation = Math::lerp(chronostasis_saturation, 0.8f, 1 / CHRONOSTASIS_TIME * elapsed_time);
            }
            if (is_chain_attack_aftertaste_timer > (BaseCamera::AttackEndCameraTimer + AddAttackEndCameraTimer) - CHRONOSTASIS_TIME)
            {
                chronostasis_scope = Math::lerp(chronostasis_scope, 0.8f, 1 / CHRONOSTASIS_TIME * elapsed_time);
                chronostasis_saturation = Math::lerp(chronostasis_saturation, 1.0f, 1 / CHRONOSTASIS_TIME * elapsed_time);
            }

            PostEffect::chronostasis_effect(chronostasis_scope, chronostasis_saturation);
        }
        // ロックオン完了から攻撃終了後カメラが追いついたあとちょっと待ってtrue
        if (during_chain_attack() && !during_chain_attack_end())
        {
            is_chain_attack_aftertaste_timer += elapsed_time;
            if (is_chain_attack_aftertaste_timer > BaseCamera::AttackEndCameraTimer + AddAttackEndCameraTimer)
            {
                chronostasis_scope = 0.8f;
                chronostasis_saturation = 1.0f;

                is_chain_attack_aftertaste = false;
                is_chain_attack_aftertaste_timer = 0;
                PostEffect::clear_post_effect();
            }
        }

        if (avoidance_buttun)
        {
            if (game_pad->get_trigger_R() < 0.1f && !(game_pad->get_button() & GamePad::BTN_RIGHT_SHOULDER))
            {
                avoidance_buttun = false;
            }
        }
        //回り込み回避のリキャストの音
        if (behaind_avoidance_recharge == true && behaind_avoidance_cool_time < 0.0f)
        {
            audio_manager->play_se(SE_INDEX::BEHAIND_RECHARGE);
            behaind_avoidance_recharge = false;
        }
        if (behaind_avoidance_recharge == false && behaind_avoidance_cool_time < -1.0f) audio_manager->stop_se(SE_INDEX::BEHAIND_RECHARGE);

        switch (behavior_state)
        {
        case Player::Behavior::Normal:
            if (is_behind_avoidance == false)
            {
                //回り込み回避よりも進んでいたら切り替えれる
                if (tutorial_state > TutorialState::BehindAvoidanceTutorial)
                {
                    if (change_normal_timer < 0 && game_pad->get_button_down() & GamePad::BTN_LEFT_SHOULDER)
                    {
                        transition_chain_behavior();
                    }
                }
                //チュートリアルがロックオンの時よりも大きければ出来る
                if (tutorial_state >= TutorialState::LockOnTutorial)     TutorialLockOn();
                //カメラリセット
                CameraReset();
                if (is_behind_avoidance == false)
                {
                    PlayerJustification(elapsed_time, position);
                    if (target_enemy != nullptr)
                    {
                        PlayerEnemyJustification(elapsed_time, position, 1.6f, target_enemy->fGetPosition(), target_enemy->fGetBodyCapsule().mRadius);
                    }
                }
            }
            break;
        case Player::Behavior::Chain:
            break;
        default:
            break;
        }

        //アニメーション更新処理
        GetPlayerDirections();
        //プレイヤーのパラメータの変更
        TutorialInflectionParameters(elapsed_time);


        if (is_awakening)
        {
            for (int i = 0; i < 2; ++i)
            {
                mSwordTrail[i].fUpdate(elapsed_time, 10);
                mSwordTrail[i].fEraseTrailPoint(elapsed_time);
            }
        }
        else
        {
            mSwordTrail[0].fUpdate(elapsed_time, 10);
            mSwordTrail[0].fEraseTrailPoint(elapsed_time);
        }
        LerpCameraTarget(elapsed_time);
        player_config->update(graphics, elapsed_time);
        player_condition->update(graphics, elapsed_time);

        if (is_dying_update == false)
        {
            //覚醒状態の時は
            if (is_awakening)
            {
                //モデルを映す
                if (threshold_mesh > 0) threshold_mesh -= 2.0f * elapsed_time;
            }
            else
            {
                //モデルを消す
                if (threshold_mesh < 1) threshold_mesh += 2.0f * elapsed_time;
            }
        }
    }
        threshold_mesh = Math::clamp(threshold_mesh, 0.0f, 1.0f);
    if(is_update_animation)model->update_animation(elapsed_time * animation_speed);
#ifdef USE_IMGUI
    static bool display_scape_imgui;
    imgui_menu_bar("Player", "Player", display_scape_imgui);
    if (display_scape_imgui)
    {
        if (ImGui::Begin("Player"))
        {
            ImGui::Text("time%.2f", model->get_anim_para().animation_tick);

            if (ImGui::TreeNode("transform"))
            {
                ImGui::DragFloat3("position", &position.x);
                ImGui::DragFloat3("scale", &scale.x, 0.001f);
                ImGui::DragFloat4("orientation", &orientation.x);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("PlayerDirection"))
            {
                ImGui::DragFloat3("forward", &forward.x);
                ImGui::DragFloat3("right", &right.x);
                ImGui::DragFloat3("up", &up.x);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("speed"))
            {
                ImGui::DragFloat3("velocity", &velocity.x);
                ImGui::DragFloat3("acceleration_velocity", &acceleration_velocity.x);
                ImGui::DragFloat("max_speed", &move_speed);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("PlayerFlags"))
            {
                ImGui::Checkbox("is_avoidance", &is_avoidance);
                ImGui::Checkbox("is_behind_avoidance", &is_behind_avoidance);
                ImGui::Checkbox("camera_reset", &camera_reset);
                ImGui::Checkbox("is_lock_on", &is_lock_on);
                ImGui::Checkbox("is_camera_lock_on", &is_camera_lock_on);
                ImGui::Checkbox("is_enemy_hit", &is_enemy_hit);
                ImGui::Checkbox("is_awakening", &is_awakening);
                ImGui::Checkbox("start_dash_effect", &start_dash_effect);
                ImGui::Checkbox("end_dash_effect", &end_dash_effect);
                ImGui::Checkbox("is_attack", &is_attack);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("CapsuleParam"))
            {
                if (ImGui::TreeNode("BodyCapsuleParam"))
                {
                    ImGui::DragFloat3("capsule_parm.start", &body_capsule_param.start.x, 0.1f);
                    ImGui::DragFloat3("capsule_parm.end", &body_capsule_param.end.x, 0.1f);
                    ImGui::DragFloat("body_capsule_param.rasius", &body_capsule_param.rasius, 0.1f);
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("easing"))
            {
                ImGui::DragFloat("avoidance_easing_time", &avoidance_easing_time, 0.1f);
                ImGui::DragFloat("avoidance_boost_time", &avoidance_boost_time, 0.1f);
                ImGui::DragFloat("leverage", &leverage, 0.1f);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("PlayerGameParm"))
            {
                ImGui::DragInt("player_health", &player_health);
                ImGui::DragFloat("combo", &combo_count);
                ImGui::DragFloat("attack_time", &attack_time);

                ImGui::DragFloat("duration_combo_timer", &duration_combo_timer);
                ImGui::DragInt("player_attack_power", &player_attack_power);
                ImGui::DragFloat("invincible_timer", &invincible_timer);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Effect"))
            {
                ImGui::DragFloat("air_registance_offset_y", &air_registance_offset_y);
                ImGui::TreePop();
            }

            if (ImGui::Button("TransitionStageMove")) TransitionStageMove();
            if (ImGui::Button("TransitionIdle")) TransitionIdle();

            ImGui::DragFloat("threshold", &threshold, 0.01f, 0, 1.0f);
            ImGui::DragFloat("threshold_mesh", &threshold_mesh, 0.01f, 0, 1.0f);
            ImGui::DragFloat("glow_time", &glow_time);
            ImGui::DragFloat4("emissive_color", &emissive_color.x, 0.1f);
            DirectX::XMFLOAT3 p{ position.x,position.y + step_offset_y,position.z };
            float length_radius = Math::calc_vector_AtoB_length(p, target);//距離(半径)
            ImGui::DragFloat("l", &length_radius);

            static int state = 1;
            ImGui::SliderInt("tutorial_state", &state, 1, 7);
            tutorial_state = static_cast<TutorialState>(state);

            ImGui::DragFloat("execution_timer", &execution_timer);
            ImGui::Checkbox("is_next", &is_next_tutorial);
            ImGui::DragFloat3("slash_effec_pos", &slash_effec_pos.x,0.1f);
            ImGui::Text("anime_time%d",model->get_anim_para().frame_index);
            ImGui::End();
        }
    }
#endif // USE_IMGUI

}


void Player::ExecFuncTutorialUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
    switch (behavior_state)
    {
    case Player::Behavior::Normal:
        //自分のクラスの関数ポインタを呼ぶ
        (this->*player_tutorial_activity)(elapsed_time, sky_dome, enemies);
        break;
    case Player::Behavior::Chain:
        //自分のクラスの関数ポインタを呼ぶ
        if (during_chain_attack() == false) (this->*chain_activity)(elapsed_time, sky_dome);
        (this->*player_chain_activity)(elapsed_time, enemies,Graphics_);
        break;
    default:
        break;
    }

}

void Player::ChangeTutorialState(int state)
{
    //もし今のチュートリアルステートよりも小さい値が入ってきたら
    //ひとつ前にもどるからはいれないようにする
    if (tutorial_state > static_cast<TutorialState>(state) || tutorial_state == static_cast<TutorialState>(state)) return;
    tutorial_state = static_cast<TutorialState>(state);
    is_next_tutorial = false;
    execution_timer = 0;
}

void Player::TutorialIdleUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    //移動に遷移
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
    {
        TransitionTutorialMove();
    }
    else if (during_chain_attack() && change_normal_timer > 0 && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
    {
        TransitionTutorialMove();
    }

    if (change_normal_timer < 0 && behavior_state == Behavior::Normal)
    {
        switch (tutorial_state)
        {
        case Player::TutorialState::MoveTutorial:
            //MoveTutorialの時にこの時間動いていたら終わったことにする
            if (execution_timer > 1.5f) is_next_tutorial = true;
            break;
        case Player::TutorialState::AvoidanceTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                TransitionTutorialAvoidance();
            }
            break;
        }
        case Player::TutorialState::LockOnTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                TransitionTutorialAvoidance();
            }
            break;
        }
        case Player::TutorialState::AttackTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                TransitionTutorialAvoidance();
            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            break;
        }
        case Player::TutorialState::BehindAvoidanceTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }
            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            break;
        }
        case Player::TutorialState::ChainAttackTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }
            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }

            break;
        }
        case Player::TutorialState::AwaikingTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }

            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            TutorialAwaiking();
            break;
        }
        case Player::TutorialState::FreePractice:
        {
            is_next_tutorial = false;
            execution_timer = 0;

            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }

            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            TutorialAwaiking();
            break;
        }

        break;
        default:
            break;
        }
    }
    UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::TutorialMoveUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    player_move_effec_r->set_position(effect_manager->get_effekseer_manager(), step_pos_r);
    player_move_effec_l->set_position(effect_manager->get_effekseer_manager(), step_pos_l);

    //移動入力がなくなったら待機に遷移
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionTutoriaIdle();
    }
    else if (during_chain_attack() && change_normal_timer > 0 && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionTutoriaIdle();
    }

    if (change_normal_timer < 0 && behavior_state == Behavior::Normal)
    {
        switch (tutorial_state)
        {
        case Player::TutorialState::MoveTutorial:
            execution_timer += 1.0f * elapsed_time;
            //MoveTutorialの時にこの時間動いていたら終わったことにする
            if (execution_timer > 1.5f) is_next_tutorial = true;
            break;
        case Player::TutorialState::AvoidanceTutorial:
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                TransitionTutorialAvoidance();
            }
            break;
        case Player::TutorialState::LockOnTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                TransitionTutorialAvoidance();
            }
        }
        break;
        case Player::TutorialState::AttackTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                TransitionTutorialAvoidance();
            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            break;
        }
        case Player::TutorialState::BehindAvoidanceTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }

            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            break;
        }
        case Player::TutorialState::ChainAttackTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }

            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }

            break;
        }
        case Player::TutorialState::AwaikingTutorial:
        {
            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }
            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            TutorialAwaiking();
            break;
        }
        case Player::TutorialState::FreePractice:
        {
            is_next_tutorial = false;
            execution_timer = 0;

            if (avoidance_buttun == false && game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER)
            {
                //回避に遷移
                float length{ Math::calc_vector_AtoB_length(position, target) };
                if (is_lock_on && is_just_avoidance_capsul && length < BEHIND_LANGE_MAX)
                {
                    TransitionTutorialJustBehindAvoidance();
                }
                else
                {
                    if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                    {
                        //後ろに回り込める距離なら回り込みようのUpdate
                        TransitionTutorialBehindAvoidance();
                    }
                    //そうじゃなかったら普通の回避
                    else TransitionTutorialAvoidance();
                }

            }
            //突進開始に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                TransitionTutorialChargeInit();
            }
            TutorialAwaiking();
            break;
        }

        break;
        default:
            break;
        }
    }
    UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::TutorialAvoidanvceUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    //エフェクトの位置，回転設定
    player_air_registance_effec->set_position(effect_manager->get_effekseer_manager(), { position.x,position.y + air_registance_offset_y ,position.z });
    player_air_registance_effec->set_quaternion(effect_manager->get_effekseer_manager(), orientation);
    avoidance_boost_time += 1.0f * elapsed_time;
    //回避の時の加速
    SetAccelerationVelocity();
    //ロックオンしている敵と一定距離近くなったら
    float length{ Math::calc_vector_AtoB_length(position, target) };
    if (is_lock_on && length < 15.0f)
    {
        //攻撃に遷移
        velocity.x *= 0.2f;
        velocity.y *= 0.2f;
        velocity.z *= 0.2f;
        player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
        TransitionTutorialAttack1();
    }
    if (avoidance_boost_time > 1.0f)
    {
        model->progress_animation();
        if (model->end_of_animation())
        {
            if(tutorial_state == TutorialState::AvoidanceTutorial) is_next_tutorial = true;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
            //回避中かどうかの設定
            is_avoidance = false;
            is_behind_avoidance = false;
            //移動入力があったら移動に遷移
            if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
            {
                TransitionTutorialMove();
}
            //移動入力がなかったら待機に遷移
            else
            {
                TransitionTutoriaIdle();
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

#if 0
    AvoidanceAcceleration(elapsed_time);
    if (avoidance_boost_time > avoidance_easing_time && model->end_of_animation())
    {
        player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
        //もしチュートリアルが回避なら
        if (tutorial_state == TutorialState::AvoidanceTutorial) is_next_tutorial = true;
        //回避中かどうかの設定
        is_avoidance = false;
        is_behind_avoidance = false;
        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
        {
            TransitionTutorialMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            TransitionTutoriaIdle();
        }
        if (tutorial_state >= TutorialState::AwaikingTutorial)   TutorialAwaiking();
        UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
    }
    else
    {
        UpdateAvoidanceVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
    }

#endif // 0
}

void Player::TutorialBehindAvoidanceUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    player_behaind_effec_2->set_position(effect_manager->get_effekseer_manager(), position);
    player_behaind_effec_2->set_quaternion(effect_manager->get_effekseer_manager(), orientation);

    player_behind_effec->set_position(effect_manager->get_effekseer_manager(), position);
    just_stun->set_position(effect_manager->get_effekseer_manager(), target);

    if (BehindAvoidanceMove(elapsed_time, behind_transit_index, position, behind_speed, behind_interpolated_way_points, 1.5f))
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
        is_just_avoidance = false;
        //もしチュートリアルが回り込み回避なら
        if (tutorial_state == TutorialState::BehindAvoidanceTutorial)
        {
            tutorial_action_count--;
            if(tutorial_action_count <= 0)is_next_tutorial = true;
        }
        TransitionTutoriaIdle();
    }
    else
    {
        is_lock_on = true;
    }
    UpdateBehindAvoidanceVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::TutorialChargeinitUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (model->end_of_animation())
    {
        TransitionTutorialCharge(attack_animation_blends_speeds.x);
    }
    //ChargeAcceleration(elapsed_time);
    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::TutorialChargeUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    //エフェクトの位置，回転設定
    player_air_registance_effec->set_position(effect_manager->get_effekseer_manager(), position);
    player_air_registance_effec->set_quaternion(effect_manager->get_effekseer_manager(), orientation);
    start_dash_effect = false;
    charge_time += charge_add_time * elapsed_time;
    //ChargeAcceleration(elapsed_time);
    SetAccelerationVelocity();
    //突進時間を超えたらそれぞれの遷移にとぶ
    if (charge_time > CHARGE_MAX_TIME)
    {
        player_air_registance_effec->stop(effect_manager->get_effekseer_manager());

        audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
        PostEffect::clear_post_effect();
        velocity.x *= 0.2f;
        velocity.y *= 0.2f;
        velocity.z *= 0.2f;
        end_dash_effect = true;

        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
        {
            charge_change_direction_count = CHARGE_DIRECTION_COUNT;
            charge_time = 0;
            is_charge = false;
            TransitionTutorialMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            charge_change_direction_count = CHARGE_DIRECTION_COUNT;
            charge_time = 0;
            is_charge = false;
            TransitionTutoriaIdle();
        }
        if (tutorial_state >= TutorialState::AwaikingTutorial)   TutorialAwaiking();

    }
    else
    {
        if (is_enemy_hit)
        {
            player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
            PostEffect::clear_post_effect();
            end_dash_effect = true;
            //敵に当たって攻撃ボタン(突進ボタン)を押したら一撃目
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            is_enemy_hit = false;
            is_attack = false;
            charge_change_direction_count = CHARGE_DIRECTION_COUNT;
            //チュートリアルが攻撃なら
            if (tutorial_state == TutorialState::AttackTutorial) is_next_tutorial = true;
            TransitionTutorialAttack1(attack_animation_blends_speeds.y);
        }
        if (is_lock_on == false && charge_change_direction_count > 0)
        {
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
                audio_manager->play_se(SE_INDEX::PLAYER_RUSH);
                player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
                //エフェクト再生
                player_air_registance_effec->play(effect_manager->get_effekseer_manager(), position, 0.3f);
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
                    //DirectX::XMVECTOR ve{ DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&velocity) )};
                    //DirectX::XMStoreFloat3(&movevec, ve);
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

void Player::TutorialAttack1Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (model->end_of_animation())
    {

        is_attack = false;
        attack_time += attack_add_time * elapsed_time;
        //猶予時間を超えたら待機に遷移
        if (attack_time > ATTACK_TYPE1_MAX_TIME)
        {
            attack_time = 0;
            TransitionTutoriaIdle();
        }
        else
        {

            //猶予時間よりも早く押したら攻撃2撃目に遷移
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                if (target_enemy != nullptr && target_enemy->fGetPercentHitPoint() != 0)
                {
                    attack_time = 0;
                    TransitionTutorialAttack2(attack_animation_blends_speeds.z);
                }
            }
        }
        //else TransitionTutoriaIdle();
    }
    if (is_awakening)
    {
        mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
        mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
    }
    else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);

    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);

}

void Player::TutorialAttack2Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
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
            is_enemy_hit = false;
            is_update_animation = true;
        }
        if (attack_time >= 0.6f)
        {
            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            attack_time = 0;
            TransitionTutoriaIdle();
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
            TransitionTutoriaIdle();
        }
        else
        {
            if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
            {
                if (target_enemy != nullptr && target_enemy->fGetPercentHitPoint() != 0)
                {
                    velocity.x *= 0.2f;
                    velocity.y *= 0.2f;
                    velocity.z *= 0.2f;
                    attack_time = 0;
                    TransitionTutorialAttack3(attack_animation_blends_speeds.w);
                }
            }
        }
        //猶予時間よりも早く押したら攻撃3撃目に遷移
        //else
        //{
        //    velocity.x *= 0.2f;
        //    velocity.y *= 0.2f;
        //    velocity.z *= 0.2f;
        //    TransitionTutoriaIdle();
        //}
    }
    if (is_awakening)
    {
        mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);
        mSwordTrail[1].fAddTrailPoint(sword_capsule_param[1].start, sword_capsule_param[1].end);
    }
    else mSwordTrail[0].fAddTrailPoint(sword_capsule_param[0].start, sword_capsule_param[0].end);

    UpdateAttackVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);

}

void Player::TutorialAttack3Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
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
            is_enemy_hit = false;
            attack_time = 0;
            is_update_animation = true;
        }
        if (attack_time >= 0.6f)
        {
            audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
            is_charge = false;
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            attack_time = 0;
            TransitionTutoriaIdle();
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
                charge_time = 0;
                velocity.x *= 0.2f;
                velocity.y *= 0.2f;
                velocity.z *= 0.2f;

                TransitionTutorialMove();
            }
            //移動入力がなかったら待機に遷移
            else
            {
                velocity.x *= 0.2f;
                velocity.y *= 0.2f;
                velocity.z *= 0.2f;
                charge_time = 0;
                TransitionTutoriaIdle();
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
                    TransitionTutorialCharge(attack_animation_blends_speeds.z);
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

void Player::TutorialAwaikingUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (model->end_of_animation())
    {
        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
        {
            TransitionTutorialMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            TransitionTutoriaIdle();
        }
    }
}

void Player::TutorialInvAwaikingUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (model->end_of_animation())
    {
        //移動入力があったら移動に遷移
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > 0)
        {
            TransitionTutorialMove();
        }
        //移動入力がなかったら待機に遷移
        else
        {
            TransitionTutoriaIdle();
        }
    }
}

void Player::TutorialDamageUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (model->end_of_animation())
    {
        TransitionTutoriaIdle();
    }
}

void Player::TutorialAwaikingEventUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    //if (awaiking_se == false && model->get_anim_para().animation_tick > 8.1f)
    //{
    //    awaiking_se = true;
    //    audio_manager->play_se(SE_INDEX::PLAYER_AWAKING);
    //}


    if (model->get_anim_para().animation_tick >= 0.66f && awaiking_event_state == 0) {
        audio_manager->play_se(SE_INDEX::DOCKING_2);
        awaiking_event_state = 1;
    }

    if (model->get_anim_para().animation_tick >= 1.0f && awaiking_event_state == 1) {
        audio_manager->play_se(SE_INDEX::DOCKING_2);
        awaiking_event_state = 2;
    }

    if (model->get_anim_para().animation_tick >= 1.33f && awaiking_event_state == 2) {
        audio_manager->play_se(SE_INDEX::DOCKING_2);
        awaiking_event_state = 3;
    }

    if (model->get_anim_para().animation_tick  >= 1.75f  &&awaiking_event_state == 3) {
        audio_manager->play_se(SE_INDEX::FOOT_TRANSFORM);
        awaiking_event_state = 4;
    }
    if (model->get_anim_para().animation_tick >= 2.88f && awaiking_event_state == 4) {
        audio_manager->play_se(SE_INDEX::SHOULDER_ARMOR);
        awaiking_event_state = 5;
    }
    if (model->get_anim_para().animation_tick >= 6.31f && awaiking_event_state == 5) {
        audio_manager->play_se(SE_INDEX::GRAB);
        awaiking_event_state = 6;
    }
    if (model->get_anim_para().animation_tick >= 8.08f && awaiking_event_state == 6) {
        audio_manager->play_se(SE_INDEX::SABER);
        awaiking_event_state = 7;
    }

    //非表示にしてるものは表示
    threshold_mesh -= 0.7f * elapsed_time;
    //カメラに渡す値を設定
    DirectX::XMFLOAT3 up = {};
    model->fech_by_bone(Math::calc_world_matrix(scale, orientation, position), player_bones[8], event_camera_joint, up);
    model->fech_by_bone(Math::calc_world_matrix(scale, orientation, position), player_bones[9], event_camera_eye, up);
    if (model->end_of_animation())
    {
        //アニメーションが終わったら遷移
        TransitionTutorialAwaikingEventIdle();
    }
}

void Player::TutorialAwaikingEventIdleUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    combo_count = MAX_COMBO_COUNT;
    //非表示にしてるものは表示
    threshold_mesh -= 0.5f * elapsed_time;
    if (model->end_of_animation())
    {
        if (awaiking_event)
        {
            //イベント中はワイプの黒枠を消す
            wipe_parm -= 0.2f * elapsed_time;
            PostEffect::wipe_effect(wipe_parm);
        }
        if (wipe_parm < 0)
        {
            //ワイプが0になったらイベント終了
            PostEffect::clear_post_effect();
            wipe_parm = 0.0f;
            awaiking_event = false;
            //もしチュートリアルが覚醒なら
            if (tutorial_state == TutorialState::AwaikingTutorial) is_next_tutorial = true;
            TransitionTutoriaIdle();
        }
    }
}

void Player::TransitionTutoriaIdle(float blend_second)
{

    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //覚醒状態の時の待機アニメーションにセット
    if (is_awakening)model->play_animation(AnimationClips::AwakingIdle, true, true, blend_second);
    //通常状態の待機アニメーションにセット
    else model->play_animation(AnimationClips::Idle, true, true, blend_second);
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //攻撃中かどうかの設定
    is_attack = false;
    player_tutorial_activity = &Player::TutorialIdleUpdate;
}

void Player::TransitionTutorialMove(float blend_second)
{

    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //エフェクト再生
    player_move_effec_r->play(effect_manager->get_effekseer_manager(), step_pos_r);
    player_move_effec_l->play(effect_manager->get_effekseer_manager(), step_pos_l);
    //覚醒状態の時の移動アニメーションの設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingMove, true, true, blend_second);
    //通常状態の時に移動アニメーションの設定
    else model->play_animation(AnimationClips::Move, true, true, blend_second);
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //攻撃中かどうかの設定
    is_attack = false;
    player_tutorial_activity = &Player::TutorialMoveUpdate;
}

void Player::TransitionTutorialAvoidance(float blend_second)
{
    //player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    audio_manager->play_se(SE_INDEX::AVOIDANCE);
    //エフェクト再生
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
    player_air_registance_effec->play(effect_manager->get_effekseer_manager(), { position.x,position.y + air_registance_offset_y ,position.z },0.3f);
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
    if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
    //通常状態の時のアニメーションの設定
    else model->play_animation(AnimationClips::Avoidance, false, true);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = AVOIDANCE_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //回避状態の時の更新関数に切り替える
    player_tutorial_activity = &Player::TutorialAvoidanvceUpdate;

}

void Player::TransitionTutorialBehindAvoidance()
{
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_behaind_effec_2->play(effect_manager->get_effekseer_manager(), position, 2.0f);

    audio_manager->play_se(SE_INDEX::WRAPAROUND_AVOIDANCE);
    if (target_enemy != nullptr)
    {
        //ロックオンしている敵をスタンさせる
        target_enemy->fSetStun(true);
    }
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
    player_tutorial_activity = &Player::TutorialBehindAvoidanceUpdate;;
}

void Player::TransitionTutorialJustBehindAvoidance()
{
    audio_manager->play_se(SE_INDEX::WRAPAROUND_AVOIDANCE);
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_behaind_effec_2->play(effect_manager->get_effekseer_manager(), position, 2.0f);

        //ロックオンしている敵をスタンさせる
    if (target_enemy != nullptr)
    {
        target_enemy->fSetStun(true, true);
    }
    DirectX::XMFLOAT3 pos{};
        pos = Math::calc_designated_point(position, forward, 5.0f);
    if (is_awakening)
    {
        just_stun->play(effect_manager->get_effekseer_manager(), target, 6.0f);
    }
    else
    {
        just_stun->play(effect_manager->get_effekseer_manager(), target, 3.0f);
    }
    is_just_avoidance = true;
    //HP回復する
    player_health += JUST_AVOIDANCE_HEALTH;
    //コンボゲージ増やす
    combo_count += JUST_AVOIDANCE_COMBO;
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
    player_tutorial_activity = &Player::TutorialBehindAvoidanceUpdate;;
}

void Player::TransitionTutorialChargeInit()
{
    //player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    //覚醒状態の時の突進の始まりのアニメーションに設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingChargeInit, false, true);
    //通常状態の時の突進の始まりのアニメーションに設定
    else model->play_animation(AnimationClips::ChargeInit, false, true);
    //攻撃中かどうかの設定
    is_attack = true;
    //突進中かどうかの設定
    is_charge = true;
    //アニメーション速度の設定
    animation_speed = CHARGEINIT_ANIMATION_SPEED;
    //ロックオンしてない場合のターゲットの設定
    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //攻撃の加速の設定
    //SetAccelerationVelocity();
    //加速のレート
    lerp_rate = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //突進の始まりの時の更新関数に切り替える
    player_tutorial_activity = &Player::TutorialChargeinitUpdate;

}

void Player::TransitionTutorialCharge(float blend_second)
{
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
        model->play_animation(AnimationClips::AwakingCharge, false, true, blend_second);
    }
    //通常状態の時の突進アニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = CHARGE_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::Charge, false, true, blend_second);
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
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //加速のレート
    lerp_rate = 4.0f;
    //突進中の更新関数に切り替える
    player_tutorial_activity = &Player::TutorialChargeUpdate;
}

void Player::TransitionTutorialAttack1(float blend_second)
{
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
    //覚醒状態の時の１撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType1, false, true, blend_second);
    }
    //通常状態の時の１撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType1, false, true, blend_second);
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
    player_tutorial_activity = &Player::TutorialAttack1Update;
}

void Player::TransitionTutorialAttack2(float blend_second)
{
    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);

    //覚醒状態の時の２撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType2, false, true, blend_second);
    }
    //通常状態の時の２撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType2, false, true, blend_second);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーション速度の設定
    animation_speed = ATTACK2_ANIMATION_SPEED;
    //攻撃の加速の設定
    //SetAccelerationVelocity();
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
    player_tutorial_activity = &Player::TutorialAttack2Update;
}

void Player::TransitionTutorialAttack3(float blend_second)
{
    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);

    //覚醒状態の時の３撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType3, false, true, blend_second);
    }
    //通常状態の時の３撃目ののアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType3, false, true, blend_second);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーション速度の設定
    animation_speed = ATTACK3_ANIMATION_SPEED;
    //攻撃の加速の設定
    //SetAccelerationVelocity();
    //加速のレート
    lerp_rate = 2.0f;
    //攻撃の時間
    attack_time = 0;
    //アニメーションをしていいかどうか
    is_update_animation = false;
    //突進中かどうかの設定
    is_charge = true;
    //３撃目の更新関数に切り替える
    player_tutorial_activity = &Player::TutorialAttack3Update;

}

void Player::TransitionTutorialAwaiking()
{
    invincible_timer = 2.0f;
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());

    player_awaiking_effec->play(effect_manager->get_effekseer_manager(), position,2.0f);
    invincible_timer = 2.0f;
    //覚醒状態になるアニメーションに設定
    model->play_animation(AnimationClips::Awaking, false, true);
    //覚醒状態かどうかの設定
    is_awakening = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //覚醒状態になる途中の更新関数に切り替える
    player_tutorial_activity = &Player::TutorialAwaikingUpdate;
}

void Player::TransitionTutorialInvAwaiking()
{
    invincible_timer = 2.0f;
    //通常状態に戻るアニメーションに設定
    model->play_animation(AnimationClips::InvAwaking, false, true);
    //覚醒状態かどうかの設定
    is_awakening = false;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //通常状態に戻ってるときの更新関数に切り替える
    player_tutorial_activity = &Player::TutorialInvAwaikingUpdate;
}

void Player::TransitionTutorialAwaikingEvent()
{
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    PostEffect::clear_post_effect();
    is_lock_on = false;
    DirectX::XMFLOAT3 up = {};
    model->fech_by_bone(Math::calc_world_matrix(scale, orientation, position), player_bones[8], event_camera_joint, up);
    model->fech_by_bone(Math::calc_world_matrix(scale, orientation, position), player_bones[9], event_camera_eye, up);
    //プレイヤーの位置は原点に移動
    position = { 0,0,0 };
    ////非表示にしてるものは表示
    //threshold_mesh = 0.0f;
    //覚醒状態になるアニメーションに設定
    model->play_animation(AnimationClips::AwaikingScene, false, true);
    //覚醒状態かどうかの設定
    is_awakening = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //イベントカメラに切り替え
    awaiking_event = true;
    awaiking_se = false;
    //覚醒状態になる途中の更新関数に切り替える
    player_tutorial_activity = &Player::TutorialAwaikingEventUpdate;
}

void Player::TransitionTutorialAwaikingEventIdle()
{
    DirectX::XMFLOAT3 up = {};
    model->fech_by_bone(Math::calc_world_matrix(scale, orientation, position), player_bones[8], event_camera_joint, up);
    model->fech_by_bone(Math::calc_world_matrix(scale, orientation, position), player_bones[9], event_camera_eye, up);
    //覚醒状態になるアニメーションに設定
    model->play_animation(AnimationClips::AwaikingSceneIdle, false, true);
    //覚醒状態かどうかの設定
    is_awakening = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //覚醒状態になる途中の更新関数に切り替える
    player_tutorial_activity = &Player::TutorialAwaikingEventIdleUpdate;

}

void Player::TransitionTutorialDamage()
{
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
    player_tutorial_activity = &Player::TutorialDamageUpdate;

}

void Player::TutorialAwaiking()
{
    //チュートリアルの覚醒はイベントシーンになる
    if (tutorial_state == TutorialState::AwaikingTutorial)
    {
        //ボタン入力
        if (game_pad->get_button() & GamePad::BTN_A)
        {
            if (tutorial_awaiking == false && combo_count >= MAX_COMBO_COUNT)
            {
                tutorial_awaiking = true;
                TransitionTutorialAwaikingEvent();//イベントの覚醒
            }
        }
        if (is_awakening && combo_count <= 0)
        {
            //覚醒状態かどうかの設定
            is_awakening = false;
            TransitionTutorialInvAwaiking();
        }
    }
    else
    {
        if (behavior_state == Behavior::Normal)
        {
            //ボタン入力
            if (game_pad->get_button() & GamePad::BTN_A)
            {
                if (combo_count >= MAX_COMBO_COUNT - 5.0f)TransitionTutorialAwaiking();//コンボカウントが最大のときは覚醒状態になる
            }
            if (is_awakening && combo_count <= 0)
            {
                //覚醒状態かどうかの設定
                is_awakening = false;
                TransitionTutorialInvAwaiking();//覚醒状態のときにカウントが0になったら通常状態になる
            }
        }
    }
}

void Player::SetTutorialDamageFunc()
{
    damage_func = [=](int damage, float invincible)->void {TutorialDamagedCheck(damage, invincible); };
}
