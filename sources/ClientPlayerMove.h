#pragma once
#include"MoveBehavior.h"

class ClientPlayerMove : public MoveBehavior
{
public:
    ClientPlayerMove();
    ~ClientPlayerMove();
protected:
    float max_length = 300.0f;
public:
    void UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);
    //プレイヤーの位置矯正
    void PlayerJustification(float elapsed_time, DirectX::XMFLOAT3& pos);
    //-----入力方向の設定-----//
    void SetMoveVec(DirectX::XMFLOAT3 vec) { movevec = vec; }
    //-----入力方向のゲッター-----//
    DirectX::XMFLOAT3 GetMoveVec() { return movevec; }

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
    //プレイヤーの前方向
    DirectX::XMFLOAT3 player_forward{};
    //方向取得
    void SetDirections(DirectX::XMFLOAT4 o);
};