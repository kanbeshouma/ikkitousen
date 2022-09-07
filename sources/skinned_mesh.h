#pragma once

//----cereal----//
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#include <vector>
#include <string>
#include <fbxsdk.h>
#include <unordered_map>

#include "constants.h"

namespace DirectX
{
    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT2& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT3& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT4& v)
    {
        archive(
            cereal::make_nvp("x", v.x),
            cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z),
            cereal::make_nvp("w", v.w)
        );
    }

    template<class T>
    void serialize(T& archive, DirectX::XMFLOAT4X4& m)
    {
        archive(
            cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12),
            cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
            cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22),
            cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
            cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32),
            cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
            cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42),
            cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
        );
    }
}

struct scene
{
    struct node
    {
        uint64_t unique_id{ 0 };
        std::string name;
        FbxNodeAttribute::EType attribute{ FbxNodeAttribute::EType::eUnknown };
        int64_t parent_index{ -1 };

        template<class T>
        void serialize(T& archive) { archive(unique_id, name, attribute, parent_index); };
    };
    std::vector<node> nodes;
    int64_t indexof(uint64_t unique_id) const
    {
        int64_t index{ 0 };
        for (const node& node : nodes)
        {
            if (node.unique_id == unique_id)
            {
                return index;
            }
            ++index;
        }
        return -1;
    }
    template<class T>
    void serialize(T& archive) { archive(nodes); };
};

struct skeleton
{
    struct bone
    {
        uint64_t unique_id{ 0 };
        std::string name;
        // 'parent_index' is index that refers to the parent bone's position in the array that contains itself.
        int64_t parent_index{ -1 }; // -1 : the bone is orphan
        // 'node_index' is an index that refers to the node array of the scene.
        int64_t node_index{ 0 };
        // 'offset_transform' is used to convert from model(mesh) space to bone(node) scene.
        DirectX::XMFLOAT4X4 offset_transform{
            1, 0, 0, 0,    0, 1, 0, 0,    0, 0, 1, 0,    0, 0, 0, 1
        };

        bool is_orphan() const { return parent_index < 0; };

        template<class T>
        void serialize(T& archive) { archive(unique_id, name, parent_index, node_index, offset_transform); }
    };
    std::vector<bone> bones;
    int64_t indexof(uint64_t unique_id) const
    {
        int64_t index{ 0 };
        for (const bone& bone : bones)
        {
            if (bone.unique_id == unique_id)
            {
                return index;
            }
            ++index;
        }
        return -1;
    }
    template<class T>
    void serialize(T& archive) { archive(bones); };
};

struct animation
{
    std::string name;
    float sampling_rate{ 0 };

    struct keyframe
    {
        struct node
        {
            // 'global_transform' is used to convert from local space of node to global space of scene.
            DirectX::XMFLOAT4X4 global_transform{
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
            // The transformation data of a node includes its translation, rotation and scaling vectors
            // with respect to its parent.
            DirectX::XMFLOAT3 scaling{ 1, 1, 1 };
            DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 }; // Rotation quaternion
            DirectX::XMFLOAT3 translation{ 0, 0, 0 };

            template<class T>
            void serialize(T& archive) { archive(global_transform, scaling, rotation, translation); }
        };
        std::vector<node> nodes;

        template<class T>
        void serialize(T& archive) { archive(nodes); };
    };
    std::vector<keyframe> sequence;

    template<class T>
    void serialize(T& archive) { archive(name, sampling_rate, sequence); };
};

class SkinnedMesh
{
public:
    //--------<constructor/destructor>--------//
    SkinnedMesh(ID3D11Device* device, const char* fbx_filename, bool triangulate = false, float sampling_rate = 0);
    SkinnedMesh(ID3D11Device* device, const char* fbx_filename, std::string sub_colors[2], bool triangulate = false, float sampling_rate = 0);
    virtual ~SkinnedMesh() = default;
private:
    //--------< 変数 >--------//
    static const int MAX_BONE_INFLUENCES{ 4 };
    static const int MAX_BONES{ 256 };
public:
    //--------< 構造体 >--------//
    struct vertex
    {
        DirectX::XMFLOAT3 position{};
        DirectX::XMFLOAT3 normal{};
        DirectX::XMFLOAT4 tangent{};
        DirectX::XMFLOAT2 texcoord{};
        float bone_weights[MAX_BONE_INFLUENCES]{ 1,0,0,0 };
        uint32_t bone_indices[MAX_BONE_INFLUENCES]{};

