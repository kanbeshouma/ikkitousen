#include"ClientPlayer.h"

void ClientPlayer::TransitionIdle()
{
    //-----もしステートがIdleでなかったら処理を止める-----//
    if (action_state != ActionState::Idle) return;

    if (is_awakening)model->play_animation(AnimationClips::AwakingIdle, true, true);
    else model->play_animation(AnimationClips::Idle, true, true);

    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;

    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time,SkyDome* sky_dome) { IdleUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionMove()
{
    if (action_state != ActionState::Move) return;

    if (is_awakening)model->play_animation(AnimationClips::AwakingMove, true, true);
    else model->play_animation(AnimationClips::Move, true, true);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;

    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { MoveUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAvoidance()
{
    if (action_state != ActionState::Avoidance) return;

    if (is_awakening)model->play_animation(AnimationClips::AwakingAvoidance, false, true);
    else model->play_animation(AnimationClips::Avoidance, false, true);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = AVOIDANCE_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AvoidanceUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionChargeInit()
{
    if (action_state != ActionState::ChargeInit) return;

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
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { ChargeInitUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionCharge()
{
    if (action_state != ActionState::Charge) return;
    //突進中かどうかの設定
    is_charge = true;

    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = CHARGE_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingCharge, false, true);
    }
    //通常状態の時の突進アニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = CHARGE_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::Charge, false, true);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //突進中かどうかの設定
    is_charge = true;
    //アニメーションスピードの設定
    animation_speed = CHARGE_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { ChargeUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAttackType1()
{
    if (action_state != ActionState::Attack1) return;
    //覚醒状態の時の１撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType1, false, true);
    }
    //通常状態の時の１撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType1, false, true);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーションスピードの設定
    animation_speed = ATTACK1_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AttackType1Update(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionAttackType2()
{
    if (action_state != ActionState::Attack2) return;

    //覚醒状態の時の２撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType2, false, true);
    }
    //通常状態の時の２撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType2, false, true);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーション速度の設定
    animation_speed = ATTACK2_ANIMATION_SPEED;
    //突進中かどうかの設定
    is_charge = true;
    //アニメーションをしていいかどうか
    is_update_animation = false;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AttackType2Update(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAttackType3()
{
    if (action_state != ActionState::Attack3) return;
    //覚醒状態の時の３撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_AWAIKING_ATTACK_POWER;
        model->play_animation(AnimationClips::AwakingAttackType3, false, true);
    }
    //通常状態の時の３撃目ののアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_NORMAL_ATTACK_POWER;
        model->play_animation(AnimationClips::AttackType3, false, true);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーション速度の設定
    animation_speed = ATTACK3_ANIMATION_SPEED;
    //突進中かどうかの設定
    is_charge = true;
    //アニメーションをしていいかどうか
    is_update_animation = false;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AttackType3Update(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionDamage()
{
    if (action_state != ActionState::Damage) return;

    //攻撃中かどうかの設定
    is_attack = false;
    //覚醒状態の時のダメージアニメーションに設定
    if (is_awakening)model->play_animation(AnimationClips::AwakingDamage, false, true, 0.0f);
    //通常状態の時のアニメーションに設定
    else model->play_animation(AnimationClips::Damage, false, true, 0.0f);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { DamageUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionTransformWing()
{
    if (action_state != ActionState::TransformWing) return;
    //飛行機モードになるアニメーションに設定
    model->play_animation(AnimationClips::TransformWing, false, true);
    //アニメーション速度の設定
    animation_speed = TRANSFORM_WING_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    velocity = {};
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { TransformWingUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionTransformHum()
{
    if (action_state != ActionState::TransformHum) return;
    //人型になるアニメーションに設定
    model->play_animation(AnimationClips::TransformHum, false, true);
    //アニメーション速度の設定
    animation_speed = TRANSFORM_HUM_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { TransformHumUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionAwaking()
{
    if (action_state != ActionState::Awaking) return;
    //覚醒状態になるアニメーションに設定
    model->play_animation(AnimationClips::Awaking, false, true);
    //覚醒状態かどうかの設定
    is_awakening = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { AwakingUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionInvAwaking()
{
    if (action_state != ActionState::InvAwaking) return;
    invincible_timer = 2.0f;
    //通常状態に戻るアニメーションに設定
    model->play_animation(AnimationClips::InvAwaking, false, true);
    //覚醒状態かどうかの設定
    is_awakening = false;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { InvAwakingUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionDie()
{
    if (action_state != ActionState::Die) return;
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
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { DieUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionDying()
{
    if (action_state != ActionState::Dying) return;
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
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { DyingUpdate(elapsed_time, sky_dome); };

}

void ClientPlayer::TransitionStageMove()
{
    if (action_state != ActionState::StageMove) return;
    velocity = {};
    //移動のアニメーションにする()
    model->play_animation(AnimationClips::TransformWing, false);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { StageMoveUpdate(elapsed_time, sky_dome); };
}

void ClientPlayer::TransitionStageMoveEnd()
{
    if (action_state != ActionState::StageMoveEnd) return;
    model->play_animation(AnimationClips::WingDashEnd, false, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //-----関数を設定する-----//
    update_animation = [=](float elapsed_time, SkyDome* sky_dome) { StageMoveEndUpdate(elapsed_time, sky_dome); };
}
