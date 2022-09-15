#pragma once

#include<memory>
#include<functional>
#include<tuple>
#include <utility>
#include<map>

#include"BasePlayer.h"
#include"ClientPlayerMove.h"
#include"collision.h"
#include"BaseEnemy.h"
#include"player_config.h"
#include"SwordTrail.h"
#include"post_effect.h"
#include"graphics_pipeline.h"
#include "Common.h"
#include"game_icon.h"
#include "effect.h"
#include "reticle.h"
#include"player_condition.h"

class ClientPlayer : public BasePlayer, private ClientPlayerMove
{
public:
    ClientPlayer(GraphicsPipeline& graphics, int object_id = 0);
    ~ClientPlayer();
private:
    enum AnimationClips
    {
        Idle,//待機
        TransformWing,//飛行機モードになる
        IdleWing,//変形待機
        TransformHum,//人型モードになる
        Move,//移動
        Avoidance,//回避
        ChargeInit,//突進開始
        Charge,//突進中
        AttackType1,//攻撃1撃目
        AttackType2,//攻撃2撃目
        AttackType3,//攻撃3撃目
        Damage,//ダメージ
        Awaking,//覚醒状態になる
        InvAwaking,//通常状態になる
        AwakingIdle,//覚醒状態の待機
        AwakingMove,//覚醒状態の移動
        AwakingAvoidance,//覚醒状態の回避
        AwakingChargeInit,//覚醒状態の突進開始
        AwakingCharge,//覚醒状態の突進
        AwakingAttackType1,//覚醒状態の攻撃1撃目
        AwakingAttackType2,//覚醒状態の攻撃2撃目
        AwakingAttackType3,//覚醒状態の攻撃3撃目
        AwakingDamage,//覚醒状態のダメージ
        WingDashStart,//飛行機モードの突進開始
        WingDashIdle,//飛行機モードの突進中
        WingDashEnd,//飛行機モードの突進終了
        Die,//死亡
        Dying,//死亡中
        AwakingDie,//死亡
        AwakingDying,//死亡中
        NamelessMotion,//モーション
        NamelessMotionIdle,//モーション
        TitleAnimationReadyIdle,//タイトルモーション1
        TitleAnimationStart,//タイトルモーション2
        TitleAnimationStartIdle,//タイトルモーション3
        TitleAnimationEnd,//タイトルモーション4
        TitleAnimationEndIdle,//タイトルモーション5
        AwaikingScene,//チュートリアルの覚醒のときに流す
        AwaikingSceneIdle
    };


    enum ActionState
    {
        //-----待機-----//
        ActionIdle,
        //-----移動-----//
        ActionMove,
        //-----回避-----//
        ActionAvoidance,
        //-----攻撃1-----//
        ActionAttack1,
        //-----攻撃2-----//
        ActionAttack2,
        //-----攻撃3-----//
        ActionAttack3,
        //-----突進開始-----//
        ActionChargeInit,
        //-----突進中-----//
        ActionCharge,
        //-----ダメージ-----//
        ActionDamage,
        //-----覚醒-----//
        ActionAwaking,
        //-----通常状態になる-----//
        ActionInvAwaking,
        //-----死亡-----//
        ActionDie,
        //-----死亡中-----//
        ActionDying,
        //-----飛行機モード待機-----//
        ActionIdleWing,
        //-----ステージ移動-----//
        ActionStageMove,
        //-----ステージ移動中-----//
        ActionStageMoveIdle,
        //-----ステージ移動終了-----//
        ActionStageMoveEnd
    };

    ////-----アニメーションのステート-----//
    ActionState action_state{ ActionState::ActionIdle };
public:
    void Initialize()override;
    void Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)override;
    bool EnemiesIsStun(std::vector<BaseEnemy*> enemies)override { return false; };
    void Render(GraphicsPipeline& graphics, float elapsed_time)override;
    void ConfigRender(GraphicsPipeline& graphics, float elapsed_time)override {};
    void ChangePlayerJustificationLength()override {};
