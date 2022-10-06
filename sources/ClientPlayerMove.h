#pragma once
#include"MoveBehavior.h"

class ClientPlayerMove : public MoveBehavior
{
public:
    ClientPlayerMove();
    ~ClientPlayerMove();
protected:
    float max_length = 300.0f;

    //-----受信データの位置の許容値-----//
    float allowable_limit_position{ 2.0f };
public:
    void UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, SkyDome* sky_dome);

    //攻撃中の更新処理(回避も)
    void UpdateAttackVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);

    //回り込み中の更新処理
    void UpdateBehindAvoidanceVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);

    //プレイヤーの位置矯正
    void PlayerJustification(float elapsed_time, DirectX::XMFLOAT3& pos);

    //急速回転突進の時
    void ChargeTurn(float elapsed_time, DirectX::XMFLOAT3 move_velocity, float speed, DirectX::XMFLOAT3 position, DirectX::XMFLOAT4& orientation);

    //-----入力方向のゲッター-----//
    DirectX::XMFLOAT3 GetInputMoveVec() { return input_move; }

    //-----動く方向の力-----//
    DirectX::XMFLOAT3 GetMoveVecter() { return movevec; }

    //-----動く力のセッター-----//
    void SetMoveVecter(DirectX::XMFLOAT3 vec) { movevec = vec; }

private:
    //垂直速力更新処理
    void UpdateVerticalVelocity(float elapsedFrame);
    //垂直移動更新処理
    void UpdateVerticalMove(float elapsedTime, DirectX::XMFLOAT3& position, SkyDome* sky_dome);
    //水平速力更新処理
    void UpdateHrizontalVelocity(float elasedFrame);
    //水平移動更新処理
    void UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, SkyDome* sky_dome);
    //ターゲットに向かって回転
    void RotateToTarget(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation);
protected:
    //敵をロックオンしたかどうか
    bool is_lock_on{ false };
    //敵がカメラ内にいない状態でロックオンボタンを押していたらtrue
    bool is_push_lock_on_button{ false };
    //ターゲット
    DirectX::XMFLOAT3 target{};
private:
    //-----入力方向-----//
    DirectX::XMFLOAT3 movevec{};
    //-----データ送信用の変数-----//
    DirectX::XMFLOAT3 input_move{};
    //プレイヤーの前方向
    DirectX::XMFLOAT3 player_forward{};
    //-----プレイヤーの補完位置-----//
    DirectX::XMFLOAT3 lerp_position{};
    //-----補完していいかどうか-----//
    bool start_lerp{ false };
    //方向取得
    void SetDirections(DirectX::XMFLOAT4 o);
    //-----位置保管-----//
    void LerpPosition(float elapsed_time , DirectX::XMFLOAT3& position);

public:
    void SetLerpPosition(DirectX::XMFLOAT3 pos);
};