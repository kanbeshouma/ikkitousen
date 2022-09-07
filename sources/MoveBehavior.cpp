#include "MoveBehavior.h"

DirectX::XMFLOAT3 MoveBehavior::SetMoveVec(const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right)
{
    //入力情報を取得
    float ax = game_pad->get_axis_LX();
    float ay = game_pad->get_axis_LY();

    //移動ベクトルはXZ平面に水平なベクトルになるようにする

    //カメラ右方向ベクトルをXZ単位ベクトルに変換

    float camerarightlengh = sqrtf((camera_right.x * camera_right.x) + (camera_right.z * camera_right.z));
    DirectX::XMFLOAT3 camera_right_normal{};
    if (camerarightlengh > 0.0f)
    {
        camera_right_normal = Math::Normalize(camera_right);
    }

    float cameraforwardlength = sqrtf((camera_forward.x * camera_forward.x) + (camera_forward.z * camera_forward.z));
    DirectX::XMFLOAT3 camera_forward_normal{};
    if (cameraforwardlength > 0.0f)
    {
        camera_forward_normal = Math::Normalize(camera_forward);
    }

    DirectX::XMFLOAT3 vec{};
    vec.x = (camera_forward_normal.x * ay) + (camera_right_normal.x * ax);
    vec.z = (camera_forward_normal.z * ay) + (camera_right_normal.z * ax);

    return vec;
}

void MoveBehavior::MovingProcess(float vx, float vz, float speed)
{
    //移動方向ベクトルを設定
    move_vec_x = vx;
    move_vec_z = vz;
    //最大速度設定
    max_move_speed = speed;
}

void MoveBehavior::Turn(float elapsed_time, DirectX::XMFLOAT3 move_velocity, float speed, DirectX::XMFLOAT3 position, DirectX::XMFLOAT4& orientation)
{
    using namespace DirectX;
    speed *= elapsed_time;
    DirectX::XMVECTOR orientation_vec{ DirectX::XMLoadFloat4(&orientation) };

    float length = sqrtf((move_velocity.x * move_velocity.x) + (move_velocity.z * move_velocity.z));
    if (length < 0.1f) return;

    DirectX::XMFLOAT4X4 m4x4 = {};
    DirectX::XMVECTOR forward, up;
    m4x4 = QuaternionConvertXMFLOAT4X4(orientation);
    up = { 0, 1, 0 };
    //up = { m4x4._21, m4x4._22, m4x4._23 };
    forward = { m4x4._31, m4x4._32, m4x4._33 };
    XMFLOAT3 front{};
    XMStoreFloat3(&front, forward);
    front.y = 0;
    XMFLOAT3 point1 = Math::calc_designated_point(position, Math::Normalize(front), Math::Length(Math::Normalize(front)));
    XMVECTOR point1_vec = XMLoadFloat3(&point1);
    XMVECTOR d = point1_vec - XMLoadFloat3(&position);

    move_velocity.y = 0;
    XMFLOAT3 point2 = Math::calc_designated_point(position, Math::Normalize(move_velocity), Math::Length(Math::Normalize(move_velocity)));
    XMVECTOR point2_vec = XMLoadFloat3(&point2);
    XMVECTOR d2 = point2_vec - XMLoadFloat3(&position);

    float angle;
    XMVECTOR dot{ XMVector3Dot(d,d2) };
    XMStoreFloat(&angle, dot);
    angle = acosf(angle);

    if (fabs(angle) > XMConvertToRadians(10.0f))
    {
        XMVECTOR q;
        float cross{ (move_velocity.x * front.z) - (move_velocity.z * front.x) };
        if (cross > 0)
        {
            q = XMQuaternionRotationAxis(up, angle);
        }
        else
        {
            q = XMQuaternionRotationAxis(up, -angle);
        }
        XMVECTOR Q = XMQuaternionMultiply(orientation_vec, q);
        orientation_vec = XMQuaternionSlerp(orientation_vec, Q, 10.0f * elapsed_time);
    }
    DirectX::XMStoreFloat4(&orientation, orientation_vec);


#if 0
    DirectX::XMFLOAT3 forw{};
    DirectX::XMStoreFloat3(&forw, f);
    forw.y = 0;
    DirectX::XMVECTOR forward{ DirectX::XMLoadFloat3(&forw) };

    DirectX::XMVECTOR axis_up = up;//Yの回転軸

    DirectX::XMFLOAT3 v{ velocity };
    v.y = 0;
    DirectX::XMVECTOR velocity_vec{ DirectX::XMLoadFloat3(&v) };
    DirectX::XMVECTOR velocity_vec_normal{ DirectX::XMVector3Normalize(velocity_vec) };

    //前方向(forward)と回転仕切った後の姿勢との内積を取る
    DirectX::XMVECTOR a;
    a = DirectX::XMVector3Dot(velocity_vec_normal, forward);
    float angle;
    DirectX::XMStoreFloat(&angle, a);
    angle = acos(angle);
    ImGui::Begin("test");
    float te{ DirectX::XMConvertToDegrees(angle) };
    ImGui::DragFloat("length", &length);
    ImGui::DragFloat("angle", &te);
    ImGui::End();

    if (fabs(angle) > DirectX::XMConvertToRadians(10.0f))
    {
        DirectX::XMFLOAT3 f{};
        DirectX::XMStoreFloat3(&f, forward);
        float cross{ (f.x * velocity.z) - (f.z * velocity.x) };

        if (cross < 0.0f)
        {
            DirectX::XMVECTOR q;
            q = DirectX::XMQuaternionRotationAxis(axis_up, angle);
            DirectX::XMVECTOR Q = DirectX::XMQuaternionMultiply(orientation_vec, q);
            orientation_vec = DirectX::XMQuaternionSlerp(orientation_vec, Q, speed);
        }
        else
        {
            DirectX::XMVECTOR q;
            q = DirectX::XMQuaternionRotationAxis(axis_up, -angle);
            DirectX::XMVECTOR Q = DirectX::XMQuaternionMultiply(orientation_vec, q);
            orientation_vec = DirectX::XMQuaternionSlerp(orientation_vec, Q, speed);
        }
    }
    DirectX::XMStoreFloat4(&orientation, orientation_vec);

#endif // 0

}