private:
    //プレイヤーの最大体力
    static constexpr int MAX_HEALTH = 100;
    //プレイヤーの体力の回復量
    static constexpr int RECOVERY_HEALTH = 100;
    //突進時間
    static constexpr float CHARGE_MAX_TIME = 1.0f;
    //突進の進行方向変換回数
    static constexpr int CHARGE_DIRECTION_COUNT = 3;
    //攻撃1撃目の猶予時間
    static constexpr float ATTACK_TYPE1_MAX_TIME = 0.3f;
    //攻撃2撃目の猶予時間
    static constexpr float ATTACK_TYPE2_MAX_TIME = 0.2f;
    //攻撃3撃目の猶予時間
    static constexpr float ATTACK_TYPE3_MAX_TIME = 0.2f;
    //ロックオンできる距離
    static constexpr float LOCK_ON_LANGE = 100.0f;
public:
    //後ろに回り込める距離
    static constexpr float  BEHIND_LANGE_MAX = 45.0f;
    static constexpr float  BEHIND_LANGE_MIN = 5.0f;
    //-----プレイヤーの入力値の最小値-----//
    static constexpr float PLAYER_INPUT_MIN = 2.0f;
private:
    //コンボの最大数
    static constexpr float MAX_COMBO_COUNT = 80.0f;
    //回避の時のアニメーションスピード
    static constexpr float AVOIDANCE_ANIMATION_SPEED = 1.5f;
    //突進開始の時のアニメーションスピード
    static constexpr float CHARGEINIT_ANIMATION_SPEED = 5.5f;
    //突進の時のアニメーションスピード
    static constexpr float CHARGE_ANIMATION_SPEED = 2.0f;
    //攻撃1の時のアニメーションスピード
    static constexpr float ATTACK1_ANIMATION_SPEED = 2.5f;
    //攻撃2の時のアニメーションスピード
    static constexpr float ATTACK2_ANIMATION_SPEED = 4.5f;
    //攻撃3の時のアニメーションスピード
    static constexpr float ATTACK3_ANIMATION_SPEED = 4.5f;
    //ゲージ消費量(突進)
    static constexpr float GAUGE_CONSUMPTION = 5.0f;
    //人型に戻るときのアニメーションスピード
    static constexpr float TRANSFORM_HUM_ANIMATION_SPEED = 2.0f;
    //飛行機モードになるときのアニメーションスピード
    static constexpr float TRANSFORM_WING_ANIMATION_SPEED = 2.0f;
    //チェイン攻撃の時にコンボゲージが減る量
    static constexpr float COMBO_COUNT_SUB = 7.0f;
    //プレイヤーが攻撃中にダメージを受けたときの減少量
    static constexpr int ATTACK_DAMAGE_INV = 1;
    //プレイヤーが回避中にダメージを受けたときの減少量
    static constexpr int AVOIDANCE_DAMAGE_INV = 2;
    //プレイヤーのジャスト回避の回復
    static constexpr int JUST_AVOIDANCE_HEALTH = 10;
    //ジャスト回避の時のコンボゲージ回復
    static constexpr float JUST_AVOIDANCE_COMBO = 5.0f;
    //プレイヤーの突進の攻撃力
    static constexpr int CHARGE_NORMAL_ATTACK_POWER = 1;
    static constexpr int CHARGE_AWAIKING_ATTACK_POWER = 1;
    //1撃目の攻撃力
    static constexpr int ATTACK_TYPE1_NORMAL_ATTACK_POWER = 1;
    static constexpr int ATTACK_TYPE1_AWAIKING_ATTACK_POWER = 4;
    //2撃目の攻撃力
    static constexpr int ATTACK_TYPE2_NORMAL_ATTACK_POWER = 2;
    static constexpr int ATTACK_TYPE2_AWAIKING_ATTACK_POWER = 6;
    //3撃目の攻撃力
    static constexpr int ATTACK_TYPE3_NORMAL_ATTACK_POWER = 4;
    static constexpr int ATTACK_TYPE3_AWAIKING_ATTACK_POWER = 9;
