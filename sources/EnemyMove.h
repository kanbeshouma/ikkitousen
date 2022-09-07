#pragma once
#include"MoveBehavior.h"
class EnemyMove : public MoveBehavior
{
public:
    EnemyMove() :MoveBehavior(){}
    ~EnemyMove(){}
protected:
    void fUpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation);
    //‘¬—ÍŒvZˆ—
    void fCalcVelocity(float elasedFrame);
    //ˆÚ“®XVˆ—
    void fUpdateMove(float elapsedTime_, DirectX::XMFLOAT3& position);

public:
    void MovingProcess(DirectX::XMFLOAT3 v, float speed);
};