        template<class T>
        void serialize(T& archive) { archive(position, normal, tangent, texcoord, bone_weights, bone_indices); }
    };
    struct mesh
    {
        uint64_t unique_id{ 0 };
        std::string name;
        // 'node_index' is an index that refers to the node array of the scene.
        int64_t node_index{ 0 };

        std::vector<vertex> vertices;
        std::vector<uint32_t> indices;
        // マテリアル単位で描画を行う
        struct subset
        {
            uint64_t material_unique_id{ 0 };
            std::string material_name;
            uint32_t start_index_location{ 0 };
            uint32_t index_count{ 0 };

            template<class T>
            void serialize(T& archive) { archive(material_unique_id, material_name, start_index_location, index_count); }
        };
        std::vector<subset> subsets;

        template<class T>
        void serialize(T& archive)
        {
            archive(unique_id, name, node_index, subsets, default_global_transform,
                bind_pose, bounding_box, vertices, indices);
        }
        DirectX::XMFLOAT4X4 default_global_transform{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1 };
    private:
        skeleton bind_pose;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
        DirectX::XMFLOAT3 bounding_box[2]
        {
            { +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX },
            { -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX }
        };
        friend class SkinnedMesh;
    };
    struct anim_Parameters
    {
        int current_anim_index        = 0;       // 現在のアニメーションのインデック
        int frame_index               = 0;       // 再生中のアニメーションのフレーム
        float animation_tick          = 0;       // フレームの中の再生時間
        bool do_loop                  = false;   // ループ再生するか
        bool stop_animation           = false;   // アニメーションの停止
        bool end_of_animation         = false;   // アニメーションが1ループ再生したか
        float animation_blend_time    = 0.0f;    // アニメーション間の補完時間
        float animation_blend_seconds = 0.0f;    // アニメーション間の補完割合
        bool interpolation            = true;    // 補完するか
        float playback_speed          = 1.0f;    // 再生速度

        animation animation{};
        animation::keyframe current_keyframe{};

        animation::keyframe old_keyframe{};
    };
private:
    struct GeometryConstants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 material_color;
        DirectX::XMFLOAT4X4 bone_transforms[MAX_BONES]{
            {1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1}
        };
        DirectX::XMFLOAT4 emissive_color{ 1,1,1,1 }; // xyz:emissive_color w:emissive_strength
        DirectX::XMFLOAT4 dissolve_threshold{ 0,0,0,0 };
        float glow_thickness = 0.8f; // glowの幅
        DirectX::XMFLOAT3 pad1;
        float sub_color_threshold_purple = 0;
        float sub_color_threshold_red = 0;
        DirectX::XMFLOAT2 pad2;
    };
    struct material
    {
        uint64_t unique_id{ 0 };
        std::string name;

        DirectX::XMFLOAT4 Ka{ 0.2f, 0.2f, 0.2f, 1.0f };
        DirectX::XMFLOAT4 Kd{ 0.8f, 0.8f, 0.8f, 1.0f };
        DirectX::XMFLOAT4 Ks{ 1.0f, 1.0f, 1.0f, 1.0f };

        static const int TEXTURE_COUNT = 8;
        std::string texture_filenames[TEXTURE_COUNT];
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[TEXTURE_COUNT];

        template<class T>
        void serialize(T& archive) { archive(unique_id, name, Ka, Kd, Ks, texture_filenames); }
    };

    // TEXTURE_COUNT増やすとシリアル作り直しになり、制作後半にそれはしんどいのでここで増やす
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sub_color_shader_resource_views[2]; // 0:purple 1:red

