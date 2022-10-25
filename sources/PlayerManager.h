#pragma once
#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include<memory>
#include<vector>

#include"BasePlayer.h"
#include"EnemyManager.h"
#include"graphics_pipeline.h"
#include"BulletManager.h"
#include"post_effect.h"
#include"NetWorkInformationStucture.h"

//===================================//
////----------プレイヤーを管理するクラス----------//
//===================================//
class PlayerManager
{
public:
    PlayerManager();
    ~PlayerManager();
public:
    ////----------更新関数----------//
    void Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);

    ////----------更新関数----------//
    void PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);

    ////-----コンフィグのレンダー//
    void ConfigRender(GraphicsPipeline& graphics, float elapsed_time);

    ////-----描画関数-----//
    void Render(GraphicsPipeline& graphics, float elapsed_time);
public:
    ////-----プレイヤー登録関数-----//
    void RegisterPlayer(BasePlayer* player);

    ////-----プレイヤーを削除-----//
    //=====================
    //第1引数 : 削除するプレイヤーのID
    void DeletePlayer(int id);

    ////-----プレイヤーの行動範囲を設定-----//
    void ChangePlayerJustificationLength();

    ////-----ステージ移動開始-----//
    void TransitionStageMove();

    ////-----ステージ移動終了-----//
    void TransitionStageMoveEnd();

    ////-----ジャスト回避したかどうか-----//
    bool GetIsJustAvoidance();

    ////-----プレイヤーの位置取得-----//
    //=========================
    //戻り値 : 登録されているプレイヤーのオブジェクト番号と位置
    //std::tuple<object_id,position>
    std::vector<std::tuple<int,DirectX::XMFLOAT3>> GetPosition();

    ////-----チェイン攻撃の作成時間中かどうか-----//
    bool DuringSearchTime();

    ////-----プレイヤーが生きているかどうか-----//
    bool GetIsAlive();

    ////-----敵とプレイヤーの攻撃の当たり判定-----//
    void PlayerAttackVsEnemy(EnemyManager* enemy_manager,GraphicsPipeline& graphics,float elapsed_time);

    ////-----ジャスト回避が可能かどうかの当たり判定-----//
    void PlayerCounterVsEnemyAttack(EnemyManager* enemy_manager);

    ////-----敵の攻撃との当たり判定-----//
    void EnemyAttackVsPlayer(EnemyManager* enemy_manager);

    ////-----クライアントがロックオンしている敵を検索する-----//
    void SearchClientPlayerLockOnEnemy(EnemyManager* enemy_manager);

    ////-----敵の弾とプレイヤーの当たり判定-----//
    void BulletVsPlayer(BulletManager& bullet_manager);

    ////-----ジャスト回避した時の範囲スタンの当たり判定-----//
    void PlayerStunVsEnemy(EnemyManager* enemy_manager);

    ////-----チェイン攻撃中に行動を停止させる-----//
    void SetPlayerChainTime(EnemyManager* enemy_manager);

    ////-----ボスのカメラに切り替える-----//
    void SetBossCamera(bool flag);

    ////-----カメラの方向を設定する-----//
    void SetCameraDirection(DirectX::XMFLOAT3 f, DirectX::XMFLOAT3 r);

    ////-----カメラのview行列とプロジェクション行列を設定する-----//
    void SetCameraView(const DirectX::XMFLOAT4X4 view);
    void SetCameraProjection(const DirectX::XMFLOAT4X4 projection);

    ////-----ロックオンした時のポストエフェクトを設定する-----//
    void LockOnPostEffect(float elapsed_time,PostEffect* post_efc);

    ////-----プレイヤーにカメラの位置を設定する-----//
    void SetCameraPosition(DirectX::XMFLOAT3 eye);

    ////-----一番近い敵を設定する-----//
    void SetTarget(BaseEnemy* enemy);

    ////-----カメラのターゲットを設定する-----//
    void SetCameraTarget(DirectX::XMFLOAT3 target);

    ////-----ダッシュエフェクトをかける-----//
    void DashPostEffect(GraphicsPipeline& graphics,PostEffect* post_efc);

    ////-----この端末のプレイヤーのターゲットにしている敵を取得-----//
    BaseEnemy* GetTargetEnemy();

    ////-----この端末のプレイヤーがロックオンしたかどうか-----//
    bool GetEnemyLockOn();

    ////-----この端末のプレイヤーの回り込み回避ができるかどうかのフラグを取得-----//
    bool GetBehaindCharge();
public:
    ////-----プレイヤーの位置を設定する-----//
    void SetPlayerPosition(DirectX::XMFLOAT3 pos);

    ////-----この端末のプレイヤーの位置を取得-----//
    DirectX::XMFLOAT3 GetMyTerminalPosition();

    ////-----この端末のプレイヤーの前方向を取得する-----//
    DirectX::XMFLOAT3 GetMyTerminalForward();

    ////-----この端末のプレイヤーのイベントカメラのジョイントを取得-----//
    DirectX::XMFLOAT3 GetMyTerminalJoint();

    ////-----この端末のプレイヤーのイベントカメラの位置を取得する-----//
    DirectX::XMFLOAT3 GetMyTerminalEye();

    ////-----この端末のプレイヤーのイベントモーションが終わったかどうかを取得する-----//
    bool GetMyTerminalEndClearMotion();

    ////-----データを設定する-----//
    //=====================
    //第1引数 : データ
    void SetPlayerMoveData(PlayerMoveData data);

    ////-----データを設定する-----//
    //=====================
    //第1引数 : データ
    void SetPlayerPositionData(PlayerPositionData data);

    ////-----データを設定する-----//
    //=====================
    //第1引数 : データ
    void SetPlayerActionData(PlayerActionData data);

    ////-----データを設定する-----//
    //=====================
    //第1引数 : データ
    void SetPlayerPlayerAttackResultData(PlayerAttackResultData data);

private:
    ////-----プレイヤーを入れる変数-----//
    std::vector<std::shared_ptr<BasePlayer>> players;

    ////-----削除するプレイヤーを入れる変数-----//
    std::vector<std::shared_ptr<BasePlayer>> remove_players;
private:
    //-----この端末で操作することができるプレイヤーのオブジェクト番号-----//
    int private_object_id{ -1 };
public:
    void SetPrivateObjectId(int id) { private_object_id = id; }
     int GetPrivatePlayerId() { return private_object_id; }
};