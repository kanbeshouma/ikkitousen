#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"ClientPlayer.h"

void ClientPlayer::TransitionIdle(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    //ダッシュエフェクトの終了
    //end_dash_effect = true;
    //覚醒状態の時の待機アニメーションにセット
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingIdle, true, true, blend_second);
    //通常状態の待機アニメーションにセット
    else model->play_animation(anim_parm, AnimationClips::Idle, true, true, blend_second);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //待機状態の時の更新関数に切り替える
    player_activity = &ClientPlayer::IdleUpdate;
}

void ClientPlayer::TransitionMove(float blend_second)
{
    charge_change_direction_count = CHARGE_DIRECTION_COUNT;

    //ダッシュエフェクトの終了
    //end_dash_effect = true;
    //覚醒状態の時の移動アニメーションの設定
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingMove, true, true, blend_second);
    //通常状態の時に移動アニメーションの設定
    else model->play_animation(anim_parm, AnimationClips::Move, true, true, blend_second);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーション速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //移動状態の時の更新関数に切り替える
    player_activity = &ClientPlayer::MoveUpdate;
}

void ClientPlayer::TransitionAvoidance()
{
    is_avoidance = true;
    //回り込み回避かどうか
    is_behind_avoidance = false;
    avoidance_boost_time = 0.0f;
    //方向転換の回数
    avoidance_direction_count = 3;
    //ロックオンしてない場合のターゲットの設定
    charge_point = Math::calc_designated_point(position, forward, 200.0f);
    //-----------------------------------------------------------------------------------------//
    //覚醒状態の時の回避アニメーションの設定
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingAvoidance, false, true);
    //通常状態の時のアニメーションの設定
    else model->play_animation(anim_parm, AnimationClips::Avoidance, false, true);
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = AVOIDANCE_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //回避状態の時の更新関数に切り替える
    player_activity = &ClientPlayer::AvoidanceUpdate;
}

void ClientPlayer::TransitionBehindAvoidance()
{
    velocity = {};
    //回避中かどうかの設定
    is_avoidance = true;
    //回り込み回避かどうか
    is_behind_avoidance = true;
    //覚醒状態の時の回避アニメーションの設定
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingAvoidance, false, true);
    //通常状態の時のアニメーションの設定
    else model->play_animation(anim_parm, AnimationClips::Avoidance, false, true);
    //後ろに回り込む座標の取得
    BehindAvoidancePosition();

    //移動速度の初期化
    velocity = {};
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //背後に回り込むときの関数に切り替える
    player_activity = &ClientPlayer::BehindAvoidanceUpdate;

}

void ClientPlayer::TransitionJustBehindAvoidance()
{
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
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingAvoidance, false, true, 0.0f);
    //通常状態の時のアニメーションの設定
    else model->play_animation(anim_parm, AnimationClips::Avoidance, false, true, 0.0f);
    //後ろに回り込む座標の取得
    BehindAvoidancePosition();
    //移動速度の初期化
    velocity = {};
    //攻撃中かどうかの設定
    is_attack = false;
    //アニメーションの速度
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //背後に回り込むときの関数に切り替える
    player_activity = &ClientPlayer::BehindAvoidanceUpdate;
}

void ClientPlayer::TransitionChargeInit()
{
    //覚醒状態の時の突進の始まりのアニメーションに設定
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingChargeInit, false, true);
    //通常状態の時の突進の始まりのアニメーションに設定
    else model->play_animation(anim_parm, AnimationClips::ChargeInit, false, true);
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
    player_activity = &ClientPlayer::ChargeInitUpdate;
}

void ClientPlayer::TransitionCharge(float blend_seconds)
{
    //ダッシュポストエフェクトをかける
    start_dash_effect = true;
    //覚醒状態の時の突進アニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = CHARGE_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingCharge, false, true, blend_seconds);
    }
    //通常状態の時の突進アニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = CHARGE_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::Charge, false, true, blend_seconds);
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
    player_activity = &ClientPlayer::ChargeUpdate;

}
void ClientPlayer::TransitionAttackType1(float blend_seconds)
{
    //覚醒状態の時の１撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingAttackType1, false, true, blend_seconds);
    }
    //通常状態の時の１撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE1_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AttackType1, false, true, blend_seconds);
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
    player_activity = &ClientPlayer::AttackType1Update;
}

void ClientPlayer::TransitionAttackType2(float blend_seconds)
{

    //覚醒状態の時の２撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingAttackType2, false, true, blend_seconds);
    }
    //通常状態の時の２撃目のアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE2_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AttackType2, false, true, blend_seconds);
    }
    //攻撃中かどうかの設定
    is_attack = true;
    //アニメーション速度の設定
    animation_speed = ATTACK2_ANIMATION_SPEED;
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
    player_activity = &ClientPlayer::AttackType2Update;
}

void ClientPlayer::TransitionAttackType3(float blend_seconds)
{
    //覚醒状態の時の３撃目のアニメーションに設定
    if (is_awakening)
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_AWAIKING_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AwakingAttackType3, false, true, blend_seconds);
    }
    //通常状態の時の３撃目ののアニメーションに設定
    else
    {
        //プレイヤーの攻撃力
        player_attack_power = ATTACK_TYPE3_NORMAL_ATTACK_POWER;
        model->play_animation(anim_parm, AnimationClips::AttackType3, false, true, blend_seconds);
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
    player_activity = &ClientPlayer::AttackType3Update;
}