public:
    //--------< 関数 >--------//
    typedef std::tuple<std::string, float> mesh_tuple;
    void render(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& world,
        const DirectX::XMFLOAT4& material_color, float threshold = 0, float glow_time = 0,
        const DirectX::XMFLOAT4& emissive_color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        float glow_thickness = 0.8f, float purple_threshold = 0, float red_threshold = 0, std::string sub_color_mesh_name = "");
    void render(ID3D11DeviceContext* dc, anim_Parameters& para, const DirectX::XMFLOAT4X4& world,
        const DirectX::XMFLOAT4& material_color, float threshold = 0, float glow_time = 0,
        const DirectX::XMFLOAT4& emissive_color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), float glow_thickness = 0.8f);
    // mesh検索
    template<class... A> void render(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& world,
        const DirectX::XMFLOAT4& material_color, float threshold, float glow_time,
        const DirectX::XMFLOAT4& emissive_color, float glow_thickness, A... mesh_tuples)
    {
        for (const mesh& mesh : meshes)
        {
            //if (!Collision::frustum_vs_cuboid(world_min_bounding_box, world_max_bounding_box))
            //{
            //    continue;
            //}

            for (mesh_tuple tup : std::initializer_list<mesh_tuple>{ mesh_tuples... })
            {
                if (mesh.name == std::get<0>(tup))
                {
                    geometry_constants->data.dissolve_threshold.x = std::get<1>(tup);
                    break;
                }
                else
                {
                    geometry_constants->data.dissolve_threshold.x = threshold;
                }
            }
            geometry_constants->data.dissolve_threshold.y = glow_time;
            geometry_constants->data.emissive_color = emissive_color;
            geometry_constants->data.glow_thickness = glow_thickness;
            geometry_constants->data.sub_color_threshold_purple = 0;
            geometry_constants->data.sub_color_threshold_red = 0;


            uint32_t stride{ sizeof(vertex) };
            uint32_t offset{ 0 };
            dc->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &stride, &offset);
            dc->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // 定数バッファのバインド
            {
                if (&anim_para.current_keyframe && (&anim_para.current_keyframe)->nodes.size() > 0)
                {
                    // ※メッシュのglobal_transform（位置・姿勢）が時間軸で変化しているので、その行列をキーフレームから取得する
                    // ※取得したglobal_transform行列を定数バッファのワールド変換行列に合成する
                    const animation::keyframe::node& mesh_node{ (&anim_para.current_keyframe)->nodes.at(mesh.node_index) };
                    XMStoreFloat4x4(&geometry_constants->data.world, XMLoadFloat4x4(&mesh_node.global_transform) * XMLoadFloat4x4(&world));

                    const size_t bone_count{ mesh.bind_pose.bones.size() };
                    _ASSERT_EXPR(bone_count < MAX_BONES, L"The value of the 'bone_count' has exceeded MAX_BONES.");
                    for (int bone_index = 0; bone_index < bone_count; ++bone_index)
                    {
                        const skeleton::bone& bone{ mesh.bind_pose.bones.at(bone_index) };
                        const animation::keyframe::node& bone_node{ (&anim_para.current_keyframe)->nodes.at(bone.node_index) };
                        XMStoreFloat4x4(&geometry_constants->data.bone_transforms[bone_index],
                            XMLoadFloat4x4(&bone.offset_transform) *
                            XMLoadFloat4x4(&bone_node.global_transform) *
                            XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh.default_global_transform))
                        );
                    }
                }
                else
                {
                    XMStoreFloat4x4(&geometry_constants->data.world,
                        XMLoadFloat4x4(&mesh.default_global_transform) * XMLoadFloat4x4(&world));
                    for (size_t bone_index = 0; bone_index < MAX_BONES; ++bone_index)
                    {
                        geometry_constants->data.bone_transforms[bone_index] = {
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1,
                        };
                    }
                }
                for (const mesh::subset& subset : mesh.subsets)
                {
                    using namespace DirectX;
                    const material& material{ materials.at(subset.material_unique_id) };

                    XMStoreFloat4(&geometry_constants->data.material_color, XMLoadFloat4(&material_color) * XMLoadFloat4(&material.Kd));
                    geometry_constants->bind(dc, 0, CB_FLAG::PS_VS);
                    // シェーダーリソースビューのセット
                    dc->PSSetShaderResources(0, 1, material.shader_resource_views[0].GetAddressOf());
                    dc->PSSetShaderResources(1, 1, material.shader_resource_views[1].GetAddressOf());
                    dc->PSSetShaderResources(2, 1, material.shader_resource_views[2].GetAddressOf());
                    dc->PSSetShaderResources(3, 1, material.shader_resource_views[3].GetAddressOf());
                    dc->PSSetShaderResources(4, 1, material.shader_resource_views[4].GetAddressOf());
                    dc->PSSetShaderResources(5, 1, material.shader_resource_views[5].GetAddressOf());
                    dc->PSSetShaderResources(8, 1, material.shader_resource_views[6].GetAddressOf());
                    dc->PSSetShaderResources(9, 1, material.shader_resource_views[7].GetAddressOf());
                    dc->PSSetShaderResources(20, 1, sub_color_shader_resource_views[0].GetAddressOf());
                    dc->PSSetShaderResources(21, 1, sub_color_shader_resource_views[1].GetAddressOf());
                    // 描画
                    dc->DrawIndexed(subset.index_count, subset.start_index_location, 0);
                }
            }
        }
    }

    template<class... A> void render(ID3D11DeviceContext* dc, anim_Parameters& para, const DirectX::XMFLOAT4X4& world,
        const DirectX::XMFLOAT4& material_color, float threshold, float glow_time,
        const DirectX::XMFLOAT4& emissive_color, float glow_thickness, A... mesh_tuples)
    {
        for (const mesh& mesh : meshes)
        {
            for (mesh_tuple tup : std::initializer_list<mesh_tuple>{ mesh_tuples... })
            {
                if (mesh.name == std::get<0>(tup))
                {
                    geometry_constants->data.dissolve_threshold.x = std::get<1>(tup);
                    break;
                }
                else
                {
                    geometry_constants->data.dissolve_threshold.x = threshold;
                }
            }
            geometry_constants->data.dissolve_threshold.y = glow_time;
            geometry_constants->data.emissive_color = emissive_color;
            geometry_constants->data.glow_thickness = glow_thickness;
            geometry_constants->data.sub_color_threshold_purple = 0;
            geometry_constants->data.sub_color_threshold_red = 0;


            uint32_t stride{ sizeof(vertex) };
            uint32_t offset{ 0 };
            dc->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &stride, &offset);
            dc->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // 定数バッファのバインド
            {
                if (&para.current_keyframe && (&para.current_keyframe)->nodes.size() > 0)
                {
                    // ※メッシュのglobal_transform（位置・姿勢）が時間軸で変化しているので、その行列をキーフレームから取得する
                    // ※取得したglobal_transform行列を定数バッファのワールド変換行列に合成する
                    const animation::keyframe::node& mesh_node{ (&para.current_keyframe)->nodes.at(mesh.node_index) };
                    XMStoreFloat4x4(&geometry_constants->data.world, XMLoadFloat4x4(&mesh_node.global_transform) * XMLoadFloat4x4(&world));

                    const size_t bone_count{ mesh.bind_pose.bones.size() };
                    _ASSERT_EXPR(bone_count < MAX_BONES, L"The value of the 'bone_count' has exceeded MAX_BONES.");
                    for (int bone_index = 0; bone_index < bone_count; ++bone_index)
                    {
                        const skeleton::bone& bone{ mesh.bind_pose.bones.at(bone_index) };
                        const animation::keyframe::node& bone_node{ (&para.current_keyframe)->nodes.at(bone.node_index) };
                        XMStoreFloat4x4(&geometry_constants->data.bone_transforms[bone_index],
                            XMLoadFloat4x4(&bone.offset_transform) *
                            XMLoadFloat4x4(&bone_node.global_transform) *
                            XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh.default_global_transform))
                        );
                    }
                }
                else
                {
                    XMStoreFloat4x4(&geometry_constants->data.world,
                        XMLoadFloat4x4(&mesh.default_global_transform) * XMLoadFloat4x4(&world));
                    for (size_t bone_index = 0; bone_index < MAX_BONES; ++bone_index)
                    {
                        geometry_constants->data.bone_transforms[bone_index] = {
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1,
                        };
                    }
                }
                for (const mesh::subset& subset : mesh.subsets)
                {
                    using namespace DirectX;
                    const material& material{ materials.at(subset.material_unique_id) };

                    XMStoreFloat4(&geometry_constants->data.material_color, XMLoadFloat4(&material_color) * XMLoadFloat4(&material.Kd));
                    geometry_constants->bind(dc, 0, CB_FLAG::PS_VS);
                    // シェーダーリソースビューのセット
                    dc->PSSetShaderResources(0, 1, material.shader_resource_views[0].GetAddressOf());
                    dc->PSSetShaderResources(1, 1, material.shader_resource_views[1].GetAddressOf());
                    dc->PSSetShaderResources(2, 1, material.shader_resource_views[2].GetAddressOf());
                    dc->PSSetShaderResources(3, 1, material.shader_resource_views[3].GetAddressOf());
                    dc->PSSetShaderResources(4, 1, material.shader_resource_views[4].GetAddressOf());
                    dc->PSSetShaderResources(5, 1, material.shader_resource_views[5].GetAddressOf());
                    dc->PSSetShaderResources(8, 1, material.shader_resource_views[6].GetAddressOf());
                    dc->PSSetShaderResources(9, 1, material.shader_resource_views[7].GetAddressOf());
                    dc->PSSetShaderResources(20, 1, sub_color_shader_resource_views[0].GetAddressOf());
                    dc->PSSetShaderResources(21, 1, sub_color_shader_resource_views[1].GetAddressOf());
                    // 描画
                    dc->DrawIndexed(subset.index_count, subset.start_index_location, 0);
                }
            }
        }
    }


    // アニメーション
    void play_animation(int animation_index, bool is_loop = false, bool interpolation = true, float blend_seconds = 0.3f, float playback_speed = 1.0f);
    void play_animation(anim_Parameters& para, int animation_index, bool is_loop = false, bool interpolation = true, float blend_seconds = 0.3f, float playback_speed = 1.0f);
    void pause_animation() { anim_para.stop_animation = true; }
    void pause_animation(anim_Parameters& para) { para.stop_animation = true; }
    void progress_animation() { anim_para.stop_animation = false; }
    void progress_animation(anim_Parameters& para) { para.stop_animation = false; }
    void update_animation(float elapsed_time);
    void update_animation(anim_Parameters& para, float elapsed_time);
    bool end_of_animation() const { return anim_para.end_of_animation; }
    bool end_of_animation(anim_Parameters& para) const { return para.end_of_animation; }

    const skeleton::bone& get_bone_by_name(std::string name);
    void fech_by_bone(const DirectX::XMFLOAT4X4& world, const skeleton::bone& bone, DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& up);
    void fech_by_bone(anim_Parameters& para, const DirectX::XMFLOAT4X4& world, const skeleton::bone& bone,
        DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& up,DirectX::XMFLOAT4X4& rotation_mat = DirectX::XMFLOAT4X4{});

    void fech_by_bone(anim_Parameters& para, const DirectX::XMFLOAT4X4& world, const skeleton::bone& bone, DirectX::XMFLOAT4X4& ResultMat);
    //--------<getter/setter>--------//
    const std::vector<mesh>& get_meshes() const { return meshes; }
    const anim_Parameters& get_anim_para() const { return anim_para; }
