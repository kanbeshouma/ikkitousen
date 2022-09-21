#include "ClientPlayerMove.h"

ClientPlayerMove::ClientPlayerMove()
    :MoveBehavior()
{
}

ClientPlayerMove::~ClientPlayerMove()
{
}

void ClientPlayerMove::UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, SkyDome* sky_dome)
{
    SetDirections(orientation);
    MovingProcess(movevec.x, movevec.z, move_speed);
    if (is_lock_on)
    {
        RotateToTarget(elapsed_time, position, orientation);
    }
    else
    {
        //旋回処理
        Turn(elapsed_time, movevec, turn_speed, position, orientation);
    }
    //経過フレーム
    float elapsed_frame = 60.0f * elapsed_time;
    UpdateVerticalVelocity(elapsed_frame);
    UpdateVerticalMove(elapsed_time, position, sky_dome);
    UpdateHrizontalVelocity(elapsed_frame);
    UpdateHorizontalMove(elapsed_time, position, sky_dome);
}

void ClientPlayerMove::PlayerJustification(float elapsed_time, DirectX::XMFLOAT3& pos)
{
    using namespace DirectX;
    //原点
    XMFLOAT3 starting_point{ 0,0,0 };
    XMVECTOR starting_point_vec{ XMLoadFloat3(&starting_point) };
    //プレイヤー
    XMVECTOR player_pos_vec{ XMLoadFloat3(&pos) };
    XMVECTOR dir{ starting_point_vec - player_pos_vec };
    //長さを求める
    XMVECTOR length_vec{ XMVector3Length(dir) };
    float length{};
    XMStoreFloat(&length, length_vec);
    //最大距離よりも大きくなったら
    if (length > max_length)
    {
        //長さの差を求める
        float difference{ length - max_length };
        //原点の方向のベクトル
        XMVECTOR dir_normal_vec{ XMVector3Normalize(dir) };
        //原点方向に差の位置を出す
        XMVECTOR remedy{ XMVectorScale(dir_normal_vec,difference) };
        XMFLOAT3 remedy_p{};
        XMStoreFloat3(&remedy_p, remedy);
        pos.x += remedy_p.x;
        //y座標は変わったらいけないから
        pos.y = 0;
        pos.z += remedy_p.z;

    }
}

void ClientPlayerMove::UpdateVerticalVelocity(float elapsed_frame)
{
    float length{ sqrtf(velocity.y * velocity.y) };
    if (length > 0.0f)
    {
        //摩擦力
        float friction{ this->friction * elapsed_frame };
        //摩擦による横方向の減速処理
        if (length > friction)
        {
            (velocity.y < 0.0f) ? velocity.y += friction : velocity.y -= friction;
        }
        else
        {
            velocity.y = 0;
        }
    }
    if (length <= max_move_speed)
    {
        //移動ベクトルが0でないなら加速する
        float moveveclength{ sqrtf((move_vec_y * move_vec_y)) };
        if (moveveclength > 0.0f)
        {
            //加速力
            float acceleration{ this->acceleration * elapsed_frame };
            //移動ベクトルによる加速処理
            velocity.y += move_vec_y * acceleration;
            float length{ sqrtf((velocity.y * velocity.y)) };
            if (length > max_move_speed)
            {
                float vy{ velocity.y / length };
                velocity.y = vy * max_move_speed;
            }
        }
    }
    move_vec_y = 0.0f;
}

void ClientPlayerMove::UpdateVerticalMove(float elapsed_time, DirectX::XMFLOAT3& position, SkyDome* sky_dome)
{
    //垂直方向の移動量
    float my = velocity.y * elapsed_time;

    if (my != 0.0f)
    {
        //レイの開始位置は足元より少し上
        DirectX::XMFLOAT3 start = { position.x, position.y, position.z };
        //レイの終点位置は移動後の位置
        DirectX::XMFLOAT3 end = { position.x, position.y - my + step_offset_y, position.z };

        if (sky_dome->RayCast(start, end, hit))
        {
            position.y = hit.position.y;
            velocity.y = 0.0f;
        }
        else
        {
            position.y += my;
        }
    }
}

void ClientPlayerMove::UpdateHrizontalVelocity(float elapsed_frame)
{
    //XZ平面の速力を減速する
    float length{ sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) };
    if (length > 0.0f)
    {

        //摩擦力
        float friction{ this->friction * elapsed_frame };
        //摩擦による横方向の減速処理
        if (length > friction)
        {
            (velocity.x < 0.0f) ? velocity.x += friction : velocity.x -= friction;
            (velocity.z < 0.0f) ? velocity.z += friction : velocity.z -= friction;
        }
        //横方向の速力が摩擦力以下になったので速力を無効化 GetMoveVec()
        else
        {
            velocity.x = 0;
            velocity.z = 0;
        }
    }
    //XZ平面の速力を加速する
    if (length <= max_move_speed)
    {
        //移動ベクトルが0でないなら加速する
        float moveveclength{ sqrtf((move_vec_x * move_vec_x) + (move_vec_z * move_vec_z)) };
        if (moveveclength > 0.0f)
        {
            //加速力
            float acceleration{ this->acceleration * elapsed_frame };
            //移動ベクトルによる加速処理
            velocity.x += move_vec_x * acceleration;
            velocity.z += move_vec_z * acceleration;

            float length{ sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z)) };
            if (length > max_move_speed)
            {
                float vx{ velocity.x / length };
                float vz{ velocity.z / length };

                velocity.x = vx * max_move_speed;
                velocity.z = vz * max_move_speed;
            }
        }
    }


    move_vec_x = 0.0f;
    move_vec_z = 0.0f;
}

