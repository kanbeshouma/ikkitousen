#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"Player.h"
#include<map>

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
    //���f���̃A�j���[�V�������I�����������1�b��������
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
    //������update���Ă����Ȃ���1�t���[�����������̃A�j���[�V�������f���Ă��܂�����
    model->update_animation(1.0f);
    //�^�C�g���̓��b�V�����B���Ȃ�����0�ɂ��Ă���
    threshold_mesh = 0.0f;
}

void Player::ExecFuncUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
    switch (behavior_state)
    {
    case Player::Behavior::Normal:
        //�����̃N���X�̊֐��|�C���^���Ă�
        (this->*player_activity)(elapsed_time, sky_dome);
        break;
    case Player::Behavior::Chain:
        //�����̃N���X�̊֐��|�C���^���Ă�
        if(during_chain_attack() == false)(this->*chain_activity)(elapsed_time, sky_dome);
        (this->*player_chain_activity)(elapsed_time, enemies,Graphics_);
        break;
    default:
        break;
    }
}

void Player::IdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    //�ړ��ɑJ��
    //�`�F�C���U���̃��b�N�I����������U���I���̎��͑���͎󂯕t���Ȃ�
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
    {
        TransitionMove();
    }
    else if (during_chain_attack() && change_normal_timer > 0 && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
    {
        TransitionMove();
    }

    //�`�F�C���U������߂��Ă��Đ��b�Ԃ͈ړ������ł��Ȃ�
    //�`�F�C���U���̏�Ԃł͈ړ��ȊO�̑���͎󂯕t���Ȃ�
    if (change_normal_timer < 0 && behavior_state == Behavior::Normal)
    {
        //����ɑJ��
        float length{ Math::calc_vector_AtoB_length(position, target) };
        if (avoidance_buttun == false && (game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER))
        {
            //�W���X�g����Ȃ�
            if (is_lock_on && is_just_avoidance_capsul)
            {
                TransitionJustBehindAvoidance();
            }
            else
            {
                //���ɉ�荞�߂鋗���Ȃ��荞�݂悤��Update
                if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                {
                    TransitionBehindAvoidance();
                }
                //��������Ȃ������畁�ʂ̉��
                else
                {
                    TransitionAvoidance();

                }
            }
        }
        //�ːi�J�n�ɑJ��
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

    //�ړ����͂��Ȃ��Ȃ�����ҋ@�ɑJ��
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionIdle();
    }
    else if (during_chain_attack() && change_normal_timer > 0 && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionIdle();
    }
    //�`�F�C���U������߂��Ă��Đ��b�Ԃ͈ړ������ł��Ȃ�
    //�`�F�C���U���̏�Ԃł͈ړ��ȊO�̑���͎󂯕t���Ȃ�
    if (change_normal_timer < 0 && behavior_state == Behavior::Normal)
    {
        //����ɑJ��
        float length{ Math::calc_vector_AtoB_length(position, target) };
        if (avoidance_buttun == false && (game_pad->get_trigger_R() || game_pad->get_button_down() & GamePad::BTN_RIGHT_SHOULDER))
        {
            //�W���X�g����Ȃ�
            if (is_lock_on && is_just_avoidance_capsul)
            {
                TransitionJustBehindAvoidance();
            }
            else
            {
                //���ɉ�荞�߂鋗���Ȃ��荞�݂悤��Update
                if (behaind_avoidance_cool_time < 0 && is_lock_on && length < BEHIND_LANGE_MAX && length > BEHIND_LANGE_MIN)
                {
                    TransitionBehindAvoidance();
                }
                //��������Ȃ������畁�ʂ̉��
                else
                {
                    TransitionAvoidance();
                }
            }
        }
        //�ːi�J�n�ɑJ��
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
    //�G�t�F�N�g�̈ʒu�C��]�ݒ�
    player_air_registance_effec->set_position(effect_manager->get_effekseer_manager(),position);
    player_air_registance_effec->set_quaternion(effect_manager->get_effekseer_manager(), orientation);
    avoidance_boost_time += 1.0f * elapsed_time;
    //����̎��̉���
    SetAccelerationVelocity();

    //-----�U���{�^������������U���ɑJ��-----//
    if (game_pad->get_button_down() & GamePad::BTN_ATTACK_B)
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
        if (model->end_of_animation())
        {
            velocity.x *= 0.2f;
            velocity.y *= 0.2f;
            velocity.z *= 0.2f;
            //��𒆂��ǂ����̐ݒ�
            is_avoidance = false;
            is_behind_avoidance = false;
            //�ړ����͂���������ړ��ɑJ��
            if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
            {
                player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
                TransitionMove();
            }
            //�ړ����͂��Ȃ�������ҋ@�ɑJ��
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
        //�A������̉񐔂�0���傫���Ƃ���
        if (avoidance_direction_count > 0)
        {
            //����{�^��������������͕����ɕ����]��
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
                //�o����Ԃ̎��̉���A�j���[�V�����̐ݒ�
                if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
                //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
                else model->play_animation(AnimationClips::Avoidance, false, true);
                avoidance_boost_time = 0.0f;

                //-----�f�[�^���M-----//
                SendPlayerActionData(GamePad::BTN_RIGHT_SHOULDER, movevec);
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
        //��𒆂��ǂ����̐ݒ�
        is_avoidance = false;
        is_behind_avoidance = false;
        //�W���X�g����̃t���O��������
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
    //�G�t�F�N�g�̈ʒu�C��]�ݒ�
    player_air_registance_effec->set_position(effect_manager->get_effekseer_manager(), position);
    player_air_registance_effec->set_quaternion(effect_manager->get_effekseer_manager(), orientation);
    start_dash_effect = false;
    charge_time += charge_add_time * elapsed_time;
    //ChargeAcceleration(elapsed_time);
    //�U���̉����̐ݒ�
    SetAccelerationVelocity();
    //�u���b�N����Ă����猕���ӂ��ċ���
    if (is_block)
    {
        TransitionAttackType1(attack_animation_blends_speeds.y);
    }
    //�ːi���Ԃ𒴂����炻�ꂼ��̑J�ڂɂƂ�
    if (charge_time > CHARGE_MAX_TIME)
    {

        audio_manager->stop_se(SE_INDEX::PLAYER_RUSH);
        PostEffect::clear_post_effect();
        velocity.x *= 0.2f;
        velocity.y *= 0.2f;
        velocity.z *= 0.2f;

        //�ړ����͂���������ړ��ɑJ��
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
        {
            charge_time = 0;
            is_charge = false;
            player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
            TransitionMove();
        }
        //�ړ����͂��Ȃ�������ҋ@�ɑJ��
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
            //�G�ɓ������čU���{�^��(�ːi�{�^��)����������ꌂ��
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
                //�G�t�F�N�g�Đ�
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

                //-----�f�[�^���M-----//
                SendPlayerActionData(GamePad::BTN_ATTACK_B, movevec);
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
            //�u���b�N���ꂽ���ǂ���
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
            //�u���b�N���ꂽ���ǂ���
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
        //�P�\���Ԃ𒴂�����ҋ@�ɑJ��
        if (attack_time > ATTACK_TYPE1_MAX_TIME)
        {
            attack_time = 0;
            TransitionIdle();
        }
        else
        {
            //�P�\���Ԃ���������������U��2���ڂɑJ��
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
            //�u���b�N���ꂽ���ǂ���
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
            //�u���b�N���ꂽ���ǂ���
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
        //�P�\���Ԃ𒴂�����ҋ@�ɑJ��
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
            //�P�\���Ԃ���������������U��3���ڂɑJ��
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
            //�u���b�N���ꂽ���ǂ���
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
            //�u���b�N���ꂽ���ǂ���
            if (is_block)
            {
                is_block = false;
                TransitionDamage();
            }
        }
    }

    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //�G�ɓ������������Ԃ�2�b��������������I���

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
            //�ړ����͂���������ړ��ɑJ��
            if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
            {
                velocity.x *= 0.2f;
                velocity.y *= 0.2f;
                velocity.z *= 0.2f;
                charge_time = 0;
                TransitionMove();
            }
            //�ړ����͂��Ȃ�������ҋ@�ɑJ��
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

    //�ݒ肵�����������Ԃ��������炻�ꂼ��̍s���ɑJ�ڂ���
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
        //�N���A���o���Ȃ��������
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
    //���b�N�I�����Ă���G�̕�������
    UpdateRotateToTarget(elapsed_time, position, orientation, camera_forward, camera_position);
   //UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}

void Player::AwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation())
    {
        audio_manager->play_se(SE_INDEX::PLAYER_AWAKING);
        player_awaiking_effec->stop(effect_manager->get_effekseer_manager());

        //�ړ����͂���������ړ��ɑJ��
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
        {
            TransitionMove();
        }
        //�ړ����͂��Ȃ�������ҋ@�ɑJ��
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
        //�ړ����͂���������ړ��ɑJ��
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
        {
            TransitionMove();
        }
        //�ړ����͂��Ȃ�������ҋ@�ɑJ��
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
        //�N���A���[�V�������I��������Ƃ�`����
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
    //�`�F�C���U�����͊o����Ԃ̊e�J�ڂɂ͂Ƃ΂Ȃ�
    if (behavior_state == Behavior::Normal)
    {
        //�{�^������
        if (game_pad->get_button() & GamePad::BTN_A)
        {
            if (combo_count >= MAX_COMBO_COUNT&& is_awakening == false)
            {
                //-----�f�[�^���M-----//
                SendPlayerActionData(GamePad::BTN_A, GetInputMoveVec());
                TransitionAwaking();//�R���{�J�E���g���ő�̂Ƃ��͊o����ԂɂȂ�
            }
        }
        if (is_awakening && combo_count <= 0)
        {
            //�o����Ԃ��ǂ����̐ݒ�
            is_awakening = false;
            TransitionInvAwaking();//�o����Ԃ̂Ƃ��ɃJ�E���g��0�ɂȂ�����ʏ��ԂɂȂ�
        }
    }
}

void Player::TransitionIdle(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //�_�b�V���G�t�F�N�g�̏I��
    //end_dash_effect = true;
    //�o����Ԃ̎��̑ҋ@�A�j���[�V�����ɃZ�b�g
    if(is_awakening)model->play_animation(AnimationClips::AwakingIdle, true,true,blend_second);
    //�ʏ��Ԃ̑ҋ@�A�j���[�V�����ɃZ�b�g
    else model->play_animation(AnimationClips::Idle, true,true,blend_second);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ҋ@��Ԃ̎��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::IdleUpdate;
}

void Player::TransitionMove(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    //�G�t�F�N�g�Đ�
    player_move_effec_r->play(effect_manager->get_effekseer_manager(), step_pos_r);
    player_move_effec_l->play(effect_manager->get_effekseer_manager(), step_pos_l);
    //�_�b�V���G�t�F�N�g�̏I��
    //end_dash_effect = true;
    //�o����Ԃ̎��̈ړ��A�j���[�V�����̐ݒ�
    if(is_awakening)model->play_animation(AnimationClips::AwakingMove, true,true, blend_second);
    //�ʏ��Ԃ̎��Ɉړ��A�j���[�V�����̐ݒ�
    else model->play_animation(AnimationClips::Move, true, true,blend_second);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ړ���Ԃ̎��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::MoveUpdate;
}

void Player::TransitionAvoidance()
{
    audio_manager->play_se(SE_INDEX::AVOIDANCE);
    //�G�t�F�N�g�Đ�
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
    player_air_registance_effec->play(effect_manager->get_effekseer_manager(), position,0.3f);
    player_air_registance_effec->set_speed(effect_manager->get_effekseer_manager(), AVOIDANCE_ANIMATION_SPEED);
    //��𒆂��ǂ����̐ݒ�
    is_avoidance = true;
    //��荞�݉�����ǂ���
    is_behind_avoidance = false;
    avoidance_boost_time = 0.0f;
    //�����]���̉�
    avoidance_direction_count = 3;
    //���b�N�I�����ĂȂ��ꍇ�̃^�[�Q�b�g�̐ݒ�
    charge_point = Math::calc_designated_point(position, forward, 200.0f);

    //-----------------------------------------------------------------------------------------//
    //�o����Ԃ̎��̉���A�j���[�V�����̐ݒ�
    if(is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false,true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
    else model->play_animation(AnimationClips::Avoidance, false,true);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�����̑��x
    animation_speed = AVOIDANCE_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;

    //-----�f�[�^���M-----//
    SendPlayerActionData(GamePad::BTN_RIGHT_SHOULDER,GetInputMoveVec());

    //�����Ԃ̎��̍X�V�֐��ɐ؂�ւ���
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
        //���b�N�I�����Ă���G���X�^��������
        target_enemy->fSetStun(true);
    }

    velocity = {};
    //��𒆂��ǂ����̐ݒ�
    is_avoidance = true;
    //��荞�݉�����ǂ���
    is_behind_avoidance = true;
    //�o����Ԃ̎��̉���A�j���[�V�����̐ݒ�
    if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
    else model->play_animation(AnimationClips::Avoidance, false, true);
    //���ɉ�荞�ލ��W�̎擾
    BehindAvoidancePosition();
    //��荞�ނƂ��̃^�C�}�[
    behind_timer = 0;
    //��荞�݂̕⊮���[�g
    behind_late = 0;
    //�ړ����x�̏�����
    velocity = {};
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�����̑��x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;

    //-----�f�[�^���M-----//
    SendPlayerActionData(GamePad::BTN_RIGHT_SHOULDER, GetInputMoveVec());

    //�w��ɉ�荞�ނƂ��̊֐��ɐ؂�ւ���
    player_activity = &Player::BehindAvoidanceUpdate;

}

void Player::TransitionJustBehindAvoidance()
{
    if (invincible_timer > 0) return;

    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());

    player_behaind_effec_2->play(effect_manager->get_effekseer_manager(), position,2.0f);
    audio_manager->play_se(SE_INDEX::WRAPAROUND_AVOIDANCE);
        //���b�N�I�����Ă���G���X�^��������
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
    //HP�񕜂���
    player_health += JUST_AVOIDANCE_HEALTH;
    //�R���{�Q�[�W���₷
    combo_count += JUST_AVOIDANCE_COMBO;
    is_just_avoidance = true;
    velocity = {};
    //��𒆂��ǂ����̐ݒ�
    is_avoidance = true;
    //��荞�݉�����ǂ���
    is_behind_avoidance = true;
    //�o����Ԃ̎��̉���A�j���[�V�����̐ݒ�
    if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true,0.0f);
    //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
    else model->play_animation(AnimationClips::Avoidance, false, true,0.0f);
    //���ɉ�荞�ލ��W�̎擾
    BehindAvoidancePosition();
    //��荞�ނƂ��̃^�C�}�[
    behind_timer = 0;
    //��荞�݂̕⊮���[�g
    behind_late = 0;
    //�ړ����x�̏�����
    velocity = {};
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�����̑��x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;

    //-----�f�[�^���M-----//
    SendPlayerActionData(GamePad::BTN_RIGHT_SHOULDER, GetInputMoveVec());

    //�w��ɉ�荞�ނƂ��̊֐��ɐ؂�ւ���
    player_activity = &Player::BehindAvoidanceUpdate;
}

void Player::TransitionChargeInit()
{
    //player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //�o����Ԃ̎��̓ːi�̎n�܂�̃A�j���[�V�����ɐݒ�
   if(is_awakening)model->play_animation(AnimationClips::AwakingChargeInit, false,true);
   //�ʏ��Ԃ̎��̓ːi�̎n�܂�̃A�j���[�V�����ɐݒ�
   else model->play_animation(AnimationClips::ChargeInit, false,true);
   //�U�������ǂ����̐ݒ�
   is_attack = true;
   //�ːi�����ǂ����̐ݒ�
   is_charge = true;
   //�A�j���[�V�������x�̐ݒ�
    animation_speed = CHARGEINIT_ANIMATION_SPEED;
    //���b�N�I�����ĂȂ��ꍇ�̃^�[�Q�b�g�̐ݒ�
    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //�����̃��[�g
    lerp_rate = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;

    //-----�f�[�^���M-----//
    //SendActionData(GamePad::BTN_ATTACK_B);
    //-----�f�[�^���M-----//
    SendPlayerActionData(GamePad::BTN_ATTACK_B ,GetInputMoveVec());

    //�ːi�̎n�܂�̎��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::ChargeInitUpdate;
}

void Player::TransitionCharge(float blend_seconds)
{
    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
    //�G�t�F�N�g�Đ�
    player_air_registance_effec->play(effect_manager->get_effekseer_manager(), position, 0.3f);
    //�_�b�V���|�X�g�G�t�F�N�g��������
    start_dash_effect = true;
    //�o����Ԃ̎��̓ːi�A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = CHARGE_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingCharge, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̓ːi�A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = CHARGE_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::Charge, false, true, blend_seconds);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
#if 1
    animation_speed = CHARGE_ANIMATION_SPEED;
#else
    //�f�o�b�O�p
    animation_speed = attack_animation_speeds.x;
#endif // 0
    //charge_point = Math::calc_designated_point(position, forward, 60.0f);
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�����̃��[�g
    lerp_rate = 4.0f;
    //�U���̉����̐ݒ�
    //SetAccelerationVelocity();

    //-----�f�[�^���M-----//
    //SendActionData(GamePad::BTN_ATTACK_B);

    //�ːi���̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::ChargeUpdate;

}
void Player::TransitionAttackType1(float blend_seconds)
{
    player_air_registance_effec->stop(effect_manager->get_effekseer_manager());

    //�o����Ԃ̎��̂P���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE1_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType1, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̂P���ڂ̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE1_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType1, false, true, blend_seconds);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
    animation_speed = ATTACK1_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�����̃��[�g
    lerp_rate = 4.0f;

    //-----�f�[�^���M-----//
    //SendActionData(GamePad::BTN_ATTACK_B);

    //�P���ڂ̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::AttackType1Update;
}

void Player::TransitionAttackType2(float blend_seconds)
{
    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);

    //�o����Ԃ̎��̂Q���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE2_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType2, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̂Q���ڂ̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE2_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType2, false, true, blend_seconds);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = ATTACK2_ANIMATION_SPEED;
    //�U���̉����̐ݒ�
    //SetAccelerationVelocity();
        //���b�N�I�����ĂȂ��ꍇ�̃^�[�Q�b�g�̐ݒ�
    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //�����̃��[�g
    lerp_rate = 2.0f;
    //�U���̎���
    attack_time = 0;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = false;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;

    //-----�f�[�^���M-----//
    //SendActionData(GamePad::BTN_ATTACK_B);

    //-----�f�[�^���M-----//
    SendPlayerActionData(GamePad::BTN_ATTACK_B);

    //�Q���ڂ̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::AttackType2Update;
}
void Player::TransitionAttackType3(float blend_seconds)
{
    audio_manager->play_se(SE_INDEX::PLAYER_RUSH);

    //�o����Ԃ̎��̂R���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE3_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType3, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̂R���ڂ̂̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE3_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType3, false, true, blend_seconds);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = ATTACK3_ANIMATION_SPEED;
    //�U���̉����̐ݒ�
    //SetAccelerationVelocity();
    //�����̃��[�g
    lerp_rate =2.0f;
    //�U���̎���
    attack_time = 0;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = false;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;

    //-----�f�[�^���M-----//
    //SendActionData(GamePad::BTN_ATTACK_B);

    //-----�f�[�^���M-----//
    SendPlayerActionData(GamePad::BTN_ATTACK_B);

    //�R���ڂ̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::AttackType3Update;
}

void Player::TransitionSpecialSurge()
{
    //��s�@���[�h�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::IdleWing, true,true);
    //�Q�[�W����̓ːi���ɓ��������G�̐���������
    special_surge_combo_count = 0;
    //�Q�[�W����̓ːi���ǂ����̐ݒ�
    is_special_surge = true;
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //���b�N�I�����ĂȂ��ꍇ�̃^�[�Q�b�g�̐ݒ�
    charge_point = Math::calc_designated_point(position, forward, 100.0f);
    //�Q�[�W����̓ːi�̈ړ����x��ݒ�
    SpecialSurgeAcceleration();
    //�R���{�J�E���g�̏���
    combo_count -= 10.0f;
    //�R���{�J�E���g�̐���
    combo_count = Math::clamp(combo_count, 0.0f, MAX_COMBO_COUNT);
    //�Q�[�W����̓ːi�̃^�C�}�[
    //special_surge_timer = 0.0f;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�Q�[�W����̓ːi�̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::SpecialSurgeUpdate;
}

void Player::TransitionOpportunity()
{
    //�Q�[�W����̓ːi�U���I��
    is_special_surge = false;
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�������������̌o�ߎ��Ԃ����Z�b�g
    //special_surge_timer = 0;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�Q�[�W����̓ːi�̌��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::OpportunityUpdate;
}

void Player::TransitionDamage()
{
    PostEffect::clear_post_effect();
    velocity = {};
    //�_�b�V���G�t�F�N�g�̏I��
    start_dash_effect = false;
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�o����Ԃ̎��̃_���[�W�A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(AnimationClips::AwakingDamage, false, true,0.0f);
    //�ʏ��Ԃ̎��̃A�j���[�V�����ɐݒ�
    else model->play_animation(AnimationClips::Damage, false, true,0.0f);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�_���[�W�󂯂��Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::DamageUpdate;
}

void Player::TransitionTransformHum()
{
    //�l�^�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::TransformHum, false,true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = TRANSFORM_HUM_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�l�^�ɂȂ��Ă�Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::TransformHumUpdate;
}

void Player::TransitionTransformWing()
{
    velocity = {};
    //��s�@���[�h�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::TransformWing, false,true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = TRANSFORM_WING_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //��s�@���[�h���̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::TransformWingUpdate;
}

void Player::TransitionAwaking()
{
    invincible_timer = 2.0f;
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    player_awaiking_effec->play(effect_manager->get_effekseer_manager(), position,2.0f);
    invincible_timer = 2.0f;
    //�o����ԂɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::Awaking, false,true);
    //�o����Ԃ��ǂ����̐ݒ�
    is_awakening = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�o����ԂɂȂ�r���̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::AwakingUpdate;
}

void Player::TransitionInvAwaking()
{
    invincible_timer = 2.0f;
    //�ʏ��Ԃɖ߂�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::InvAwaking, false,true);
    //�o����Ԃ��ǂ����̐ݒ�
    is_awakening = false;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ʏ��Ԃɖ߂��Ă�Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::InvAwakingUpdate;

}

void Player::TransitionWingDashStart()
{
    model->play_animation(AnimationClips::WingDashStart, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    player_activity = &Player::WingDashStartUpdate;

}

void Player::TransitionWingDashIdle()
{
    model->play_animation(AnimationClips::WingDashIdle, true, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    player_activity = &Player::WingDashIdleUpdate;

}

void Player::TransitionWingDashEnd()
{
}
void Player::TransitionDie()
{
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    velocity = {};
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�o����Ԃ̎��̃_���[�W�A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(AnimationClips::AwakingDie, false, true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����ɐݒ�
    else model->play_animation(AnimationClips::Die, false, true);
    //�X�V�֐��ɐ؂�ւ�
    player_activity = &Player::DieUpdate;
    if (GameFile::get_instance().get_vibration())game_pad->set_vibration(1.0f, 1.0f, 1.0f);
}

void Player::TransitionDying()
{
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    velocity = {};
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�o����Ԃ̎��̃_���[�W�A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(AnimationClips::AwakingDying, true, true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����ɐݒ�
    else model->play_animation(AnimationClips::Dying, true, true);
    //�X�V�֐��ɐ؂�ւ�
    player_activity = &Player::DyingUpdate;

}

void Player::TransitionNamelessMotionIdle()
{
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    velocity = {};
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::NamelessMotionIdle, false, true);
    //�X�V�֐��ɐ؂�ւ�
    player_activity = &Player::NamelessMotionIdleUpdate;
}

void Player::TransitionStartMothin()
{
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    velocity = {};
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V�����ɐݒ�
    //model->play_animation(AnimationClips::StartMothin, false, true);
    //�X�V�֐��ɐ؂�ւ�
    player_activity = &Player::StartMothinUpdate;

}

void Player::TransitionNamelessMotion()
{
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    //�N���A�p���[�V�������n�܂����炩��true�ɂ���
    is_start_cleear_motion = true;
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    velocity = {};
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::NamelessMotion, false, true);
    //�X�V�֐��ɐ؂�ւ�
    player_activity = &Player::NamelessMotionUpdate;
}

void Player::TransitionStageMove()
{
    player_move_effec_r->stop(effect_manager->get_effekseer_manager());
    player_move_effec_l->stop(effect_manager->get_effekseer_manager());

    //�X�e�[�W�J�ڂ̎��ɉ񕜂���
    const float health = static_cast<float>(player_health) /static_cast<float>(MAX_HEALTH);
    if (health < 0.7f) player_health = static_cast<int>(MAX_HEALTH * 0.7);
    //player_health += RECOVERY_HEALTH;
    velocity = {};
    //�ړ��̃A�j���[�V�����ɂ���()
    model->play_animation(AnimationClips::TransformWing, false);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ʏ��Ԃɖ߂��Ă�Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &Player::StageMoveUpdate;
    during_clear = true;
}

void Player::TransitionStageMoveEnd()
{
    model->play_animation(AnimationClips::WingDashEnd, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
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
    //�ړ����͂��Ȃ��Ȃ�����ҋ@�ɑJ��
    if (during_chain_attack() == false && sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) <= 0)
    {
        TransitionChainIdle();
    }
    UpdateVelocity(elapsed_time, position, orientation, camera_forward, camera_right, camera_position, sky_dome);
}
void Player::TransitionChainIdle(float blend_second)
{
    if (is_awakening)model->play_animation(AnimationClips::AwakingIdle, true, true, blend_second);
    //�ʏ��Ԃ̑ҋ@�A�j���[�V�����ɃZ�b�g
    else model->play_animation(AnimationClips::Idle, true, true, blend_second);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ҋ@��Ԃ̎��̍X�V�֐��ɐ؂�ւ���
    chain_activity = &Player::ChainIdleUpdate;
}
void Player::TransitionChainMove(float blend_second)
{
    if (is_awakening)model->play_animation(AnimationClips::AwakingMove, true, true, blend_second);
    //�ʏ��Ԃ̎��Ɉړ��A�j���[�V�����̐ݒ�
    else model->play_animation(AnimationClips::Move, true, true, blend_second);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ړ���Ԃ̎��̍X�V�֐��ɐ؂�ւ���
    chain_activity = &Player::ChainMoveUpdate;
}

