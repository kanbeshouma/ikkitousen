#pragma once
#include<DirectXMath.h>
#include"skinned_mesh.h"
#include"practical_entities.h"
#include"graphics_pipeline.h"
#include"collision.h"
#include"user.h"
#include "constants.h"

class SkyDome : PracticalEntities
{
public:
    SkyDome(GraphicsPipeline& grahics);
    ~SkyDome();
public:
    void Render(GraphicsPipeline& grahics, float elapsed_time);
    bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit);
private:
    DirectX::XMFLOAT3 position{};
    DirectX::XMFLOAT3 scale{ 0.158f , 0.158f , 0.158f };
    DirectX::XMFLOAT3 angle{};
    DirectX::XMFLOAT4 emissive_color{ 1.0f,1.0f,1.0f,3.0f };

public:
    void SetPosition(DirectX::XMFLOAT3 p) { position = p; }
    void SetScale(DirectX::XMFLOAT3 p) { scale = p; }
    void SetAngle(DirectX::XMFLOAT3 p) { angle = p; }

    static void set_purple_threshold(float threshold)
    {
        purple_threshold = threshold;
        purple_threshold = Math::clamp(purple_threshold, 0.0f, 1.0f);
    }
    static void set_red_threshold(float threshold)
    {
        red_threshold = threshold;
        red_threshold = Math::clamp(red_threshold, 0.0f, 1.0f);
    }

private:
    std::shared_ptr<SkinnedMesh> model;
    std::shared_ptr<SkinnedMesh> field;
    float field_glow_time = 0.0f;


    static float purple_threshold;
    static float red_threshold;


    //--------< •Ï” >--------//
    struct FieldConstants
    {
        float field_time;
        DirectX::XMFLOAT2 field_resolution{ 1,1 };
        float field_pad1;
    };
    std::unique_ptr<Constants<FieldConstants>> constants;
};