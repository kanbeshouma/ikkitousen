#include "EnemyMove.h"
void EnemyMove::fUpdateVelocity(float elapsedTime_, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation)
{
    //経過フレーム
    float elapsed_frame = 60.0f * elapsedTime_;

    fCalcVelocity(elapsed_frame);

    fUpdateMove(elapsedTime_, position);
}

void EnemyMove::fCalcVelocity(float elasedFrame_)
{
    //速力を減速する
    const float length{ sqrtf((velocity.x * velocity.x) + (velocity.y * velocity.y) + (velocity.z * velocity.z)) };
    if (length > 0.0f)
    {
        //摩擦力
        const float friction{ this->friction * elasedFrame_ };
        //摩擦による横方向の減速処理
        if (length > friction)
        {
            (velocity.x < 0.0f) ? velocity.x += friction : velocity.x -= friction;
            (velocity.y < 0.0f) ? velocity.y += friction : velocity.y -= friction;
            (velocity.z < 0.0f) ? velocity.z += friction : velocity.z -= friction;
        }
        //横方向の速力が摩擦力以下になったので速力を無効化 GetMoveVec()
        else
        {
            velocity.x = 0;
            velocity.y = 0;
            velocity.z = 0;
        }
    }
    //速力を加速する
    if (length <= max_move_speed)
    {
        //移動ベクトルが0でないなら加速する
        const float moveveclength{ sqrtf((move_vec_x * move_vec_x) + (move_vec_y * move_vec_y) + (move_vec_z * move_vec_z)) };
        if (moveveclength > 0.0f)
        {
            //加速力
            const float acceleration{ this->acceleration * elasedFrame_ };
            //移動ベクトルによる加速処理
            velocity.x += move_vec_x * acceleration;
            velocity.y += move_vec_y * acceleration;
            velocity.z += move_vec_z * acceleration;
            float length{ sqrtf((velocity.x * velocity.x) + (velocity.y * velocity.y) + (velocity.z * velocity.z)) };
            if (length > max_move_speed)
            {
                const float vx{ velocity.x / length };
                const float vy{ velocity.y / length };
                const float vz{ velocity.z / length };

                velocity.x = vx * max_move_speed;
                velocity.y = vy * max_move_speed;
                velocity.z = vz * max_move_speed;
            }
        }
    }
    move_vec_x = 0.0f;
    move_vec_y = 0.0f;
    move_vec_z = 0.0f;
}

void EnemyMove::fUpdateMove(float elapsedTime_, DirectX::XMFLOAT3& position)
{
    using namespace DirectX;
    //速力計算
    const float velocity_length_xyz = sqrtf((velocity.x * velocity.x) + (velocity.y * velocity.y) + (velocity.z * velocity.z));
    if (velocity_length_xyz > 0.0f)
    {
        position.x += velocity.x * elapsedTime_;
        position.y += velocity.y * elapsedTime_;
        position.z += velocity.z * elapsedTime_;
    }
}

void EnemyMove::MovingProcess(DirectX::XMFLOAT3 v, float speed)
{
    //移動方向ベクトルを設定
    move_vec_x = v.x;
    move_vec_y = v.y;
    move_vec_z = v.z;
    //最大速度設定
    max_move_speed = speed;
}

