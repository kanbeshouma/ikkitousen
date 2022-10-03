#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include"ClientPlayer.h"

void ClientPlayer::TransitionIdle(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    //�_�b�V���G�t�F�N�g�̏I��
    //end_dash_effect = true;
    //�o����Ԃ̎��̑ҋ@�A�j���[�V�����ɃZ�b�g
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingIdle, true, true, blend_second);
    //�ʏ��Ԃ̑ҋ@�A�j���[�V�����ɃZ�b�g
    else model->play_animation(anim_parm, AnimationClips::Idle, true, true, blend_second);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ҋ@��Ԃ̎��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::IdleUpdate;
}

void ClientPlayer::TransitionMove(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    //�_�b�V���G�t�F�N�g�̏I��
    //end_dash_effect = true;
    //�o����Ԃ̎��̈ړ��A�j���[�V�����̐ݒ�
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingMove, true, true, blend_second);
    //�ʏ��Ԃ̎��Ɉړ��A�j���[�V�����̐ݒ�
    else model->play_animation(anim_parm, AnimationClips::Move, true, true, blend_second);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�������x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ړ���Ԃ̎��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::MoveUpdate;
}

void ClientPlayer::TransitionAvoidance()
{
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
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingAvoidance, false, true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
    else model->play_animation(anim_parm, AnimationClips::Avoidance, false, true);
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�����̑��x
    animation_speed = AVOIDANCE_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�����Ԃ̎��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::AvoidanceUpdate;
}

void ClientPlayer::TransitionBehindAvoidance()
{
    velocity = {};
    //��𒆂��ǂ����̐ݒ�
    is_avoidance = true;
    //��荞�݉�����ǂ���
    is_behind_avoidance = true;
    //�o����Ԃ̎��̉���A�j���[�V�����̐ݒ�
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingAvoidance, false, true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
    else model->play_animation(anim_parm, AnimationClips::Avoidance, false, true);
    //���ɉ�荞�ލ��W�̎擾
    BehindAvoidancePosition();

    //�ړ����x�̏�����
    velocity = {};
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�����̑��x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�w��ɉ�荞�ނƂ��̊֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::BehindAvoidanceUpdate;

}

void ClientPlayer::TransitionJustBehindAvoidance()
{
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
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingAvoidance, false, true, 0.0f);
    //�ʏ��Ԃ̎��̃A�j���[�V�����̐ݒ�
    else model->play_animation(anim_parm, AnimationClips::Avoidance, false, true, 0.0f);
    //���ɉ�荞�ލ��W�̎擾
    BehindAvoidancePosition();
    //�ړ����x�̏�����
    velocity = {};
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�A�j���[�V�����̑��x
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�w��ɉ�荞�ނƂ��̊֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::BehindAvoidanceUpdate;
}

void ClientPlayer::TransitionChargeInit()
{
    //�o����Ԃ̎��̓ːi�̎n�܂�̃A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingChargeInit, false, true);
    //�ʏ��Ԃ̎��̓ːi�̎n�܂�̃A�j���[�V�����ɐݒ�
    else model->play_animation(anim_parm, AnimationClips::ChargeInit, false, true);
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
    //�ːi�̎n�܂�̎��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::ChargeInitUpdate;
}

void ClientPlayer::TransitionCharge(float blend_seconds)
{
    //�_�b�V���|�X�g�G�t�F�N�g��������
    start_dash_effect = true;
    //�o����Ԃ̎��̓ːi�A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = CHARGE_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingCharge, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̓ːi�A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = CHARGE_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::Charge, false, true, blend_seconds);
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
    //�ːi���̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::ChargeUpdate;

}
void ClientPlayer::TransitionAttackType1(float blend_seconds)
{
    //�o����Ԃ̎��̂P���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE1_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingAttackType1, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̂P���ڂ̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE1_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AttackType1, false, true, blend_seconds);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
    animation_speed = ATTACK1_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�����̃��[�g
    lerp_rate = 4.0f;
    //�P���ڂ̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::AttackType1Update;
}

void ClientPlayer::TransitionAttackType2(float blend_seconds)
{

    //�o����Ԃ̎��̂Q���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE2_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingAttackType2, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̂Q���ڂ̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE2_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AttackType2, false, true, blend_seconds);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = ATTACK2_ANIMATION_SPEED;
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
    //�Q���ڂ̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::AttackType2Update;
}

void ClientPlayer::TransitionAttackType3(float blend_seconds)
{
    //�o����Ԃ̎��̂R���ڂ̃A�j���[�V�����ɐݒ�
    if (is_awakening)
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE3_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingAttackType3, false, true, blend_seconds);
    }
    //�ʏ��Ԃ̎��̂R���ڂ̂̃A�j���[�V�����ɐݒ�
    else
    {
        //�v���C���[�̍U����
        player_attack_power = ATTACK_TYPE3_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AttackType3, false, true, blend_seconds);
    }
    //�U�������ǂ����̐ݒ�
    is_attack = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = ATTACK3_ANIMATION_SPEED;
    //�U���̉����̐ݒ�
    //SetAccelerationVelocity();
    //�����̃��[�g
    lerp_rate = 2.0f;
    //�U���̎���
    attack_time = 0;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = false;
    //�ːi�����ǂ����̐ݒ�
    is_charge = true;
    //�R���ڂ̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::AttackType3Update;
}

