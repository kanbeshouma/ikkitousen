#include "EnemyMove.h"
void EnemyMove::fUpdateVelocity(float elapsedTime_, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation)
{
    //�o�߃t���[��
    float elapsed_frame = 60.0f * elapsedTime_;

    fCalcVelocity(elapsed_frame);

    fUpdateMove(elapsedTime_, position);
}

void EnemyMove::fCalcVelocity(float elasedFrame_)
{
    //���͂���������
    const float length{ sqrtf((velocity.x * velocity.x) + (velocity.y * velocity.y) + (velocity.z * velocity.z)) };
    if (length > 0.0f)
    {
        //���C��
        const float friction{ this->friction * elasedFrame_ };
        //���C�ɂ�鉡�����̌�������
        if (length > friction)
        {
            (velocity.x < 0.0f) ? velocity.x += friction : velocity.x -= friction;
            (velocity.y < 0.0f) ? velocity.y += friction : velocity.y -= friction;
            (velocity.z < 0.0f) ? velocity.z += friction : velocity.z -= friction;
        }
        //�������̑��͂����C�͈ȉ��ɂȂ����̂ő��͂𖳌��� GetMoveVec()
        else
        {
            velocity.x = 0;
            velocity.y = 0;
            velocity.z = 0;
        }
    }
    //���͂���������
    if (length <= max_move_speed)
    {
        //�ړ��x�N�g����0�łȂ��Ȃ��������
        const float moveveclength{ sqrtf((move_vec_x * move_vec_x) + (move_vec_y * move_vec_y) + (move_vec_z * move_vec_z)) };
        if (moveveclength > 0.0f)
        {
            //������
            const float acceleration{ this->acceleration * elasedFrame_ };
            //�ړ��x�N�g���ɂ���������
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
    //���͌v�Z
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
    //�ړ������x�N�g����ݒ�
    move_vec_x = v.x;
    move_vec_y = v.y;
    move_vec_z = v.z;
    //�ő呬�x�ݒ�
    max_move_speed = speed;
}

