#pragma once

#include <DirectXMath.h>
#include "practical_entities.h"
#include "constants.h"

class BaseCamera : public PracticalEntities
{
public:
    BaseCamera(){}
    virtual ~BaseCamera(){};
public:

    //forward
    DirectX::XMFLOAT3 GetForward() { return forward; }
    //right
    DirectX::XMFLOAT3 GetRight() { return right; }

    void UpdateViewProjection();

    void AttitudeControl(float elapsedTime);

    void DebugGUI();

    void calc_view_projection(GraphicsPipeline& graphics);

    void debug_gui();
    //--------<getter/setter>--------//
    // range
    void set_range(float r) { range = r; }
    // target
    void set_target(const DirectX::XMFLOAT3& t) { target = t; }
    const DirectX::XMFLOAT3& get_target() { return target; }
    // eye
    void set_eye(const DirectX::XMFLOAT3& e) { eye = e; }
    const DirectX::XMFLOAT3& get_eye() { return eye; }
    // angle
    void set_angle(const DirectX::XMFLOAT3& a) { angle = a; }
    // light_direction
    const DirectX::XMFLOAT4& get_light_direction() { return scene_constants->data.light_direction; }
    // view
    static const DirectX::XMFLOAT4X4& get_keep_view() { return keep_view; }
    // projection
    static const DirectX::XMFLOAT4X4& get_keep_projection() { return keep_projection; }
    // projection
    static const float get_keep_range() { return keep_range; }

protected:
    //--------< 定数/構造体 >--------//
    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 view_projection;              //ビュー・プロジェクション変換行列
        DirectX::XMFLOAT4 light_direction{ 0, -1, 0, 1 }; //ライトの向き
        DirectX::XMFLOAT4 light_color{ 1,1,1,1 };         //ライトの色
        DirectX::XMFLOAT4 camera_position;
        DirectX::XMFLOAT4X4 shake_matrix;
        DirectX::XMFLOAT4X4 inverse_view_projection;
    };
    std::unique_ptr<Constants<SceneConstants>> scene_constants;
    //--------< 変数 >--------//
    float range{ 0.5f };
    DirectX::XMFLOAT3 eye{};
    DirectX::XMFLOAT2 near_far = { 0.1f, 2000.0f }; // x:near y:far
    DirectX::XMFLOAT3 target{};
    DirectX::XMFLOAT3 angle{ 0.4f, 0, 0 };
    DirectX::XMFLOAT4 orientation;
    DirectX::XMFLOAT3 forward{};
    DirectX::XMFLOAT3 right{};
    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 projection{};

    DirectX::XMFLOAT3 eyeVector{};
    DirectX::XMFLOAT3 eyeCenter{};
    DirectX::XMFLOAT3 rockOnEyeVector{};
    DirectX::XMFLOAT3 playerToRockOn{};

    float radius{};
    float horizonDegree{};
    float verticalDegree{};

    bool rockOnStart{ true };
    float rockOnTimer{};
    bool display_camera_imgui{ false };

    static DirectX::XMFLOAT4X4 keep_view;
    static DirectX::XMFLOAT4X4 keep_projection;
    static DirectX::XMFLOAT3 keep_eye;
    static DirectX::XMFLOAT3 keep_target;
    static float keep_range;

    bool is_mouse_operation{ false };

    float up{};
    float up1{};

    bool attack{ false };
    bool attacking{ false };
    bool cameraAvoidEnd{ false };
    float lerpLate{ 1.0f };

public:
    static constexpr float AttackEndCameraTimer = 1.5f;

};