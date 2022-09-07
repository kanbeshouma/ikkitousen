#pragma once

#include <DirectXMath.h>
#include "skinned_mesh.h"
#include "practical_entities.h"

// �q�b�g����
struct HitResult
{
    DirectX::XMFLOAT3 position = { 0,0,0 };   // ���C�ƃ|���S���̌�_
    DirectX::XMFLOAT3 normal   = { 0,0,0 };   // �Փ˂����|���S���̖@���x�N�g��
    float distance = 0.0f;
    int material_index = -1;
};

class Collision : public PracticalEntities
{
public:
    /*----< 2D >----*/
    //--�~���m�̓����蔻��--//
    static bool hit_check_circle(const DirectX::XMFLOAT2& pos1, float r1, const DirectX::XMFLOAT2& pos2, float r2);
    // ��`���m�̓����蔻��
    static bool hit_check_rect(const DirectX::XMFLOAT2& center_a, const DirectX::XMFLOAT2& radius_a,
        const DirectX::XMFLOAT2& center_b, const DirectX::XMFLOAT2& radius_b);
    /*----< 3D >----*/
    //--���Ƌ��̌�������--//
    static bool sphere_vs_sphere(
        const DirectX::XMFLOAT3& center_a, float radius_a,
        const DirectX::XMFLOAT3& center_b, float radius_b,
        DirectX::XMFLOAT3& out_center_b);
    //--�~���Ɖ~���̌�������--//
    static bool cylinder_vs_cylinder(
        const DirectX::XMFLOAT3& position_a, float radius_a, float height_a,
        const DirectX::XMFLOAT3& position_b, float radius_b, float height_b,
        DirectX::XMFLOAT3& out_position_b);
    //--���Ɖ~���̌�������--//
    static bool sphere_vs_cylinder(
        const DirectX::XMFLOAT3& sphere_position, float sphere_radius,
        const DirectX::XMFLOAT3& cylinder_position, float cylinder_radius, float cylinder_height,
        DirectX::XMFLOAT3& out_cylinder_position);
    //--�����̂ƒ����̂̌�������--//
    static bool cuboid_vs_cuboid(
        const DirectX::XMFLOAT3& center_a, const DirectX::XMFLOAT3& radius_a,
        const DirectX::XMFLOAT3& center_b, const DirectX::XMFLOAT3& radius_b,
        DirectX::XMFLOAT3& velocity_b);
    //-----�����̂̊e�p�����[�^�[�ϊ�-----//
    // center,radius �� min position
    static const DirectX::XMFLOAT3& calc_cuboid_min_pos(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& radius)
    {
        return DirectX::XMFLOAT3(center.x - radius.x, center.y - radius.y, center.z - radius.z);
    }
    // center,radius �� max position
    static const DirectX::XMFLOAT3& calc_cuboid_max_pos(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& radius)
    {
        return DirectX::XMFLOAT3(center.x + radius.x, center.y + radius.y, center.z + radius.z);
    }
    // min,max position �� radius
    static const DirectX::XMFLOAT3& calc_cuboid_radius(const DirectX::XMFLOAT3& min_pos, const DirectX::XMFLOAT3& max_pos)
    {
        return DirectX::XMFLOAT3((max_pos.x - min_pos.x) / 2, (max_pos.y - min_pos.y) / 2, (max_pos.z - min_pos.z) / 2);
    }
    // min,max position �� center
    static const DirectX::XMFLOAT3& calc_cuboid_center(const DirectX::XMFLOAT3& min_pos, const DirectX::XMFLOAT3& max_pos)
    {
        DirectX::XMFLOAT3 radius = calc_cuboid_radius(min_pos, max_pos);
        return DirectX::XMFLOAT3(max_pos.x - radius.x, max_pos.y - radius.y, max_pos.z - radius.z);
    }
    //--������ƒ����̂̌�������--//
    static bool frustum_vs_cuboid(const DirectX::XMFLOAT3& cuboid_min_pos, const DirectX::XMFLOAT3& cuboid_max_pos);
    static bool forefront_frustum_vs_cuboid(const DirectX::XMFLOAT3& cuboid_min_pos, const DirectX::XMFLOAT3& cuboid_max_pos); // �v���C���[�ƃJ�����̊Ԃ̎�����
    //--���ƃJ�v�Z���̌�������--//
    static bool sphere_vs_capsule(
        const DirectX::XMFLOAT3& sphere_center, float sphere_radius,
        const DirectX::XMFLOAT3& capsule_start, const DirectX::XMFLOAT3& capsule_end, float capsule_radius);
    //--�J�v�Z���ƃJ�v�Z���̌�������--//
    static bool capsule_vs_capsule(
        const DirectX::XMFLOAT3& start_a, const DirectX::XMFLOAT3& end_a, float radius_a,
        const DirectX::XMFLOAT3& start_b, const DirectX::XMFLOAT3& end_b, float radius_b);
    //--���C�ƃ��f���̌�������--//
    static bool ray_vs_model(
        const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const SkinnedMesh* model,
        const DirectX::XMFLOAT3& model_position, const DirectX::XMFLOAT3& model_scale, const DirectX::XMFLOAT3& model_angle,
        const DirectX::XMFLOAT4X4& model_coordinate, HitResult& result);
    static bool ray_vs_model(
        const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const SkinnedMesh* model,
        const DirectX::XMFLOAT3& model_position, const DirectX::XMFLOAT3& model_scale, const DirectX::XMFLOAT3& model_angle, HitResult& result);
};