private:
    //敵に当たったかどうか
    bool is_enemy_hit{ false };
private:
    //突進中の時間(突進が始まったらタイマーが動く)
    float charge_time{ 0 };
    //突進中の時間をどれだけ増やすか
    float charge_add_time{ 1.0f };
    //突進中の進行方向変換回数
    int charge_change_direction_count{ CHARGE_DIRECTION_COUNT };
    //攻撃の時間
    float attack_time{ 0 };
    //攻撃中の時間をどれだけ増やすか
    float attack_add_time{ 1.0f };
private:
    //回避が始まった時間
    float avoidance_boost_time{ 0 };
    //回避の方向転換の回数
    int avoidance_direction_count{ 3 };
    //回避のボタンを押して離したかどうか
    bool avoidance_buttun{ false };
    //回避力
    float  avoidance_velocity{ 15.0f };
    //回避のアニメーションをスタートする
    bool avoidance_animation_start{ false };
    //イージングの効果時間
    float easing_time{ 1.0f };
    //回避ブースト開始と到達
    DirectX::XMFLOAT3 avoidance_start{};
    DirectX::XMFLOAT3 avoidance_end{};
    float avoidance_easing_time{ 0.6f };
    //ブーストの倍率
    float leverage{ 15.0f };
    //カメラ用の回避した瞬間
    bool is_avoidance{ false };
    //回り込み回避かどうか
    bool is_behind_avoidance{ false };
    //回り込み回避のクールタイム
    float behaind_avoidance_cool_time{ 0 };
    //クールタイムが解除されたかどうか true :解除されていない
    bool behaind_avoidance_recharge{ false };
    //ジャスト回避しているかどうか
    bool is_just_avoidance{ false };
    //ジャスト回避できるカプセルに入っているかどうか
    bool is_just_avoidance_capsul{ false };
    //倒した敵の位置を保存
    DirectX::XMFLOAT3 old_target{};
    //カメラの補間のゴール地点
    DirectX::XMFLOAT3 end_target{};
    //補間率
    float target_lerp_rate{ 0 };
    //ロックオンしていないときのカメラのターゲット
    DirectX::XMFLOAT3 camera_target{};
private:
    //ターゲットの敵
    BaseEnemy* target_enemy;
    float enemy_length{ 0 };//敵とプレイヤーの距離
    int target_count{ 0 };
    int old_target_count{ 0 };
private:
    //プレイヤーの攻撃力(コンボによって変化していく)
    int player_attack_power{ 3 };
    //コンボ数
    float combo_count{ 0 };
    //コンボゲージSE
    bool combo_max_se{ false };
    //ゲージ消費の突進中に当たった数
    float special_surge_combo_count{ 0 };
    //プレイヤーが今攻撃中かそうでないか
    bool is_attack{ false };
    //プレイヤーげゲージ消費の突進をしているか
    bool is_special_surge{ false };
    //ゲージ消費の突進の隙
    float special_surge_opportunity{ 1.5f };
    //float special_surge_timer{ 0 };
    float opportunity_timer{ 0 };
    //プレイヤーの体力
    int player_health = MAX_HEALTH;
    //プレイヤー死んだかどうか
    bool is_alive{ true };
    //無敵時間
    float invincible_timer{};
    //コンボの持続時間
    float duration_combo_timer{};
    //ロックオンしていないときの突進のターゲット
    DirectX::XMFLOAT3 charge_point{};
    //ダッシュエフェクト開始
    bool start_dash_effect{ false };
    //ダッシュエフェクト終了
    bool end_dash_effect{ false };
    //覚醒状態かどうか
    bool is_awakening{ false };
    //突進中かどうか
    bool is_charge{ false };
    //ブロックされたかどうか
    bool is_block{ false };
    //プレイヤーのアニメーションスピード
    float animation_speed{ 1.0f };
    //突進の加速用のvelocity
    DirectX::XMFLOAT3 acceleration_velocity;
    //突進のvelocityの補間のレート
    float lerp_rate{ 1.0f };
    //突進のターゲットまでの距離の倍率(どれだけ伸ばすか)
    float charge_length_magnification{ 100.0f };
    //攻撃のアニメーションスピードのデバッグ用
    DirectX::XMFLOAT4 attack_animation_speeds{ 1.0f,1.0f,1.0f,1.0f };
    //攻撃のアニメーション補間の時間
    DirectX::XMFLOAT4 attack_animation_blends_speeds{ 0.3f,0.0f,0.0f,0.0f };
    //アニメーションをしていいかどうか
    bool is_update_animation{ true };
    //クリア演出中
    bool during_clear{ false };
    //ボスの演出中
    bool boss_camera{ false };
    //プレイヤーのパラメータ
    std::unique_ptr<PlayerConfig> player_config{ nullptr };
    std::unique_ptr<PlayerCondition> player_condition{ nullptr };
    //--------------------<SwordTrail～剣の軌跡～>--------------------//
    SwordTrail mSwordTrail[2]{};
    float mTrailEraseTimer{};

    skeleton::bone player_bones[12];