void ClientPlayer::TransitionSpecialSurge()
{
    //��s�@���[�h�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(anim_parm, AnimationClips::IdleWing, true, true);
    //�Q�[�W����̓ːi���ɓ��������G�̐���������
    special_surge_combo_count = 0;
    //�Q�[�W����̓ːi���ǂ����̐ݒ�
    is_special_surge = true;
    //�U�������ǂ����̐ݒ�
    is_attack = true;
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
    player_activity = &ClientPlayer::SpecialSurgeUpdate;
}

void ClientPlayer::TransitionOpportunity()
{
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�������������̌o�ߎ��Ԃ����Z�b�g
    //special_surge_timer = 0;
    //�A�j���[�V�����X�s�[�h�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�Q�[�W����̓ːi�̌��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::OpportunityUpdate;
}

void ClientPlayer::TransitionDamage()
{
    velocity = {};
    //�_�b�V���G�t�F�N�g�̏I��
    start_dash_effect = false;
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    //�o����Ԃ̎��̃_���[�W�A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingDamage, false, true, 0.0f);
    //�ʏ��Ԃ̎��̃A�j���[�V�����ɐݒ�
    else model->play_animation(anim_parm, AnimationClips::Damage, false, true, 0.0f);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�_���[�W�󂯂��Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::DamageUpdate;
}

void ClientPlayer::TransitionTransformHum()
{
    //�l�^�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(anim_parm, AnimationClips::TransformHum, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = TRANSFORM_HUM_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�l�^�ɂȂ��Ă�Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::TransformHumUpdate;
}

void ClientPlayer::TransitionTransformWing()
{
    velocity = {};
    //��s�@���[�h�ɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(anim_parm, AnimationClips::TransformWing, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = TRANSFORM_WING_ANIMATION_SPEED;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //��s�@���[�h���̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::TransformWingUpdate;
}

void ClientPlayer::TransitionAwaking()
{
    invincible_timer = 2.0f;
    invincible_timer = 2.0f;
    //�o����ԂɂȂ�A�j���[�V�����ɐݒ�
    model->play_animation(anim_parm, AnimationClips::Awaking, false, true);
    //�o����Ԃ��ǂ����̐ݒ�
    is_awakening = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�o����ԂɂȂ�r���̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::AwakingUpdate;
}

void ClientPlayer::TransitionInvAwaking()
{
    invincible_timer = 2.0f;
    //�ʏ��Ԃɖ߂�A�j���[�V�����ɐݒ�
    model->play_animation(anim_parm, AnimationClips::InvAwaking, false, true);
    //�o����Ԃ��ǂ����̐ݒ�
    is_awakening = false;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ʏ��Ԃɖ߂��Ă�Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::InvAwakingUpdate;

}

void ClientPlayer::TransitionWingDashStart()
{
    model->play_animation(anim_parm, AnimationClips::WingDashStart, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    player_activity = &ClientPlayer::WingDashStartUpdate;

}

void ClientPlayer::TransitionWingDashIdle()
{
    model->play_animation(anim_parm, AnimationClips::WingDashIdle, true, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    player_activity = &ClientPlayer::WingDashIdleUpdate;

}

void ClientPlayer::TransitionWingDashEnd()
{
}

void ClientPlayer::TransitionDie()
{
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    velocity = {};
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�o����Ԃ̎��̃_���[�W�A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingDie, false, true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����ɐݒ�
    else model->play_animation(anim_parm, AnimationClips::Die, false, true);
    //�X�V�֐��ɐ؂�ւ�
    player_activity = &ClientPlayer::DieUpdate;
}

void ClientPlayer::TransitionDying()
{
    //�U�������ǂ����̐ݒ�
    is_attack = false;
    velocity = {};
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�o����Ԃ̎��̃_���[�W�A�j���[�V�����ɐݒ�
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingDying, true, true);
    //�ʏ��Ԃ̎��̃A�j���[�V�����ɐݒ�
    else model->play_animation(anim_parm, AnimationClips::Dying, true, true);
    //�X�V�֐��ɐ؂�ւ�
    player_activity = &ClientPlayer::DyingUpdate;

}


void ClientPlayer::TransitionStartMothin()
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
    player_activity = &ClientPlayer::StartMothinUpdate;

}

void ClientPlayer::TransitionStageMove()
{
    //�X�e�[�W�J�ڂ̎��ɉ񕜂���
    const float health = static_cast<float>(player_health) / static_cast<float>(MAX_HEALTH);
    if (health < 0.7f) player_health = static_cast<int>(MAX_HEALTH * 0.7);
    //player_health += RECOVERY_HEALTH;
    velocity = {};
    //�ړ��̃A�j���[�V�����ɂ���()
    model->play_animation(anim_parm, AnimationClips::TransformWing, false);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    //�ʏ��Ԃɖ߂��Ă�Ƃ��̍X�V�֐��ɐ؂�ւ���
    player_activity = &ClientPlayer::StageMoveUpdate;
    during_clear = true;
}

void ClientPlayer::TransitionStageMoveEnd()
{
    model->play_animation(anim_parm, AnimationClips::WingDashEnd, false, true);
    //�A�j���[�V�������x�̐ݒ�
    animation_speed = 1.0f;
    //�A�j���[�V���������Ă������ǂ���
    is_update_animation = true;
    player_activity = &ClientPlayer::WingDashEndUpdate;

}

