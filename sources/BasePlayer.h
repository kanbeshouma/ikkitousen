#pragma once
#include<memory>
#include<DirectXMath.h>
#include"graphics_pipeline.h"
#include"skinned_mesh.h"
#include "practical_entities.h"
#include"SkyDome.h"
#include "BaseEnemy.h"
#include"Common.h"
#include"NetWorkInformationStucture.h"

class BasePlayer :public PracticalEntities
{
public:
    BasePlayer() {}
    virtual ~BasePlayer() {}
public:
    virtual  void Initialize() = 0;
    virtual  void Update(float elapsed_time, GraphicsPipeline& graphics,SkyDome* sky_dome, std::vector<BaseEnemy*> enemies) = 0;
    virtual void Render(GraphicsPipeline& graphics, float elapsed_time) = 0;
    //スタンしている敵がいるかどうか
    virtual bool EnemiesIsStun(std::vector<BaseEnemy*> enemies) = 0;
    virtual void ConfigRender(GraphicsPipeline& graphics, float elapsed_time) = 0;
    virtual void ChangePlayerJustificationLength() = 0;

    //-----リトライ時再初期化-----//
    virtual void RestartInitialize(int health) = 0;;
protected:
    //プレイヤーの最大体力
    static constexpr int MAX_HEALTH = 100;

    //プレイヤーの体力
    int player_health = MAX_HEALTH;