private:
    float sphere_radius{ 0.0f };
    CapsuleParam sword_capsule_param[2]{};
    CapsuleParam charge_capsule_param{};
    CapsuleParam body_capsule_param{};
    CapsuleParam just_avoidance_capsule_param{};
    DirectX::XMFLOAT3 step_pos_r{};
    DirectX::XMFLOAT3 step_pos_l{};
    DirectX::XMFLOAT3 capsule_body_start{ 0,2.6f,0 };
    DirectX::XMFLOAT3 capsule_body_end{ 0,0.2f,0 };
    //足元のカプセル
    void StepCapsule();
    void BodyCapsule();
    //剣のカプセル判定
    void SwordCapsule();
    //範囲スタンのパラメータ設定
    void StunSphere();

    //-----パラメータの更新-----//
    void InflectionParameters(float elapesd_time);

    enum class ConditionState
    {
        Alive,//生きている
        Die//死んでいる
    };
    //生きているかどうか
    ConditionState condition_state{ ConditionState::Alive };


    void SetEndDashEffect(bool a)override { end_dash_effect = a; }
    void SetCameraTarget(DirectX::XMFLOAT3 p)override { camera_target = p; }
    void SetBossCamera(bool boss_c) override { boss_camera = boss_c; }
    void SetPosition(DirectX::XMFLOAT3 pos) override { position = pos; }
    DirectX::XMFLOAT3 GetForward()override { return {}; }
    DirectX::XMFLOAT3 GetRight()override { return {}; }
    DirectX::XMFLOAT3 GetUp()override { return {}; }
    DirectX::XMFLOAT3 GetPosition()override { return position; }
    DirectX::XMFLOAT3 GetVelocity()override { return velocity; }
    HitResult& GetPlayerHitResult()override { return hit; }
    bool GetCameraReset()override { return false; }
    bool GetCameraLockOn()override { return false; }
    bool GetEnemyLockOn()override { return is_lock_on; }
    bool GetAvoidance()override { return is_avoidance; }
    bool GetBehindAvoidance()override { return is_behind_avoidance; }
    bool GetIsPlayerAttack()override { return is_attack; }
    bool GetIsCharge()override { return is_charge; }
    bool GetIsSpecialSurge() override { return is_special_surge; }
    bool GetStartDashEffect() override { return start_dash_effect; }
    bool GetEndDashEffect() override { return end_dash_effect; }
    bool GetIsAwakening() override { return is_awakening; }
    bool GetIsAlive() override { return is_alive; }
    bool GetIsJustAvoidance() override { return is_just_avoidance; }
    bool GetBehaindCharge() override { return behaind_avoidance_recharge; }
    CapsuleParam GetBodyCapsuleParam() override { return body_capsule_param; }
    CapsuleParam GetJustAvoidanceCapsuleParam() override { return just_avoidance_capsule_param; }
    CapsuleParam GetSwordCapsuleParam(int i)override
    {
        //もし突進中なら突進中の当たり判定を返す
        if (is_charge)
        {
            return charge_capsule_param;
        }
        //覚醒状態なら引数で受け取った値を渡す
        if (is_awakening)
        {
            return sword_capsule_param[i];
        }
        //普通の剣の位置を渡す
        return sword_capsule_param[0];
    }
    float GetStunRadius() override { return sphere_radius; }
    std::vector<DirectX::XMFLOAT3> GetBehindPoint() override { return {}; }
    void SetRaycast(bool r) override { raycast = r; }
    int GetPlayerPower() override { return player_attack_power; }
    [[nodiscard("Not used")]] const AddDamageFunc GetDamagedFunc() override { return damage_func; }

    BaseEnemy* GetPlayerTargetEnemy() const  override
    {
        if (target_enemy != nullptr && target_enemy->fComputeAndGetIntoCamera())
        {
            return target_enemy;
        }
        return nullptr;
    }
    //一番近い敵を持って来てその位置をセットする
    void SetTarget(BaseEnemy* target_enemy)override {};
    DirectX::XMFLOAT3 GetTarget() override { return target; };
    void AddCombo(int count, bool& block)override;
    //覚醒状態の時は２つ当たり判定があるから引数が２つ
    void AwakingAddCombo(int hit_count1, int hit_count2, bool& block)override;
    //--------------------<敵からダメージを受ける>--------------------//
    void DamagedCheck(int damage, float InvincibleTime)override {};
    void TutorialDamagedCheck(int damage, float InvincibleTime)override {};
    void PlayerKnocKback(float elapsed_time)override {};
    //プレイヤーのジャスト回避用の当たり判定に当たったら
    void PlayerJustAvoidance(bool hit)override {};
