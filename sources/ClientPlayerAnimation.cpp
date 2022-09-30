#include"ClientPlayer.h"

void ClientPlayer::ExecFuncUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_)
{
    (this->*player_activity)(elapsed_time,sky_dome);
}

void ClientPlayer::IdleUpdate(float elapsed_time, SkyDome* sky_dome)
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
    if (change_normal_timer <= 0 && behavior_state == Behavior::Normal)
    {
        //����ɑJ��
        float length{ Math::calc_vector_AtoB_length(position, target) };
        if (avoidance_buttun == false && (triggerR || button_down & GamePad::BTN_RIGHT_SHOULDER))
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
                else TransitionAvoidance();
            }
        }
        //�ːi�J�n�ɑJ��
        if (button_down & GamePad::BTN_ATTACK_B)
        {
            TransitionChargeInit();
        }

        Awaiking();
    }
}

void ClientPlayer::MoveUpdate(float elapsed_time, SkyDome* sky_dome)
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
        if (avoidance_buttun == false && (triggerR || button_down & GamePad::BTN_RIGHT_SHOULDER))
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
    //����̎��̉���
    SetAccelerationVelocity();

    //-----�U���{�^������������U���ɑJ��-----//
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
            //��𒆂��ǂ����̐ݒ�
            is_avoidance = false;
            is_behind_avoidance = false;
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
                //�o����Ԃ̎��̉���A�j���[�V�����̐ݒ�
                if (is_awakening)model->play_animation(anim_parm,AnimationClips::AwakingAvoidance, false, true);
                //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
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
    //�u���b�N����Ă����猕���ӂ��ċ���
    if (is_block)
    {
        TransitionAttackType1(attack_animation_blends_speeds.y);
    }
    charge_time += charge_add_time * elapsed_time;
    //�U���̉����̐ݒ�
    SetAccelerationVelocity();

    //�ːi���Ԃ𒴂����炻�ꂼ��̑J�ڂɂƂ�
    if (charge_time > CHARGE_MAX_TIME)
    {

        velocity.x *= 0.2f;
        velocity.y *= 0.2f;
        velocity.z *= 0.2f;

        //�ړ����͂���������ړ��ɑJ��
        if (sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) > PLAYER_INPUT_MIN)
        {
            charge_time = 0;
            is_charge = false;
            TransitionMove();
        }
        //�ړ����͂��Ȃ�������ҋ@�ɑJ��
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

    if (model->end_of_animation(anim_parm))
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

    //�G�ɓ������������Ԃ�2�b��������������I���

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

    //�ݒ肵�����������Ԃ��������炻�ꂼ��̍s���ɑJ�ڂ���
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
        //�N���A���o���Ȃ��������
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

void ClientPlayer::InvAwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    if (model->end_of_animation(anim_parm))
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
    //�`�F�C���U�����͊o����Ԃ̊e�J�ڂɂ͂Ƃ΂Ȃ�
    if (behavior_state == Behavior::Normal)
    {
        //�{�^������
        if (game_pad->get_button() & GamePad::BTN_A)
        {
            if (combo_count >= MAX_COMBO_COUNT && is_awakening == false)
            {
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