    DirectX::XMFLOAT3 position{};
    DirectX::XMFLOAT4 orientation{ 0,0,0,1.0f };
    DirectX::XMFLOAT3 scale{ 1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 color{ 1.0f,0.325f,0.325f,1.0f };
    std::vector<DirectX::XMFLOAT4X4> transform{};

    float threshold;
    float threshold_mesh{ 1.0f };
    float threshold_camera_mesh{ 1.0f };
    float glow_time;
    DirectX::XMFLOAT4 emissive_color{ 1.0f,1.0f,1.0f,0.9f };
    //カプセル敵との当たり判定
    struct CapsuleParam
    {
        DirectX::XMFLOAT3 start{};
        DirectX::XMFLOAT3 end{};
        float rasius{ 1.2f };
    };
public:
    enum class PlayerColor
    {
        Red,
        Yellow,
        Blue,
        SkyBlue,
        Green,
        Purple,
        Pink,
        White,
        Black,
    };
    //-----色の設定-----//
    //==============
    //第1引数 : 色の種類のenum class
    void SetColor(PlayerColor kind);
protected:
    //-----キャラクターそれぞれの番号-----//
    int object_id{ -1 };
    SkinnedMesh::anim_Parameters anim_parm{};
    //-----ロックオンしている敵の番号-----//
    int lock_on_enemy_id{ -1 };
protected:
    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 projection{};
public:
    ////-----オブジェクトIDのゲッター-----//
    int GetObjectId() { return object_id; }

    //------クライアントのロックオンしている敵の番号-----//
    int GetLockPnEnemyId() { return lock_on_enemy_id; }

    virtual  void SetEndDashEffect(bool a) = 0;
  virtual  void SetCameraTarget(DirectX::XMFLOAT3 p) = 0;
  virtual  void SetBossCamera(bool boss_c) = 0;
  virtual  void SetPosition(DirectX::XMFLOAT3 pos) = 0;
  virtual  DirectX::XMFLOAT3 GetForward() = 0;
  virtual  DirectX::XMFLOAT3 GetRight() = 0;
  virtual  DirectX::XMFLOAT3 GetUp() = 0;
  virtual  DirectX::XMFLOAT3 GetPosition()  = 0;
  virtual  DirectX::XMFLOAT3 GetVelocity()  = 0;
  virtual  HitResult& GetPlayerHitResult() = 0;
  virtual  bool GetCameraReset() = 0;
  virtual  bool GetCameraLockOn() = 0;
  virtual  bool GetEnemyLockOn() = 0;
  virtual  bool GetAvoidance() = 0;
  virtual  bool GetBehindAvoidance() = 0;
  virtual  bool GetIsPlayerAttack() = 0;
  virtual  bool GetIsCharge() = 0;
  virtual  bool GetIsSpecialSurge() = 0;
  virtual  bool GetStartDashEffect() = 0;
  virtual  bool GetEndDashEffect() = 0;
  virtual  bool GetIsAwakening() = 0;
  virtual  bool GetIsAlive() = 0;
  virtual  bool GetIsJustAvoidance() = 0;
  virtual  bool GetBehaindCharge() = 0;
  virtual  CapsuleParam GetBodyCapsuleParam() = 0;
  virtual  CapsuleParam GetJustAvoidanceCapsuleParam() = 0;
  virtual  CapsuleParam GetSwordCapsuleParam(int i) = 0;
  virtual float GetStunRadius() = 0;
  virtual std::vector<DirectX::XMFLOAT3> GetBehindPoint() = 0;
  virtual void SetRaycast(bool r) = 0;
  virtual int GetPlayerPower() = 0;
  virtual [[nodiscard("Not used")]] const AddDamageFunc GetDamagedFunc() = 0;

  //-----カメラのview行列とプロジェクション行列を設定-----//
  void SetCameraView(DirectX::XMFLOAT4X4 v) { view = v; };
  void SetCameraProjection(DirectX::XMFLOAT4X4 p) { projection = p; };

  //-----ClientPlayerとPlayerの距離を設定する-----//
  virtual void SetPlayerToClientLength(float l) {}

    virtual BaseEnemy* GetPlayerTargetEnemy() const = 0;
   //一番近い敵を持って来てその位置をセットする
    virtual void SetTarget(BaseEnemy* target_enemy) = 0;
    virtual DirectX::XMFLOAT3 GetTarget() = 0;
    virtual  void AddCombo(int count, bool& block) = 0;
    //覚醒状態の時は２つ当たり判定があるから引数が２つ
    virtual void AwakingAddCombo(int hit_count1, int hit_count2, bool& block) = 0;
    //--------------------<敵からダメージを受ける>--------------------//
    virtual  void DamagedCheck(int damage, float InvincibleTime) = 0;
    virtual void TutorialDamagedCheck(int damage, float InvincibleTime) = 0;
    virtual  void PlayerKnocKback(float elapsed_time) = 0;
    //プレイヤーのジャスト回避用の当たり判定に当たったら
    virtual void PlayerJustAvoidance(bool hit) = 0;

    virtual int GetHealth() = 0;
    virtual void SetHealth(int arg) = 0;
public:
    virtual void FalseCameraReset() = 0;
    virtual void FalseCameraLockOn() = 0;
    virtual  void FalseAvoidance() = 0;
    virtual void SetCameraDirection(const DirectX::XMFLOAT3& c_forward, const DirectX::XMFLOAT3& c_right) = 0;
    virtual void SetCameraPosition(DirectX::XMFLOAT3 p) = 0;

    //待機に遷移
    virtual void TransitionIdle(float blend_second = 0.3f) = 0;
    //スタートモーション
    virtual void TransitionStartMothin() = 0;
    //ステージ移動に遷移
    virtual void TransitionStageMove() = 0;
    //ステージ遷移終了
    virtual void TransitionStageMoveEnd() = 0;

    //-----受信データを設定する-----//
    virtual void SetReceiveData(PlayerMoveData data) = 0;
    virtual void SetReceivePositionData(PlayerPositionData data) = 0;
    virtual void SetPlayerActionData(PlayerActionData data) = 0;
public:
    virtual DirectX::XMFLOAT3 GetEnentCameraEye() = 0;
    virtual DirectX::XMFLOAT3 GetEnentCameraJoint() = 0;
    virtual bool GetEndClearMotion() = 0;
    virtual bool GetStartClearMotion() = 0;
    virtual void PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies) = 0;
public:
    virtual bool during_search_time() = 0;
    virtual bool during_chain_attack_end() = 0;
    virtual bool during_chain_attack() = 0;
    virtual void lockon_post_effect(float elapsed_time, std::function<void(float, float)> effect_func, std::function<void()> effect_clear_func) = 0;

protected:
    //-----マルチプレイ時のチェイン攻撃の許可-----//
    bool permit_chain_attack{ false };
public:
    bool GetPermitChainAttack() { return permit_chain_attack; }
    void SetPermitChainAttack(bool arg) { permit_chain_attack = arg; }
protected:
    //-----マルチプレイ時のチェイン攻撃終了して敵のホスト権を返還する-----//
    bool return_enemy_control{ false };
public:
    bool GetRetrunEnemyControl() { return return_enemy_control; }
    void SetReturnEnemyControl(bool arg) { return_enemy_control = arg; }
protected:
    //-----チェイン攻撃の敵の番号を受信した時に入れる箱-----//
    std::vector<char> receive_chain_lock_on_enemy_id;
public:
    void SetChainLockOnId(std::vector<char> d) { receive_chain_lock_on_enemy_id = d; }

    enum class Behavior
    {
        //通常状態
        Normal,
        //スタンした敵に攻撃する状態
        Chain
    };
protected:
     Behavior behavior_state{ Behavior::Normal };
public:
    Behavior GetBehaviorState() { return behavior_state; }
protected:
    //-----チェイン攻撃をしていいかどうか(マルチプレイの時にしか使わない)------//
    bool do_chain{ true };
public:
    void SetDoChain(bool arg) { do_chain = arg; }
protected:
    std::shared_ptr<SkinnedMesh> model{ nullptr };
};