public:
    void FalseCameraReset() override {}
    void FalseCameraLockOn() override {}
    void FalseAvoidance() override {}
    DirectX::XMFLOAT3 GetEnentCameraEye()override { return {}; };
    DirectX::XMFLOAT3 GetEnentCameraJoint()override { return {}; };
    bool GetEndClearMotion()override { return false; };
    bool GetStartClearMotion()override { return false; };
    void PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)override {};
public:
    bool during_search_time()override { return false; }
    bool during_chain_attack_end()override { return false; }
    bool during_chain_attack()override { return false; }
    void lockon_post_effect(float elapsed_time, std::function<void(float, float)> effect_func, std::function<void()> effect_clear_func) override {}

private:
    void GetPlayerDirections();
public:
    void SetCameraDirection(const DirectX::XMFLOAT3& c_forward, const DirectX::XMFLOAT3& c_right) override{}
    void SetCameraPosition(DirectX::XMFLOAT3 p) override {}
private:
        AddDamageFunc damage_func;
        bool display_scape_imgui;

private:
    ////-----アニメーションの遷移に関する関数が入る-----//
    using TransitionAnimation = std::function <void(void)>;

    ////-----アニメーション中の更新処理の関数が入る-----//
    using UpdateAnimation = std::function <void(float, SkyDome*)>;

    ////-----アニメーション中の更新処理-----//
    UpdateAnimation update_animation{ [=](float elapsed_time,SkyDome* sky_dome) {IdleUpdate(elapsed_time,sky_dome); } };

    ////-----遷移元のアニメーションと遷移先のアニメーションの番号のペア-----//
    //=========================================================
    //tuple構造 : [遷移元 , 遷移先]
    using TransEdge = std::tuple<int, int>;

    ////-----今のアニメーションから遷移できるアニメーションを保存する変数-----//
    //===========================================================
    //マップの構造 : [今のアニメーション,[遷移先のアニメーション,遷移先のアニメーションにいく条件の関数]]
    using TransitionAnimationsMap = std::map<int, std::map<int, TransitionAnimation>>;

    ////-----遷移先のアニメーションをまとめたmap-----//
    TransitionAnimationsMap transition_animations;

    ////----------遷移元のアニメーションと遷移先のアニメーションの関数を結びつける(1つだけ)----------//
    //===================================================================
    //第1引数 : 元のアニメーション番号と遷移先のアニメーション番号のTuple
    //第2引数 : 遷移先の条件関数
    void RegisterTransitionAnimationMap(TransEdge tuples, TransitionAnimation func);

    ////----------遷移元ののアニメーション遷移先のアニメーションの関数を結びつける(複数)----------//
    ////-----std::functionの型に関数を登録するときはラムダ式でキャプチャしてあげる-----//
    //========================================================================
    //ラムダ式 : [=]()->bool{ return 遷移関数;}
    template<class... Tuples>
    void RegisterTransitionAnimations(TransitionAnimation func, Tuples&&... tuples)
    {
        //-----TransEdge型であることを保証している-----//
        //-----std::forwardはinitializer_listを使うときにセットで使うもので可変長引数の先頭を取得する-----//
        for (const auto trans_edge : std::initializer_list<TransEdge>{ std::forward<Tuples>(tuples)... })
        {
            RegisterTransitionAnimationMap(trans_edge, func);
        }
    }

    ////----------遷移関数を登録する----------//
    void RegisterAnimationFunctions();

    ////----------遷移関数のmapを回す----------//
    //==============================
    //第1引数 : 今のアニメーション番号
    void ActivationTransitionMap(int root_animation_index);
