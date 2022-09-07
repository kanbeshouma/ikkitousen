#pragma once
#include<memory>
#include <map>
#include"BasePlayer.h"
#include"PlayerMove.h"
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
enum class SePriset : uint16_t
{
    None = (0 << 0),
    First = (1 << 0),
    Second = (2 << 0),
    Third = (3 << 0),
    Forth = (4 << 0),
    Fifth = (5 << 0),
    Sixth = (6 << 0),
    Seventh = (7 << 0),

    Se0 = None,
    Se1 = First,
    Se2 = First | Second,
    Se3 = First | Second | Third,
    Se4 = First | Second | Third | Forth,
    Se5 = First | Second | Third | Forth | Fifth,
    Se6 = First | Second | Third | Forth | Fifth | Sixth,
    Se7 = First | Second | Third | Forth | Fifth | Sixth | Seventh,

};
inline bool operator&(SePriset lhs, SePriset rhs)
{
    return static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs);
}

class Player :public BasePlayer, private PlayerMove
{
public:
    Player(GraphicsPipeline& graphics);
    ~Player();
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
public:
    void Initialize()override;
    void Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)override;
    //チュートリアル用のアップデート
    void UpdateTutorial(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    //タイトル用のアップデート
    void UpdateTitle(float elapsed_time);
    //スタンしている敵がいるかどうか
    bool EnemiesIsStun(std::vector<BaseEnemy*> enemies);
    void Render(GraphicsPipeline& graphics, float elapsed_time)override;
    void ConfigRender(GraphicsPipeline& graphics, float elapsed_time);
    void TutorialConfigRender(GraphicsPipeline& graphics, float elapsed_time, bool condition);
    void TitleRender(GraphicsPipeline& graphics, float elapsed_time);
    void ChangePlayerJustificationLength();
private:
    //エフェクト
    //回り込み回避
    std::unique_ptr<Effect> player_behind_effec;
    //回避
    std::unique_ptr<Effect> player_air_registance_effec;
    float air_registance_offset_y{4.0f};
    //チェイン攻撃のヒット
    std::unique_ptr<Effect> player_slash_hit_effec;
    DirectX::XMFLOAT3 slash_effec_pos{};
    std::unique_ptr<Effect> player_awaiking_effec;
    std::unique_ptr<Effect> just_stun;
    std::unique_ptr<Effect> player_move_effec_r;
    std::unique_ptr<Effect> player_move_effec_l;
    std::unique_ptr<Effect> player_behaind_effec_2;

    //USE_IMGUI
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
    DirectX::XMFLOAT3 camera_forward{};//カメラの前方向
    DirectX::XMFLOAT3 camera_right{};//カメラの右方向
    DirectX::XMFLOAT3 camera_position{};//カメラの右方向
private:
    //プレイヤーの各方向
    DirectX::XMFLOAT3 forward;
    DirectX::XMFLOAT3 right;
    DirectX::XMFLOAT3 up;
private:
    //カメラの向きをリセット
    bool camera_reset{ false };
    //敵をロックオンしたかどうか(カメラ用のロックオンしたかどうか)
    bool is_camera_lock_on{ false };
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
    //カメラのターゲットの補間
    void LerpCameraTarget(float elapsed_time);
private:
    //ターゲットの敵
    BaseEnemy* target_enemy;
    float enemy_length{ 0 };//敵とプレイヤーの距離
    int target_count{ 0 };
    int old_target_count{ 0 };
private:
    //後ろに回り込むための計算する関数
    void BehindAvoidancePosition();
    //スプライン曲線を使うための途中の点
    DirectX::XMFLOAT3 behind_point_0{};//スタート
    DirectX::XMFLOAT3 behind_point_1{};//中継地点
    DirectX::XMFLOAT3 behind_point_2{};//中継地点
    DirectX::XMFLOAT3 behind_point_3{};//ゴール

       //回り込むスピード
    float behind_speed{ 0.0f };
    float behind_test_timer{ 0.0f };

    int behind_transit_index = 0;
    std::vector<DirectX::XMFLOAT3> behind_way_points;
    std::vector<DirectX::XMFLOAT3> behind_interpolated_way_points;

    //背後に回り込む点
    std::vector<DirectX::XMFLOAT3> behind_point{};
    bool BehindAvoidanceMove(float elapsed_time, int& index, DirectX::XMFLOAT3& position, float speed,
        const std::vector<DirectX::XMFLOAT3>& points, float play);
    //背後に回り込むときに進むタイマー
    float behind_timer{};
    //背後に回り込むときのレート
    float behind_late{};
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
    //プレイヤーのパラメータの変化
    void InflectionParameters(float elpased_time);
    void TutorialInflectionParameters(float elpased_time);
    //攻撃力の変化
    void InflectionPower(float elapsed_time);
    //コンボの変化
    void InflectionCombo(float elapsed_time);
    //死んでるかどうか
    void PlayerAlive();
    //チュートリアルの時の死んでるかどうか
    void TutorialPlayerAlive();
private:
    //カプセル敵との当たり判定
    struct CapsuleParam
    {
        DirectX::XMFLOAT3 start{};
        DirectX::XMFLOAT3 end{};
        float rasius{ 1.2f };
    };

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
    enum class ConditionState
    {
        Alive,//生きている
        Die//死んでいる
    };
    //生きているかどうか
    ConditionState condition_state{ ConditionState::Alive };
public:
    void SetEndDashEffect(bool a) { end_dash_effect = a; }
    void SetCameraTarget(DirectX::XMFLOAT3 p) { camera_target = p; }
    void SetBossCamera(bool boss_c) { boss_camera = boss_c; }
    void SetPosition(DirectX::XMFLOAT3 pos) { position = pos; }
    DirectX::XMFLOAT3 GetForward() { return forward; }
    DirectX::XMFLOAT3 GetRight() { return right; }
    DirectX::XMFLOAT3 GetUp() { return up; }
    DirectX::XMFLOAT3 GetPosition() { return position; }
    DirectX::XMFLOAT3 GetVelocity() { return velocity; }
    HitResult& GetPlayerHitResult() { return hit; }
    bool GetCameraReset() { return camera_reset; }
    bool GetCameraLockOn() { return is_camera_lock_on; }
    bool GetEnemyLockOn() { return is_lock_on; }
    bool GetAvoidance() { return is_avoidance; }
    bool GetBehindAvoidance() { return is_behind_avoidance; }
    bool GetIsPlayerAttack() { return is_attack; }
    bool GetIsCharge() { return is_charge; }
    bool GetIsSpecialSurge() { return is_special_surge; }
    bool GetStartDashEffect() { return start_dash_effect; }
    bool GetEndDashEffect() { return end_dash_effect; }
    bool GetIsAwakening() { return is_awakening; }
    bool GetIsAlive() { return is_alive; }
    bool GetIsJustAvoidance() { return is_just_avoidance; }
    bool GetBehaindCharge() { return behaind_avoidance_recharge;}
    CapsuleParam GetBodyCapsuleParam() { return body_capsule_param; }
    CapsuleParam GetJustAvoidanceCapsuleParam() { return just_avoidance_capsule_param; }
    CapsuleParam GetSwordCapsuleParam(int i)
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
    float GetStunRadius() { return sphere_radius; }
    std::vector<DirectX::XMFLOAT3> GetBehindPoint() { return behind_point; }
    void SetRaycast(bool r) { raycast = r; }
    int GetPlayerPower() { return player_attack_power; }
    [[nodiscard("Not used")]] const AddDamageFunc GetDamagedFunc() { return damage_func; }

    [[nodiscard]] const  BaseEnemy* GetPlayerTargetEnemy() const
    {
        if (target_enemy != nullptr && target_enemy->fComputeAndGetIntoCamera())
        {
            return target_enemy;
        }
        return nullptr;
    }
    //一番近い敵を持って来てその位置をセットする
    void SetTarget(BaseEnemy* target_enemy);
    DirectX::XMFLOAT3 GetTarget() { return target; };
    void AddCombo(int count, bool& block);
    //覚醒状態の時は２つ当たり判定があるから引数が２つ
    void AwakingAddCombo(int hit_count1, int hit_count2, bool& block);
    //--------------------<敵からダメージを受ける>--------------------//
    void DamagedCheck(int damage, float InvincibleTime);
    void TutorialDamagedCheck(int damage, float InvincibleTime);
    void PlayerKnocKback(float elapsed_time);
    //プレイヤーのジャスト回避用の当たり判定に当たったら
    void PlayerJustAvoidance(bool hit);
public:
    void FalseCameraReset() { camera_reset = false; }
    void FalseCameraLockOn() { is_camera_lock_on = false; }
    void FalseAvoidance() { is_avoidance = false; }
private:
    void GetPlayerDirections();
public:
    void SetCameraDirection(const DirectX::XMFLOAT3& c_forward, const DirectX::XMFLOAT3& c_right)
    {
        camera_forward = c_forward;
        camera_right = c_right;
    }
    void SetCameraPosition(DirectX::XMFLOAT3 p) { camera_position = p; }
private:
    //回避の加速
    void AvoidanceAcceleration(float elapse_time);
    //突進の加速(線形補間)SetAccelerationVelocityで動くように変更した
    void ChargeAcceleration(float elapse_time);
    //攻撃の加速の設定
    void SetAccelerationVelocity();
    //ゲージ消費の突進
    void SpecialSurgeAcceleration();
private:
    //ロックオン
    void LockOn();
    //チュートリアルでのロックオン(処理は変わらない)
    void TutorialLockOn();
    //チェイン攻撃の時のロックオン
    void ChainLockOn();
    //カメラリセット
    void CameraReset();
private:
    enum class Behavior
    {
        //通常状態
        Normal,
        //スタンした敵に攻撃する状態
        Chain
    };
    Behavior behavior_state{ Behavior::Normal };
private:
    typedef void(Player::* PlayerTitleActivity)(float elapsed_time);
    PlayerTitleActivity player_title_activity = &Player::UpdateTitleAnimationReadyIdle;
    void ExecFuncUpdate(float elapsed_time);
    //タイトル用アニメーションを再生
    bool start_title_animation{ false };
    //タイトル用アニメーションが終わった時
    bool end_title_animation{ false };
    //タイトル用タイマー
    float title_timer{ 0 };
    //タイトル用待機
    void UpdateTitleAnimationReadyIdle(float elaosed_time);
    void UpdateTitleAnimationStart(float elaosed_time);
    void UpdateTitleAnimationStartIdle(float elaosed_time);
    void UpdateTitleAnimationEnd(float elaosed_time);
    void UpdateTitleAnimationEndIdle(float elaosed_time);
public:
    void TransitionTitleAnimationReadyIdle();
    void StartTitleAnimation() { start_title_animation = true; }
    bool GetStartTitleAnimation() { return start_title_animation; }
    bool GetEndTitleAnimation() { return end_title_animation; }
private:
    //-----------アニメーションに関係する関数,変数------------//
    //アニメーション遷移の関数ポインタ//
    //関数ポインタ
    typedef void(Player::* PlayerActivity)(float elapsed_time, SkyDome* sky_dome);
    //関数ポインタの変数
    PlayerActivity player_activity = &Player::IdleUpdate;
    AddDamageFunc damage_func;
    //自分のメンバ関数の関数ポインタを呼ぶ
    void ExecFuncUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    //待機アニメーション中の更新処理
    void IdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //移動アニメーション中の更新処理
    void MoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //回避アニメーション中の更新処理
    void AvoidanceUpdate(float elapsed_time, SkyDome* sky_dome);
    //後ろに回り込む回避の更新処理
    void BehindAvoidanceUpdate(float elapsed_time, SkyDome* sky_dome);
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
    //ゲージ消費する突進
    void SpecialSurgeUpdate(float elapsed_time, SkyDome* sky_dome);
    //ゲージ消費突進が終わってからの隙
    void OpportunityUpdate(float elapsed_time, SkyDome* sky_dome);
    //ダメージ受けたとき
    void DamageUpdate(float elapsed_time, SkyDome* sky_dome);
    //人型に戻る
    void TransformHumUpdate(float elapsed_time, SkyDome* sky_dome);
    //飛行機モード
    void TransformWingUpdate(float elapsed_time, SkyDome* sky_dome);
    //覚醒状態に変形するときの更新
    void AwakingUpdate(float elapsed_time, SkyDome* sky_dome);
    //通常状態に変形するときの更新
    void InvAwakingUpdate(float elapsed_time, SkyDome* sky_dome);
    //ステージ移動の時の更新
    void StageMoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //飛行機モードの突進開始
    void WingDashStartUpdate(float elapsed_time, SkyDome* sky_dome);
    //飛行機モードの突進中
    void WingDashIdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //飛行機モードの突進終了
    void WingDashEndUpdate(float elapsed_time, SkyDome* sky_dome);
    //死亡
    void DieUpdate(float elapsed_time, SkyDome* sky_dome);
    //死亡中
    void DyingUpdate(float elapsed_time, SkyDome* sky_dome);
    //死亡の更新処理に入ったらtrue
    bool is_dying_update{ false };
    //モーション
    void StartMothinUpdate(float elapsed_time, SkyDome* sky_dome);


    void Awaiking();//覚醒状態のON,OFF
    //アニメーション遷移(1frameだけしか呼ばないもの)
public:
    //待機に遷移
    void TransitionIdle(float blend_second = 0.3f);
private:
    //移動に遷移
    void TransitionMove(float blend_second = 0.3f);
    //回避に遷移
    void TransitionAvoidance();
    //背後に回り込む回避に遷移
    void TransitionBehindAvoidance();
    //ジャスト回避の回り込み回避に遷移
    void TransitionJustBehindAvoidance();
    //突進開始に遷移
    void TransitionChargeInit();
    //突進に遷移
    void TransitionCharge(float blend_seconds = 0.3f);
    //１撃目に遷移
    void TransitionAttackType1(float blend_seconds = 0.3f);
    //２撃目に遷移
    void TransitionAttackType2(float blend_seconds = 0.3f);
    //３撃目に遷移
    void TransitionAttackType3(float blend_seconds = 0.3f);
    //ゲージ消費の突進に遷移
    void TransitionSpecialSurge();
    //ゲージ消費の突進後の隙に遷移
    void TransitionOpportunity();
    //ダメージ受けたときに遷移
    void TransitionDamage();
    //飛行機モードに遷移
    void TransitionTransformWing();
    //人型に変形に遷移
    void TransitionTransformHum();
    //覚醒状態に遷移
    void TransitionAwaking();
    //通常状態に遷移
    void TransitionInvAwaking();
    //飛行機モードの突進開始
    void TransitionWingDashStart();
    //飛行機モードの突進中
    void TransitionWingDashIdle();
    //飛行機モードの突進終了
    void TransitionWingDashEnd();
    //死亡
    void TransitionDie();
    //死亡中
    void TransitionDying();

public:
    //スタートモーション
    void TransitionStartMothin();
    //ステージ移動に遷移
    void TransitionStageMove();
    //ステージ遷移終了
    void TransitionStageMoveEnd();
    //クリアに関すること
private:
    //モーション
    void NamelessMotionUpdate(float elapsed_time, SkyDome* sky_dome);
    void NamelessMotionIdleUpdate(float elapsed_time, SkyDome* sky_dome);
    void TransitionNamelessMotionIdle();
    //クリアモーションの時の音
    int nameless_motion_se_state = 0;
    //クリアモーションに遷移
    void TransitionNamelessMotion();
    //クリア用モーションが終わったらtrue
    bool is_end_clear_motion{ false };
    //クリア用モーションが始まったらtrue
    bool is_start_cleear_motion{ false };
    //イベントシーンの黒の枠
    float wipe_parm{ 0.0f };
    DirectX::XMFLOAT3 event_camera_eye{ 0,3.4f,0.0f };
    DirectX::XMFLOAT3 event_camera_joint{};
public:
    DirectX::XMFLOAT3 GetEnentCameraEye() { return event_camera_eye; }
    DirectX::XMFLOAT3 GetEnentCameraJoint() { return event_camera_joint; }
    bool GetEndClearMotion() { return is_end_clear_motion; }
    bool GetStartClearMotion() { return is_start_cleear_motion; }
    void PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
private:
    //関数ポインタ
    typedef void(Player::* PlayerChainMoveActivity)(float elapsed_time, SkyDome* sky_dome);
    PlayerChainMoveActivity chain_activity = &Player::ChainIdleUpdate;
    //待機アニメーション中の更新処理
    void ChainIdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //移動アニメーション中の更新処理
    void ChainMoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //待機に遷移
    void TransitionChainIdle(float blend_second = 0.3f);
    //移動に遷移
    void TransitionChainMove(float blend_second = 0.3f);


private:
    //--------<藤岡パート>--------//
    //関数ポインタ
    typedef void(Player::* PlayerChainActivity)(float elapsed_time, std::vector<BaseEnemy*> enemies,GraphicsPipeline& Graphics_);
    //関数ポインタの変数
    PlayerChainActivity player_chain_activity = &Player::chain_search_update;
    // 索敵
    void chain_search_update(float elapsed_time, std::vector<BaseEnemy*> enemies,
        GraphicsPipeline& graphics_);
    void transition_chain_search();
    // ロックオン準備
    void chain_lockon_begin_update(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    void transition_chain_lockon_begin();
    // ロックオン
    void chain_lockon_update(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    void transition_chain_lockon();
    // 移動
    void chain_move_update(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    void transition_chain_move();
    // 攻撃
    void chain_attack_update(float elapsed_time, std::vector<BaseEnemy*> enemies,
        GraphicsPipeline& Graphics_);
    void transition_chain_attack();
    // 指定したポイント全てを通る関数
    bool transit(float elapsed_time, int& index, DirectX::XMFLOAT3& position,
        float speed, const std::vector<DirectX::XMFLOAT3>& points, float play = 0.01f);
    // 進んでる方向に回転する関数
    void rotate(float elapsed_time, int index, const std::vector<DirectX::XMFLOAT3>& points);
    // behaviorの遷移関数
    void transition_chain_behavior()
    {
        player_move_effec_r->stop(effect_manager->get_effekseer_manager());
        player_move_effec_l->stop(effect_manager->get_effekseer_manager());
        player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
        behavior_state = Behavior::Chain;
        transition_chain_search();
    }
    void transition_normal_behavior()
    {
        PostEffect::clear_post_effect();
        behavior_state = Behavior::Normal;
        if (is_tutorial)TransitionTutoriaIdle();
        else TransitionIdle();
    }

    void chain_parm_reset();
    float change_normal_timer{ 0.0f };
public:
    bool during_search_time() { return search_time < SEARCH_TIME && search_time > 0; }
    bool during_chain_attack_end() { return behavior_state == Behavior::Chain && is_chain_attack; }  // ロックオン完了から攻撃終了までtrue
    bool during_chain_attack() { return is_chain_attack_aftertaste; }  // ロックオン完了から攻撃終了後カメラが追いついたあとちょっと待ってtrue
    void lockon_post_effect(float elapsed_time, std::function<void(float, float)> effect_func, std::function<void()> effect_clear_func);
private:
    //--------< 変数 >--------//
    struct LockOnSuggest
    {
        DirectX::XMFLOAT3 position{};
        bool detection = false;
    };
    static constexpr int STEPS = 3;
    static constexpr float AddAttackEndCameraTimer = 1.0f;

    static constexpr float CHRONOSTASIS_TIME = 0.3f;
    float chronostasis_scope = 0.8f;
    float chronostasis_saturation = 1.0f;


    float SEARCH_TIME = 0.5f;
    bool setup_search_time = false;
    float search_time = SEARCH_TIME;
    int transit_index = 0;
    std::vector<int> chain_lockon_enemy_indexes; // ロックオンされたの敵のインデックス
    std::vector<LockOnSuggest> lockon_suggests;  // プレイヤーとロックオンされた敵の情報を持つソートするための情報
    std::vector<DirectX::XMFLOAT3> sort_points;  // ソートされたポイント
    std::vector<DirectX::XMFLOAT3> way_points;   // 中間点を算出したポイント
    std::vector<DirectX::XMFLOAT3> interpolated_way_points; // way_pointsを通るように分割したポイント
    std::map<std::unique_ptr<Reticle>, BaseEnemy*> reticles; // チェイン攻撃のreticles
    bool is_chain_attack = false; // ロックオン完了から攻撃終了までtrue
    bool is_chain_attack_aftertaste = false; // ロックオン完了から攻撃終了後カメラが追いついたあとちょっと待ってtrue
    float is_chain_attack_aftertaste_timer = 0;
    static constexpr float ROCKON_FRAME = 0.3f;
    float frame_time  = 0.0f;
    float frame_scope = 0.5f;
    float frame_alpha = 0.0f;
    bool chain_cancel = false;
    enum class ATTACK_TYPE
    {
        FIRST,
        SECOND,
        THIRD,
    };
    ATTACK_TYPE attack_type = ATTACK_TYPE::FIRST;
private:
    //------------------------------------------------------------------------------------------//
    //                        チュートリアルに関する関数,変数
    //------------------------------------------------------------------------------------------//
    //チュートリアルかどうか trueでチュートリアル
    bool is_tutorial{ false };
    //今のチュートリアルが終わった時にtrueになる
    bool is_next_tutorial{ false };
    //今のチュートリアルでどれだけ操作したかどうか
    float execution_timer{ 0 };
    //今のチュートリアルで何回その行動をしたか
    int tutorial_action_count{ 3 };
    //チュートリアルの関数ポインタを呼ぶ
    void ExecFuncTutorialUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
public:
    //チュートリアルのステート変更
    void ChangeTutorialState(int state);
    void SetIsTutorial(bool tutorial) { is_tutorial = tutorial; }
    void FalseNextTutorial() { is_next_tutorial = false; }
    void SetTutorialCount(int count) { tutorial_action_count = count; }
    bool GetNextTutorial() { return is_next_tutorial; }
    int GetTutorialCount() { return tutorial_action_count; }
private:
    //1が最初で大きくなっていくようにする
    enum class TutorialState
    {
        //移動
        MoveTutorial = 1,
        //回避(通常)
        AvoidanceTutorial,
        //ロックオン
        LockOnTutorial,
        //攻撃
        AttackTutorial,
        //回り込み回避
        BehindAvoidanceTutorial,
        //チェイン攻撃
        ChainAttackTutorial,
        //覚醒
        AwaikingTutorial,
        //自由時間
        FreePractice,
    };
    TutorialState tutorial_state{ TutorialState::MoveTutorial };
private:
    //チュートリアルの時のアニメーション更新処理
    typedef void(Player::* PlayerTutorialActivity)(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    //関数ポインタの変数
    PlayerTutorialActivity player_tutorial_activity = &Player::TutorialIdleUpdate;
    //各種更新処理
    void TutorialIdleUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialMoveUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAvoidanvceUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialBehindAvoidanceUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialChargeinitUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialChargeUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAttack1Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAttack2Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAttack3Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAwaikingUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialInvAwaikingUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialDamageUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAwaikingEventUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAwaikingEventIdleUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    //各遷移関数
    void TransitionTutoriaIdle(float blend_second = 0.3f);
    void TransitionTutorialMove(float blend_second = 0.3f);
    void TransitionTutorialAvoidance(float blend_second = 0.3f);
    void TransitionTutorialBehindAvoidance();
    void TransitionTutorialJustBehindAvoidance();
    void TransitionTutorialChargeInit();
    void TransitionTutorialCharge(float blend_second = 0.3f);
    void TransitionTutorialAttack1(float blend_second = 0.3f);
    void TransitionTutorialAttack2(float blend_second = 0.3f);
    void TransitionTutorialAttack3(float blend_second = 0.3f);
    void TransitionTutorialAwaiking();
    void TransitionTutorialInvAwaiking();
    //イベントシーンの覚醒
    void TransitionTutorialAwaikingEvent();
    void TransitionTutorialAwaikingEventIdle();
    int awaiking_event_state = 0;
    //チュートリアルの覚醒イベントが始まったらtrue(１回だけ)
    bool tutorial_awaiking{ false };
    bool awaiking_event{ false };
    bool awaiking_se{ false };
    //ダメージ受けたときに遷移
    void TransitionTutorialDamage();

    SePriset se_priset = SePriset::Se0;
    void TutorialAwaiking();//覚醒状態のON,OFF
public:
    bool GetTutorialEvent() { return awaiking_event; }
    int GetTutorialState() { return static_cast<int>(tutorial_state); }
    void SetTutorialDamageFunc();
};