private:
    //--------< 変数 >--------//
    std::unique_ptr<Constants<GeometryConstants>> geometry_constants;
    // FBXファイルからメッシュが使用するマテリアル情報（色・テクスチャ）を抽出する
    std::unordered_map<uint64_t, material> materials;
    anim_Parameters anim_para;
    std::vector<mesh> meshes;
    std::vector<animation> animation_clips;
private:
    //--------< 関数 >--------//
    void fetch_meshes(FbxScene* fbx_scene, std::vector<mesh>& meshes);
    void fetch_materials(const char* fbx_filename, FbxScene* fbx_scene, std::unordered_map<uint64_t, material>& materials);
    // FBXメッシュからバインドポーズの情報を抽出する
    void fetch_skeleton(FbxMesh* fbx_mesh, skeleton& bind_pose);
    void create_com_objects(ID3D11Device* device, const char* fbx_filename);
    // FBXシーンからアニメーションの情報を抽出する
    void fetch_animations(FbxScene* fbx_scene, std::vector<animation>& animation_clips,
        float sampling_rate /*If this value is 0, the animation data will be sampled at the default frame rate.*/);
    // アニメーション関連
    void update_blend_animation(animation::keyframe& keyframe);
    bool append_animations(const char* animation_filename, float sampling_rate);
    void blend_animations(const animation::keyframe* keyframes[2], float factor, animation::keyframe& keyframe);
    // 固有IDからノードを取得
    FbxNode* find_fbxNode_by_uniqueId(FbxNode* fbx_node, uint64_t id);
    FbxNode* find_fbxNode_by_uniqueId(FbxScene* fbx_scene, uint64_t id);
protected:
    scene scene_view;
};