public:
    //待機に遷移
    void TransitionIdle(float blend_second) override {};
private:
    void TransitionIdle();
    //移動に遷移
    void TransitionMove();
    //回避に遷移
    void TransitionAvoidance();
    //突進開始に遷移
    void TransitionChargeInit();
    //突進に遷移
    void TransitionCharge();
    //１撃目に遷移
    void TransitionAttackType1();
    //２撃目に遷移
    void TransitionAttackType2();
    //３撃目に遷移
    void TransitionAttackType3();
    //ダメージ受けたときに遷移
    void TransitionDamage();
    //覚醒状態に遷移
    void TransitionAwaking();
    //通常状態に遷移
    void TransitionInvAwaking();
    //死亡
    void TransitionDie();
    //死亡中
    void TransitionDying();
    //ステージ移動中
    void TransitionStageMoveIdle();
private:
    //待機アニメーション中の更新処理
    void IdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //移動アニメーション中の更新処理
    void MoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //回避アニメーション中の更新処理
    void AvoidanceUpdate(float elapsed_time, SkyDome* sky_dome);
    //突進開始アニメーション中の更新処理
    void ChargeInitUpdate(float elapsed_time, SkyDome* sky_dome);
    //突進中の更新処理
    void ChargeUpdate(float elapsed_time, SkyDome* sky_dome);
    //攻撃1撃目の更新処理
    void AttackType1Update(float elapsed_time, SkyDome* sky_dome);
    //攻撃2撃目の更新処理
    void AttackType2Update(float elapsed_time, SkyDome* sky_dome);
    //攻撃3撃目の更新処理
    void AttackType3Update(float elapsed_time, SkyDome* sky_dome);
    //ダメージ受けたとき
    void DamageUpdate(float elapsed_time, SkyDome* sky_dome);
    //覚醒状態に変形するときの更新
    void AwakingUpdate(float elapsed_time, SkyDome* sky_dome);
    //通常状態に変形するときの更新
    void InvAwakingUpdate(float elapsed_time, SkyDome* sky_dome);
    //ステージ移動の時の更新
    void StageMoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //ステージ移動中の待機
    void StageMoveIdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //ステージ移動の時の更新
    void StageMoveEndUpdate(float elapsed_time, SkyDome* sky_dome);
    //死亡
    void DieUpdate(float elapsed_time, SkyDome* sky_dome);
    //死亡中
    void DyingUpdate(float elapsed_time, SkyDome* sky_dome);
public:
    //スタートモーション
    void TransitionStartMothin()override {};
    //ステージ移動に遷移
    void TransitionStageMove()override;
    //ステージ遷移終了
    void TransitionStageMoveEnd()override;

};