void ClientPlayerMove::UpdateHorizontalMove(float elapsed_time, DirectX::XMFLOAT3& position, SkyDome* sky_dome)
{
    using namespace DirectX;
    // 水平速力計算
    float velocity_length_xz = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
    //水平移動値
    float mx{ velocity.x * elapsed_time };
    float mz{ velocity.z * elapsed_time };
    if (velocity_length_xz > 0.0f)
    {
        // レイの開始位置と終点位置
        DirectX::XMFLOAT3 start = { position.x, position.y, position.z };
        DirectX::XMFLOAT3 end = { position.x + mx, start.y, position.z + mz };

        if (sky_dome->RayCast(start, end, hit))
        {
            using namespace DirectX;
            // 壁までのベクトル
            auto start_vec = XMLoadFloat3(&start);
            auto end_vec = XMLoadFloat3(&end);
            auto vec = end_vec - start_vec;
            // 壁の法線
            auto normal_vec = XMLoadFloat3(&hit.normal);
            // 入射ベクトルを法線に射影
            auto dot_vec = XMVector3Dot(vec, normal_vec);
            // 補正位置の計算
            auto corrected_position_vec = end_vec - (normal_vec * dot_vec);
            XMVECTOR interpolationvec{ corrected_position_vec - start_vec };//壁ずりベクトルの向き

            XMFLOAT3 corrected_position{};
            XMStoreFloat3(&corrected_position, corrected_position_vec);
            // レイの開始位置と終点位置
            DirectX::XMFLOAT3 start2 = { position.x, position.y, position.z };
            DirectX::XMFLOAT3 end2 = { corrected_position };
            if (sky_dome->RayCast(start2, end2, hit))
            {
                position.x = hit.position.x;
                position.z = hit.position.z;
            }
            else
            {
                XMFLOAT3 interpolation{};
                XMStoreFloat3(&interpolation, interpolationvec);
                position.x += interpolation.x;
                position.z += interpolation.z;

            }
        }
        else
        {
            position.x += mx;
            position.z += mz;
        }
    }
}

void ClientPlayerMove::RotateToTarget(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation)
{
    using namespace DirectX;
    //ターゲットに向かって回転
    XMVECTOR orientation_vec = DirectX::XMLoadFloat4(&orientation);
    DirectX::XMVECTOR forward, right, up;
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientation_vec);
    DirectX::XMFLOAT4X4 m4x4 = {};
    DirectX::XMStoreFloat4x4(&m4x4, m);
    right = { m4x4._11, m4x4._12, m4x4._13 };
    //up = { m4x4._21, m4x4._22, m4x4._23 };
    up = { 0, 1, 0 };
    forward = { m4x4._31, m4x4._32, m4x4._33 };

    XMVECTOR pos_vec = XMLoadFloat3(&position);//自分の位置
    DirectX::XMFLOAT3 front{};
    DirectX::XMStoreFloat3(&front, forward);
    DirectX::XMFLOAT3 t{ target.x, target.y, target.z };
    XMVECTOR target_vec = XMLoadFloat3(&t);
    XMVECTOR d = XMVector3Normalize(target_vec - pos_vec);
    float d_length = Math::calc_vector_AtoB_length(position, t);
    {
        DirectX::XMFLOAT3 point = Math::calc_designated_point(position, front, d_length);
        //point.y = target.y;
        DirectX::XMVECTOR point_vec = DirectX::XMLoadFloat3(&point);


        XMVECTOR d2 = XMVector3Normalize(point_vec - pos_vec);

        float an;
        XMVECTOR a = XMVector3Dot(d2, d);
        XMStoreFloat(&an, a);
        an = acosf(an);
        float de = DirectX::XMConvertToDegrees(an);

        if (fabs(an) > DirectX::XMConvertToRadians(0.01f))
        {
            XMVECTOR q;
            DirectX::XMFLOAT3 a{};
            DirectX::XMStoreFloat3(&a, d2);
            DirectX::XMFLOAT3 b{};
            DirectX::XMStoreFloat3(&b, d);
            float cross{ (b.x * a.z) - (b.z * a.x) };

            if (cross > 0)
            {
                q = XMQuaternionRotationAxis(up, an);//正の方向に動くクオータニオン
            }
            else
            {
                q = XMQuaternionRotationAxis(up, -an);//正の方向に動くクオータニオン
            }
            XMVECTOR Q = XMQuaternionMultiply(orientation_vec, q);
            orientation_vec = XMQuaternionSlerp(orientation_vec, Q, 10.0f * elapsed_time);
        }
    }
    DirectX::XMStoreFloat4(&orientation, orientation_vec);
}

void ClientPlayerMove::SetDirections(DirectX::XMFLOAT4 o)
{
    using namespace DirectX;
    //ターゲットに向かって回転
    XMVECTOR orientation_vec = DirectX::XMLoadFloat4(&o);
    DirectX::XMVECTOR forward;
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientation_vec);
    DirectX::XMFLOAT4X4 m4x4 = {};
    DirectX::XMStoreFloat4x4(&m4x4, m);
    forward = { m4x4._31, m4x4._32, m4x4._33 };

    XMStoreFloat3(&player_forward, forward);
}
