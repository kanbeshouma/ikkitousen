#include"ClientPlayer.h"

void ClientPlayer::TransitionIdle()
{
    //-----�����X�e�[�g��Idle�łȂ������珈�����~�߂�-----//
    if (action_state != ActionState::Idle) return;

    if (is_awakening)model->play_animation(AnimationClips::AwakingIdle, true, true);
    else model->play_animation(AnimationClips::Idle, true, true);

    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;

    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time,SkyDome* sky_dome) { IdleUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionMove()
{
    if (action_state != ActionState::Move) return;

    if (is_awakening)model->play_animation(AnimationClips::AwakingMove, true, true);
    else model->play_animation(AnimationClips::Move, true, true);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;

    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { MoveUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAvoidance()
{
    if (action_state != ActionState::Avoidance) return;

    if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
    else model->play_animation(AnimationClips::Avoidance, false, true);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�����̑��x
    animation_speed = AVOIDANCE_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AvoidanceUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionChargeInit()
{
    if (action_state != ActionState::ChargeInit) return;

    //�o����Ԃ̎��̓ːi�̎n�܂�̃A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(AnimationClips::AwakingChargeInit, false, true);
    //�ʏ��Ԃ̎��̓ːi�̎n�܂�̃A�j���[�V�����ɐݒ�
    else model->play_animation(AnimationClips::ChargeInit, false, true);

    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = CHARGEINIT_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { ChargeInitUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionCharge()
{
    if (action_state != ActionState::Charge) return;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;

    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = CHARGE_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingCharge, false, true);
    }
    //�ʏ��Ԃ̎��̓ːi�A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = CHARGE_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::Charge, false, true);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
    animation_speed = CHARGE_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { ChargeUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAttackType1()
{
    if (action_state != ActionState::Attack1) return;
    //�o����Ԃ̎��̂P���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE1_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType1, false, true);
    }
    //�ʏ��Ԃ̎��̂P���ڂ̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE1_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType1, false, true);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
    animation_speed = ATTACK1_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AttackType1Update(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionAttackType2()
{
    if (action_state != ActionState::Attack2) return;

    //�o����Ԃ̎��̂Q���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE2_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType2, false, true);
    }
    //�ʏ��Ԃ̎��̂Q���ڂ̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE2_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType2, false, true);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = ATTACK2_ANIMATION_SPEED;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = false;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AttackType2Update(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAttackType3()
{
    if (action_state != ActionState::Attack3) return;
    //�o����Ԃ̎��̂R���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE3_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType3, false, true);
    }
    //�ʏ��Ԃ̎��̂R���ڂ̂̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE3_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType3, false, true);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = ATTACK3_ANIMATION_SPEED;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = false;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AttackType3Update(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionDamage()
{
    if (action_state != ActionState::Damage) return;

    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�o����Ԃ̎��̃_���[�W�A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(AnimationClips::AwakingDamage, false, true, 0.0f);
    //�ʏ��Ԃ̎��̃A�j���[�V�����ɐݒ�
    else model->play_animation(AnimationClips::Damage, false, true, 0.0f);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { DamageUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionTransformWing()
{
    if (action_state != ActionState::TransformWing) return;
    //��s�@���[�h�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::TransformWing, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = TRANSFORM_WING_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    velocity = {};
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { TransformWingUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionTransformHum()
{
    if (action_state != ActionState::TransformHum) return;
    //�l�^�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::TransformHum, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = TRANSFORM_HUM_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { TransformHumUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAwaking()
{
    if (action_state != ActionState::Awaking) return;
    //�o����ԂɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::Awaking, false, true);
    //�o����Ԃ��ǂ����̐ݒ�
    is_awakening = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AwakingUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionInvAwaking()
{
    if (action_state != ActionState::InvAwaking) return;
    invincible_timer = 2.0f;
    //�ʏ��Ԃɖ߂�A�j���[�V�����ɐݒ�
    model->play_animation(AnimationClips::InvAwaking, false, true);
    //�o����Ԃ��ǂ����̐ݒ�
    is_awakening = false;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { InvAwakingUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionDie()
{
    if (action_state != ActionState::Die) return;
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
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { DieUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionDying()
{
    if (action_state != ActionState::Dying) return;
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
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { DyingUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionStageMove()
{
    if (action_state != ActionState::StageMove) return;
    velocity = {};
    //�ړ��̃A�j���[�V�����ɂ���()
    model->play_animation(AnimationClips::TransformWing, false);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { StageMoveUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionStageMoveEnd()
{
    if (action_state != ActionState::StageMoveEnd) return;
    model->play_animation(AnimationClips::WingDashEnd, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //-----�֐���ݒ肷��-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { StageMoveEndUpdate(elapsed_time, sky_dome); };
}