void ClientPlayer::TransitionSpecialSurge()
{
    //飛行機モードになるアニメーションに設定
    model->play_animation(anim_parm, AnimationClips::IdleWing, true, true);
    //ゲージ消費の突進中に当たった敵の数を初期化
    special_surge_combo_count = 0;
    //ゲージ消費の突進かどうかの設定
    is_special_surge = true;
    //攻撃中かどうかの設定
    is_attack = true;
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
    player_activity = &ClientPlayer::SpecialSurgeUpdate;
}

void ClientPlayer::TransitionOpportunity()
{
    //攻撃中かどうかの設定
    is_attack = false;
    //隙が生じた時の経過時間をリセット
    //special_surge_timer = 0;
    //アニメーションスピードの設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //ゲージ消費の突進の隙の更新関数に切り替える
    player_activity = &ClientPlayer::OpportunityUpdate;
}

void ClientPlayer::TransitionDamage()
{
    velocity = {};
    //ダッシュエフェクトの終了
    start_dash_effect = false;
    //攻撃中かどうかの設定
    is_attack = false;
    //覚醒状態の時のダメージアニメーションに設定
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingDamage, false, true, 0.0f);
    //通常状態の時のアニメーションに設定
    else model->play_animation(anim_parm, AnimationClips::Damage, false, true, 0.0f);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //ダメージ受けたときの更新関数に切り替える
    player_activity = &ClientPlayer::DamageUpdate;
}

void ClientPlayer::TransitionTransformHum()
{
    //人型になるアニメーションに設定
    model->play_animation(anim_parm, AnimationClips::TransformHum, false, true);
    //アニメーション速度の設定
    animation_speed = TRANSFORM_HUM_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //人型になってるときの更新関数に切り替える
    player_activity = &ClientPlayer::TransformHumUpdate;
}

void ClientPlayer::TransitionTransformWing()
{
    velocity = {};
    //飛行機モードになるアニメーションに設定
    model->play_animation(anim_parm, AnimationClips::TransformWing, false, true);
    //アニメーション速度の設定
    animation_speed = TRANSFORM_WING_ANIMATION_SPEED;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //飛行機モード中の更新関数に切り替える
    player_activity = &ClientPlayer::TransformWingUpdate;
}

void ClientPlayer::TransitionAwaking()
{
    invincible_timer = 2.0f;
    invincible_timer = 2.0f;
    //覚醒状態になるアニメーションに設定
    model->play_animation(anim_parm, AnimationClips::Awaking, false, true);
    //覚醒状態かどうかの設定
    is_awakening = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //覚醒状態になる途中の更新関数に切り替える
    player_activity = &ClientPlayer::AwakingUpdate;
}

void ClientPlayer::TransitionInvAwaking()
{
    invincible_timer = 2.0f;
    //通常状態に戻るアニメーションに設定
    model->play_animation(anim_parm, AnimationClips::InvAwaking, false, true);
    //覚醒状態かどうかの設定
    is_awakening = false;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //通常状態に戻ってるときの更新関数に切り替える
    player_activity = &ClientPlayer::InvAwakingUpdate;

}

void ClientPlayer::TransitionWingDashStart()
{
    model->play_animation(anim_parm, AnimationClips::WingDashStart, false, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    player_activity = &ClientPlayer::WingDashStartUpdate;

}

void ClientPlayer::TransitionWingDashIdle()
{
    model->play_animation(anim_parm, AnimationClips::WingDashIdle, true, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    player_activity = &ClientPlayer::WingDashIdleUpdate;

}

void ClientPlayer::TransitionWingDashEnd()
{
}

void ClientPlayer::TransitionDie()
{
    //攻撃中かどうかの設定
    is_attack = false;
    velocity = {};
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //覚醒状態の時のダメージアニメーションに設定
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingDie, false, true);
    //通常状態の時のアニメーションに設定
    else model->play_animation(anim_parm, AnimationClips::Die, false, true);
    //更新関数に切り替え
    player_activity = &ClientPlayer::DieUpdate;
}

void ClientPlayer::TransitionDying()
{
    //攻撃中かどうかの設定
    is_attack = false;
    velocity = {};
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //覚醒状態の時のダメージアニメーションに設定
    if (is_awakening)model->play_animation(anim_parm, AnimationClips::AwakingDying, true, true);
    //通常状態の時のアニメーションに設定
    else model->play_animation(anim_parm, AnimationClips::Dying, true, true);
    //更新関数に切り替え
    player_activity = &ClientPlayer::DyingUpdate;

}


void ClientPlayer::TransitionStartMothin()
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
    player_activity = &ClientPlayer::StartMothinUpdate;

}

void ClientPlayer::TransitionStageMove()
{
    //ステージ遷移の時に回復する
    const float health = static_cast<float>(player_health) / static_cast<float>(MAX_HEALTH);
    if (health < 0.7f) player_health = static_cast<int>(MAX_HEALTH * 0.7);
    //player_health += RECOVERY_HEALTH;
    velocity = {};
    //移動のアニメーションにする()
    model->play_animation(anim_parm, AnimationClips::TransformWing, false);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    //通常状態に戻ってるときの更新関数に切り替える
    player_activity = &ClientPlayer::StageMoveUpdate;
    during_clear = true;
}

void ClientPlayer::TransitionStageMoveEnd()
{
    model->play_animation(anim_parm, AnimationClips::WingDashEnd, false, true);
    //アニメーション速度の設定
    animation_speed = 1.0f;
    //アニメーションをしていいかどうか
    is_update_animation = true;
    player_activity = &ClientPlayer::WingDashEndUpdate;

}

