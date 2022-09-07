#include <sstream>
#include <functional>
#include <filesystem>
#include <fstream>
#include "misc.h"
#include "skinned_mesh.h"
#include "user.h"
#include "shader.h"
#include "texture.h"
#include "collision.h"

#include"imgui_include.h"

//----------------------------------------------
//
//    inline関数
//
//----------------------------------------------
inline DirectX::XMFLOAT4X4 to_xmfloat4x4(const FbxAMatrix& fbxamatrix)
{
    DirectX::XMFLOAT4X4 xmfloat4x4;
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
        }
    }
    return xmfloat4x4;
}
inline DirectX::XMFLOAT3 to_xmfloat3(const FbxDouble3& fbxdouble3)
{
    DirectX::XMFLOAT3 xmfloat3;
    xmfloat3.x = static_cast<float>(fbxdouble3[0]);
    xmfloat3.y = static_cast<float>(fbxdouble3[1]);
    xmfloat3.z = static_cast<float>(fbxdouble3[2]);
    return xmfloat3;
}
inline DirectX::XMFLOAT4 to_xmfloat4(const FbxDouble4& fbxdouble4)
{
    DirectX::XMFLOAT4 xmfloat4;
    xmfloat4.x = static_cast<float>(fbxdouble4[0]);
    xmfloat4.y = static_cast<float>(fbxdouble4[1]);
    xmfloat4.z = static_cast<float>(fbxdouble4[2]);
    xmfloat4.w = static_cast<float>(fbxdouble4[3]);
    return xmfloat4;
}
//-------------------------------------------------------
//
//         構造体宣言
//
//-------------------------------------------------------
// ※bone_influence 構造体の１つのインスタンスは１つ頂点が影響を受けるボーン番号とその重みを表現する
// ※１つ頂点は複数のボーンから影響を受ける場合があるので可変長配列で表現している
struct bone_influence
{
    uint32_t bone_index;
    float bone_weight;
};
using bone_influences_per_control_point = std::vector<bone_influence>;
//-------------------------------------------------------
//
//         関数
//
//-------------------------------------------------------
// ボーン影響度をFBXデータから取得する
void fetch_bone_influences(const FbxMesh* fbx_mesh,
    std::vector<bone_influences_per_control_point>& bone_influences)
{
    const int control_points_count{ fbx_mesh->GetControlPointsCount() };
    bone_influences.resize(control_points_count);

    const int skin_count{ fbx_mesh->GetDeformerCount(FbxDeformer::eSkin) };
    for (int skin_index = 0; skin_index < skin_count; ++skin_index)
    {
        const FbxSkin* fbx_skin
        { static_cast<FbxSkin*>(fbx_mesh->GetDeformer(skin_index, FbxDeformer::eSkin)) };

        const int cluster_count{ fbx_skin->GetClusterCount() };
        for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
        {
            const FbxCluster* fbx_cluster{ fbx_skin->GetCluster(cluster_index) };

            const int control_point_indices_count{ fbx_cluster->GetControlPointIndicesCount() };
            for (int control_point_indices_index = 0; control_point_indices_index < control_point_indices_count;
                ++control_point_indices_index)
            {
                int control_point_index{ fbx_cluster->GetControlPointIndices()[control_point_indices_index] };
                double control_point_weight
                { fbx_cluster->GetControlPointWeights()[control_point_indices_index] };
                bone_influence& bone_influence{ bone_influences.at(control_point_index).emplace_back() };
                bone_influence.bone_index = static_cast<uint32_t>(cluster_index);
                bone_influence.bone_weight = static_cast<float>(control_point_weight);
            }
        }
    }
}
//-------------------------------------------------------
//
//         SkinnedMeshクラス
//
//-------------------------------------------------------
SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbx_filename, bool triangulate, float sampling_rate)
{
    std::filesystem::path cereal_filename(fbx_filename);
    if (!std::filesystem::exists(cereal_filename.c_str()))
    {
        _ASSERT_EXPR(false, L"指定されたfbxファイルが存在しません");
    }

    // ※このコードでは与えられた変数fbx_filenameの拡張子を“cereal”に変え、そのファイルが存在する場合はシリアライズされたデータか
    // らロードする。また、存在しない場合は従来通りFBXファイルからロードする。
    // ※skinnd_meshクラスのデータ構造に変更があった場合はシリアライズされたファイルは無効になるので削除する必要がある。
    cereal_filename.replace_extension("cereal");
    if (std::filesystem::exists(cereal_filename.c_str()))
    {
        std::ifstream ifs(cereal_filename.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(scene_view, meshes, materials, animation_clips);
        
    }
    else
    {
        FbxManager* fbx_manager{ FbxManager::Create() };
        FbxScene* fbx_scene = { FbxScene::Create(fbx_manager,"") };

        FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager,"") };
        bool import_status{ false };
        import_status = fbx_importer->Initialize(fbx_filename);
        _ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());

        import_status = fbx_importer->Import(fbx_scene);
        _ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());

        FbxGeometryConverter fbx_converter(fbx_manager);
        if (triangulate)
        {
            fbx_converter.Triangulate(fbx_scene, true/*replace*/, false/*legacy*/);
            fbx_converter.RemoveBadPolygonsFromMeshes(fbx_scene);
        }

        std::function<void(FbxNode*)> traverse{ [&](FbxNode* fbx_node) {
            scene::node& node{scene_view.nodes.emplace_back()};
            node.attribute = fbx_node->GetNodeAttribute() ?
                fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
            node.name = fbx_node->GetName();
            node.unique_id = fbx_node->GetUniqueID();
            node.parent_index = scene_view.indexof(fbx_node->GetParent() ?
                fbx_node->GetParent()->GetUniqueID() : 0);
            for (int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index)
            {
                traverse(fbx_node->GetChild(child_index));
            }
        } };
        traverse(fbx_scene->GetRootNode());

        fetch_meshes(fbx_scene, meshes);
        fetch_materials(fbx_filename, fbx_scene, materials);
        fetch_animations(fbx_scene, animation_clips, sampling_rate);

        fbx_manager->Destroy();

        std::ofstream ofs(cereal_filename.c_str(), std::ios::binary);
        cereal::BinaryOutputArchive serialization(ofs);
        serialization(scene_view, meshes, materials, animation_clips);
    }
    create_com_objects(device, fbx_filename);
#if 0
    for (const scene::node& node : scene_view.nodes)
    {
        FbxNode* fbx_node{ fbx_scene->FindNodeByName(node.name.c_str()) };
        // Display node data in the output window as debug
        std::string node_name = fbx_node->GetName();
        uint64_t uid = fbx_node->GetUniqueID();
        uint64_t parent_uid = fbx_node->GetParent() ? fbx_node->GetParent()->GetUniqueID() : 0;
        int32_t type = fbx_node->GetNodeAttribute() ? fbx_node->GetNodeAttribute()->GetAttributeType() : 0;
        std::stringstream debug_string;
        debug_string << node_name << ":" << uid << ":" << parent_uid << ":" << type << "\n";
        OutputDebugStringA(debug_string.str().c_str());
    }
#endif
}

SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbx_filename,
    std::string sub_colors[2], bool triangulate, float sampling_rate)
    : SkinnedMesh(device, fbx_filename, triangulate, sampling_rate)
{
    // sub color map作成
    for (size_t texture_index = 0; texture_index < 2; ++texture_index)
    {
        if (sub_colors[texture_index].size() > 0)
        {
            std::filesystem::path path(fbx_filename);
            path.replace_filename(sub_colors[texture_index]);
            D3D11_TEXTURE2D_DESC texture2d_desc;
            load_texture_from_file(device, path.c_str(),
                sub_color_shader_resource_views[texture_index].GetAddressOf(), &texture2d_desc);
        }
    }
}


const skeleton::bone& SkinnedMesh::get_bone_by_name(std::string name)
{
    skeleton::bone dummy = {};
    for (const mesh& mesh : meshes)
    {
        const size_t bone_count{ mesh.bind_pose.bones.size() };
        _ASSERT_EXPR(bone_count < MAX_BONES, L"The value of the 'bone_count' has exceeded MAX_BONES.");
        for (int bone_index = 0; bone_index < bone_count; ++bone_index)
        {
            const skeleton::bone& bone = mesh.bind_pose.bones.at(bone_index);
            if (bone.name == name)
            {
                return bone;
            }
        }
    }
    _ASSERT_EXPR(false,"指定された名前のボーンがありません");
    return dummy;
}

void SkinnedMesh::fech_by_bone(const DirectX::XMFLOAT4X4& world, const skeleton::bone& bone, DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& up)
{
    if (&anim_para.current_keyframe && (&anim_para.current_keyframe)->nodes.size() > 0)
    {
        const animation::keyframe::node& bone_node{ (&anim_para.current_keyframe)->nodes.at(bone.node_index) };
        DirectX::XMFLOAT4X4 w;
        XMStoreFloat4x4(&w, XMLoadFloat4x4(&bone_node.global_transform) * XMLoadFloat4x4(&world));

        pos = { w._41,w._42,w._43 };
        DirectX::XMFLOAT3 scale = { Math::Length({w._11,w._12,w._13}),  Math::Length({w._21,w._22,w._23}),  Math::Length({w._31,w._32,w._33}) };

        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z) };
        DirectX::XMMATRIX R = DirectX::XMLoadFloat4x4(&w) * DirectX::XMMatrixInverse(nullptr, S) * DirectX::XMMatrixInverse(nullptr, T);

        DirectX::XMFLOAT4X4 r = {};
        DirectX::XMStoreFloat4x4(&r, R);
        DirectX::XMVECTOR right_vec = { r._11, r._12, r._13 };
        XMStoreFloat3(&up, right_vec);
    }
}

void SkinnedMesh::fech_by_bone(anim_Parameters& para, const DirectX::XMFLOAT4X4& world,
    const skeleton::bone& bone, DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& up, DirectX::XMFLOAT4X4& rotation_mat)
{
    if (&para.current_keyframe && (&para.current_keyframe)->nodes.size() > 0)
    {
        const animation::keyframe::node& bone_node{ (&para.current_keyframe)->nodes.at(bone.node_index) };
        DirectX::XMFLOAT4X4 w;
        XMStoreFloat4x4(&w, XMLoadFloat4x4(&bone_node.global_transform) * XMLoadFloat4x4(&world));

        pos = { w._41,w._42,w._43 };
        DirectX::XMFLOAT3 scale = { Math::Length({w._11,w._12,w._13}),  Math::Length({w._21,w._22,w._23}),  Math::Length({w._31,w._32,w._33}) };

        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z) };
        DirectX::XMMATRIX R = DirectX::XMLoadFloat4x4(&w) * DirectX::XMMatrixInverse(nullptr, S) * DirectX::XMMatrixInverse(nullptr, T);

        DirectX::XMFLOAT4X4 r = {};
        DirectX::XMStoreFloat4x4(&r, R);
        rotation_mat = r;
        using namespace DirectX;
        XMVECTOR Rot = XMLoadFloat4(&bone_node.rotation);
        //DirectX::XMStoreFloat4(&quaternion, XMVector4Transform(Rot, XMLoadFloat4x4(&world)));
        //quaternion = bone_node.rotation;

        DirectX::XMVECTOR right_vec = { r._11, r._12, r._13 };
        XMStoreFloat3(&up, right_vec);

    }
}

void SkinnedMesh::fech_by_bone(anim_Parameters& para,
                              const DirectX::XMFLOAT4X4& world,
    const skeleton::bone& bone,
    DirectX::XMFLOAT4X4& ResultMat)
{
    if (&para.current_keyframe && (&para.current_keyframe)->nodes.size() > 0)
    {
        const animation::keyframe::node& bone_node{ (&para.current_keyframe)->nodes.at(bone.node_index) };
        DirectX::XMFLOAT4X4 w;
        XMStoreFloat4x4(&w, XMLoadFloat4x4(&bone_node.global_transform) * XMLoadFloat4x4(&world));

        DirectX::XMFLOAT3 pos = { w._41,w._42,w._43 };
        DirectX::XMFLOAT3 scale = { Math::Length({w._11,w._12,w._13}),  Math::Length({w._21,w._22,w._23}),  Math::Length({w._31,w._32,w._33}) };

        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z) };
        DirectX::XMMATRIX R = DirectX::XMLoadFloat4x4(&w) * DirectX::XMMatrixInverse(nullptr, S) * DirectX::XMMatrixInverse(nullptr, T);

        DirectX::XMStoreFloat4x4(&ResultMat, S * R * T);
    }


}

void SkinnedMesh::render(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& world,
    const DirectX::XMFLOAT4& material_color, float threshold, float glow_time,
    const DirectX::XMFLOAT4& emissive_color, float glow_thickness,
    float purple_threshold, float red_threshold, std::string sub_color_mesh_name)
{
    geometry_constants->data.dissolve_threshold.x = threshold;
    geometry_constants->data.dissolve_threshold.y = glow_time;
    geometry_constants->data.emissive_color = emissive_color;
    geometry_constants->data.glow_thickness = glow_thickness;


    for (const mesh& mesh : meshes)
    {
        if (mesh.name == sub_color_mesh_name)
        {
            geometry_constants->data.sub_color_threshold_purple = purple_threshold;
            geometry_constants->data.sub_color_threshold_red    = red_threshold;
        }
        else
        {
            geometry_constants->data.sub_color_threshold_purple = 0;
            geometry_constants->data.sub_color_threshold_red    = 0;
        }

        //if (!Collision::frustum_vs_cuboid(world_min_bounding_box, world_max_bounding_box))
        //{
        //    continue;
        //}

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

void SkinnedMesh::render(ID3D11DeviceContext* dc, anim_Parameters& para, const DirectX::XMFLOAT4X4& world,
    const DirectX::XMFLOAT4& material_color, float threshold, float glow_time, const DirectX::XMFLOAT4& emissive_color, float glow_thickness)
{
    geometry_constants->data.dissolve_threshold.x = threshold;
    geometry_constants->data.dissolve_threshold.y = glow_time;
    geometry_constants->data.emissive_color = emissive_color;
    geometry_constants->data.glow_thickness = glow_thickness;

    geometry_constants->data.sub_color_threshold_purple = 0;
    geometry_constants->data.sub_color_threshold_red = 0;


    for (const mesh& mesh : meshes)
    {
        //if (!Collision::frustum_vs_cuboid(world_min_bounding_box, world_max_bounding_box))
        //{
        //    continue;
        //}

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

void SkinnedMesh::fetch_meshes(FbxScene* fbx_scene, std::vector<mesh>& meshes)
{
    for (const scene::node& node : scene_view.nodes)
    {
        if (node.attribute != FbxNodeAttribute::EType::eMesh)
        {
            continue;
        }

        FbxNode* fbx_node{ find_fbxNode_by_uniqueId(fbx_scene, node.unique_id) };
        //FbxNode* fbx_node{ fbx_scene->FindNodeByName(node.name.c_str()) };
        FbxMesh* fbx_mesh{ fbx_node->GetMesh() };

        mesh& mesh{ meshes.emplace_back() };
        mesh.unique_id = fbx_mesh->GetNode()->GetUniqueID();
        mesh.name = fbx_mesh->GetNode()->GetName();
        mesh.node_index = scene_view.indexof(mesh.unique_id);
        mesh.default_global_transform = to_xmfloat4x4(fbx_mesh->GetNode()->EvaluateGlobalTransform());
        // バウンディングボックスのパラメータの設定
        for (const vertex& v : mesh.vertices)
        {
            mesh.bounding_box[0].x = std::min<float>(mesh.bounding_box[0].x, v.position.x);
            mesh.bounding_box[0].y = std::min<float>(mesh.bounding_box[0].y, v.position.y);
            mesh.bounding_box[0].z = std::min<float>(mesh.bounding_box[0].z, v.position.z);
            mesh.bounding_box[1].x = std::max<float>(mesh.bounding_box[1].x, v.position.x);
            mesh.bounding_box[1].y = std::max<float>(mesh.bounding_box[1].y, v.position.y);
            mesh.bounding_box[1].z = std::max<float>(mesh.bounding_box[1].z, v.position.z);
        }
        // 頂点が影響を受けるボーン番号とウエイト値を頂点構造体(vertex)のメンバ変数にセットする
        std::vector<bone_influences_per_control_point> bone_influences;
        fetch_bone_influences(fbx_mesh, bone_influences);
        // FBXメッシュからバインドポーズの情報を抽出する
        fetch_skeleton(fbx_mesh, mesh.bind_pose);

        std::vector<mesh::subset>& subsets{ mesh.subsets };
        const int material_count{ fbx_mesh->GetNode()->GetMaterialCount() };
        subsets.resize(material_count > 0 ? material_count : 1);
        for (int material_index = 0; material_index < material_count; ++material_index)
        {
            const FbxSurfaceMaterial* fbx_material{ fbx_mesh->GetNode()->GetMaterial(material_index) };
            subsets.at(material_index).material_name = fbx_material->GetName();
            subsets.at(material_index).material_unique_id = fbx_material->GetUniqueID();
        }
        if (material_count > 0)
        {
            const int polygon_count{ fbx_mesh->GetPolygonCount() };
            for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
            {
                const int material_index{ fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(polygon_index) };
                subsets.at(material_index).index_count += 3;
            }
            uint32_t offset{ 0 };
            for (mesh::subset& subset : subsets)
            {
                subset.start_index_location = offset;
                offset += subset.index_count;
                // This will be used as counter in the following procedures, reset to zero
                subset.index_count = 0;
            }
        }
        const int polygon_count{ fbx_mesh->GetPolygonCount() };
        mesh.vertices.resize(polygon_count * 3LL);
        mesh.indices.resize(polygon_count * 3LL);

        FbxStringList uv_names;
        fbx_mesh->GetUVSetNames(uv_names);
        const FbxVector4* control_points{ fbx_mesh->GetControlPoints() };
        for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
        {
            const int material_index{ material_count > 0 ?
                fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(polygon_index) : 0 };
            mesh::subset& subset{ subsets.at(material_index) };
            const uint32_t offset{ subset.start_index_location + subset.index_count };

            for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon)
            {
                const int vertex_index{ polygon_index * 3 + position_in_polygon };

                vertex vertex;
                const int polygon_vertex{ fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon) };
                vertex.position.x = static_cast<float>(control_points[polygon_vertex][0]);
                vertex.position.y = static_cast<float>(control_points[polygon_vertex][1]);
                vertex.position.z = static_cast<float>(control_points[polygon_vertex][2]);

                const bone_influences_per_control_point& influences_per_control_point{ bone_influences.at(polygon_vertex) };
                // ウェイトの合計を出す
                float bone_weight_total = 0;
                for (size_t influence_index = 0; influence_index < influences_per_control_point.size(); ++influence_index)
                {
                    bone_weight_total += influences_per_control_point.at(influence_index).bone_weight;
                }
                for (size_t influence_index = 0; influence_index < influences_per_control_point.size(); ++influence_index)
                {
#if 0
                    // ※影響を受けるボーンは最大４つまでとする
                    // ※下記コードは影響を受けるボーン数が４つを超える場合は、アサートを出している
                    assert(influence_index < MAX_BONE_INFLUENCES && "!ボーンの影響数が4を超えています!");
                    {
                        vertex.bone_weights[influence_index] = influences_per_control_point.at(influence_index).bone_weight / bone_weight_total; // 正規化
                        vertex.bone_indices[influence_index] = influences_per_control_point.at(influence_index).bone_index;
                    }

#endif // 0
                    if (influence_index < MAX_BONE_INFLUENCES)
                    {
                        vertex.bone_weights[influence_index] = influences_per_control_point.at(influence_index).bone_weight / bone_weight_total;
                        vertex.bone_indices[influence_index] = influences_per_control_point.at(influence_index).bone_index;
                    }
#if 1
                    else
                    {
                        size_t minimum_value_index = 0;
                        float minimum_value = FLT_MAX;
                        for (size_t i = 0; i < MAX_BONE_INFLUENCES; ++i)
                        {
                            if (minimum_value > vertex.bone_weights[i])
                            {
                                minimum_value = vertex.bone_weights[i];
                                minimum_value_index = i;
                            }
                        }
                        vertex.bone_weights[minimum_value_index] += influences_per_control_point.at(influence_index).bone_weight / bone_weight_total;
                        vertex.bone_indices[minimum_value_index] = influences_per_control_point.at(influence_index).bone_index;
                    }
#endif
                }
                // normal
                //if (fbx_mesh->GenerateNormals(false))
                if (fbx_mesh->GetElementNormalCount() > 0)
                {
                    FbxVector4 normal;
                    fbx_mesh->GetPolygonVertexNormal(polygon_index, position_in_polygon, normal);
                    vertex.normal.x = static_cast<float>(normal[0]);
                    vertex.normal.y = static_cast<float>(normal[1]);
                    vertex.normal.z = static_cast<float>(normal[2]);
                }
                // UV
                if (fbx_mesh->GetElementUVCount() > 0)
                {
                    FbxVector2 uv;
                    bool unmapped_uv;
                    fbx_mesh->GetPolygonVertexUV(polygon_index, position_in_polygon,
                        uv_names[0], uv, unmapped_uv);
                    vertex.texcoord.x = static_cast<float>(uv[0]);
                    vertex.texcoord.y = 1.0f - static_cast<float>(uv[1]);
                }
                // tangent
                if (fbx_mesh->GetElementTangentCount() > 0)
                {
                    const FbxGeometryElementTangent* tangent = fbx_mesh->GetElementTangent(0);
                    _ASSERT_EXPR(tangent->GetMappingMode() == FbxGeometryElement::EMappingMode::eByPolygonVertex &&
                        tangent->GetReferenceMode() == FbxGeometryElement::EReferenceMode::eDirect,
                        L"Only supports a combination of these modes.");

                    vertex.tangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[0]);
                    vertex.tangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[1]);
                    vertex.tangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[2]);
                    vertex.tangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[3]);
                }
                else // 法線ベクトル情報を持たない場合
                {
                    // tangentを生成
                    if (fbx_mesh->GenerateTangentsData(0, false))
                    {
                        vertex.tangent.x = fbx_mesh->GenerateTangentsData(0, false);
                        vertex.tangent.y = fbx_mesh->GenerateTangentsData(0, false);
                        vertex.tangent.z = fbx_mesh->GenerateTangentsData(0, false);
                        vertex.tangent.w = fbx_mesh->GenerateTangentsData(0, false);
                    }
                    else
                    {
                        vertex.tangent.x = 1;
                        vertex.tangent.y = 0;
                        vertex.tangent.z = 0;
                        vertex.tangent.w = 1;
                    }
                }
                mesh.vertices.at(vertex_index) = std::move(vertex);
                mesh.indices.at(static_cast<size_t>(offset) + position_in_polygon) = vertex_index;
                subset.index_count++;
            }
        }
    }
}

FbxNode* SkinnedMesh::find_fbxNode_by_uniqueId(FbxNode* fbx_node, uint64_t id)
{
    if (fbx_node->GetUniqueID() == id)
    {
        return fbx_node;
    }

    for (int child_index = 0; child_index < fbx_node->GetChildCount(); child_index++)
    {
        FbxNode* result = find_fbxNode_by_uniqueId(fbx_node->GetChild(child_index), id);
        if (result != nullptr)
        {
            return result;
        }
    }
    return nullptr;
}

FbxNode* SkinnedMesh::find_fbxNode_by_uniqueId(FbxScene* fbx_scene, uint64_t id)
{
    return find_fbxNode_by_uniqueId(fbx_scene->GetRootNode(), id);
}

void SkinnedMesh::fetch_materials(const char* fbx_filename, FbxScene* fbx_scene, std::unordered_map<uint64_t, material>& materials)
{
    const size_t node_count{ scene_view.nodes.size() };
    for (size_t node_index = 0; node_index < node_count; ++node_index)
    {
        const scene::node& node{ scene_view.nodes.at(node_index) };
        const FbxNode* fbx_node{ find_fbxNode_by_uniqueId(fbx_scene, node.unique_id) };
        // const FbxNode* fbx_node{ fbx_scene->FindNodeByName(node.name.c_str()) };

        const int material_count{ fbx_node->GetMaterialCount() };
        for (int material_index = 0; material_index < material_count; ++material_index)
        {
            const FbxSurfaceMaterial* fbx_material{ fbx_node->GetMaterial(material_index) };

            material material;
            material.name = fbx_material->GetName();
            material.unique_id = fbx_material->GetUniqueID();
            {
                FbxProperty fbx_property;
                fbx_property = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuse);
                // ディフューズ(マテリアルの本体色)
                if (fbx_property.IsValid())
                {
                    const FbxDouble3 color{ fbx_property.Get<FbxDouble3>() };
                    material.Kd.x = static_cast<float>(color[0]);
                    material.Kd.y = static_cast<float>(color[1]);
                    material.Kd.z = static_cast<float>(color[2]);
                    material.Kd.w = 1.0f;

                    const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
                    material.texture_filenames[0] = fbx_texture ? fbx_texture->GetRelativeFileName() : "";
                }
                // アンビエント(マテリアルの影)
                fbx_property = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbient);
                if (fbx_property.IsValid())
                {
                    const FbxDouble3 color{ fbx_property.Get<FbxDouble3>() };
                    material.Ka.x = static_cast<float>(color[0]);
                    material.Ka.y = static_cast<float>(color[1]);
                    material.Ka.z = static_cast<float>(color[2]);
                    material.Ka.w = 1.0f;
                }
                // スペキュラ(マテリアルのハイライト色)
                fbx_property = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecular);
                if (fbx_property.IsValid())
                {
                    const FbxDouble3 color{ fbx_property.Get<FbxDouble3>() };
                    material.Ks.x = static_cast<float>(color[0]);
                    material.Ks.y = static_cast<float>(color[1]);
                    material.Ks.z = static_cast<float>(color[2]);
                    material.Ks.w = 1.0f;
                }
                // 法線マップのファイル名
                fbx_property = fbx_material->FindProperty(FbxSurfaceMaterial::sNormalMap);
                if (fbx_property.IsValid())
                {
                    std::string color_map_filename = material.texture_filenames[0];
                    std::string normal_map_filename = "";
                    if (color_map_filename.find("Color.png") != std::string::npos)
                    {
                        normal_map_filename = color_map_filename.erase(color_map_filename.find("Color.png")) + "Normal.png";
                    }
                    std::filesystem::path fbm_filename(fbx_filename);
                    fbm_filename.remove_filename();
                    std::string s = fbm_filename.string() + normal_map_filename;
                    if (std::filesystem::exists(s.c_str()))
                    {
                        material.texture_filenames[1] = normal_map_filename;
                    }
                    else
                    {
                        const FbxFileTexture* file_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
                        material.texture_filenames[1] = file_texture ? file_texture->GetRelativeFileName() : "";
                    }
                }
                // metal map
                {
                    std::string color_map_filename = material.texture_filenames[0];
                    std::string metallic_map_filename = "";
                    if (color_map_filename.find("Color.png") != std::string::npos)
                    {
                        metallic_map_filename = color_map_filename.erase(color_map_filename.find("Color.png")) + "Metalness.png";
                    }
                    std::filesystem::path fbm_filename(fbx_filename);
                    fbm_filename.remove_filename();
                    std::string s = fbm_filename.string() + metallic_map_filename;
                    if (std::filesystem::exists(s.c_str()))
                    {
                        material.texture_filenames[2] = metallic_map_filename;
                    }
                    else
                    {
                        material.texture_filenames[2] = "";
                    }
                }
                // Roughness map
                {
                    std::string color_map_filename = material.texture_filenames[0];
                    std::string roughness_map_filename = "";
                    if (color_map_filename.find("Color.png") != std::string::npos)
                    {
                        roughness_map_filename = color_map_filename.erase(color_map_filename.find("Color.png")) + "Roughness.png";
                    }
                    std::filesystem::path fbm_filename(fbx_filename);
                    fbm_filename.remove_filename();
                    std::string s = fbm_filename.string() + roughness_map_filename;
                    if (std::filesystem::exists(s.c_str()))
                    {
                        material.texture_filenames[3] = roughness_map_filename;
                    }
                    else
                    {
                        material.texture_filenames[3] = "";
                    }
                }
                // AO map
                {
                    std::string color_map_filename = material.texture_filenames[0];
                    std::string ao_map_filename = "";
                    if (color_map_filename.find("Color.png") != std::string::npos)
                    {
                        ao_map_filename = color_map_filename.erase(color_map_filename.find("Color.png")) + "Ao.png";
                    }
                    std::filesystem::path fbm_filename(fbx_filename);
                    fbm_filename.remove_filename();
                    std::string s = fbm_filename.string() + ao_map_filename;
                    if (std::filesystem::exists(s.c_str()))
                    {
                        material.texture_filenames[4] = ao_map_filename;
                    }
                    else
                    {
                        material.texture_filenames[4] = "";
                    }
                }
                // Emissive map
                {
                    std::string color_map_filename = material.texture_filenames[0];
                    std::string emissive_map_filename = "";
                    if (color_map_filename.find("Color.png") != std::string::npos)
                    {
                        emissive_map_filename = color_map_filename.erase(color_map_filename.find("Color.png")) + "Emissive.png";
                    }
                    std::filesystem::path fbm_filename(fbx_filename);
                    fbm_filename.remove_filename();
                    std::string s = fbm_filename.string() + emissive_map_filename;
                    if (std::filesystem::exists(s.c_str()))
                    {
                        material.texture_filenames[5] = emissive_map_filename;
                    }
                    else
                    {
                        material.texture_filenames[5] = "";
                    }
                }
                // Dissolve map
                {
                    std::string color_map_filename = material.texture_filenames[0];
                    std::string dissolve_map_filename = "";
                    if (color_map_filename.find("Color.png") != std::string::npos)
                    {
                        dissolve_map_filename = color_map_filename.erase(color_map_filename.find("Color.png")) + "Dissolve.png";
                    }
                    std::filesystem::path fbm_filename(fbx_filename);
                    fbm_filename.remove_filename();
                    std::string s = fbm_filename.string() + dissolve_map_filename;
                    if (std::filesystem::exists(s.c_str()))
                    {
                        material.texture_filenames[6] = dissolve_map_filename;
                    }
                    else
                    {
                        material.texture_filenames[6] = "";
                    }
                }
                // Glow map
                {
                    std::string color_map_filename = material.texture_filenames[0];
                    std::string glow_map_filename = "";
                    if (color_map_filename.find("Color.png") != std::string::npos)
                    {
                        glow_map_filename = color_map_filename.erase(color_map_filename.find("Color.png")) + "Glow.png";
                    }
                    std::filesystem::path fbm_filename(fbx_filename);
                    fbm_filename.remove_filename();
                    std::string s = fbm_filename.string() + glow_map_filename;
                    if (std::filesystem::exists(s.c_str()))
                    {
                        material.texture_filenames[7] = glow_map_filename;
                    }
                    else
                    {
                        material.texture_filenames[7] = "";
                    }
                }

                materials.emplace(material.unique_id, std::move(material));
            }
        }
    }
    // マテリアル情報(materials)のサイズがゼロになるので描画が出来ない
    // ダミーマテリアルの作成
    materials.emplace();
}

void SkinnedMesh::fetch_skeleton(FbxMesh* fbx_mesh, skeleton& bind_pose)
{
    const int deformer_count = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int deformer_index = 0; deformer_index < deformer_count; ++deformer_index)
    {
        FbxSkin* skin = static_cast<FbxSkin*>(fbx_mesh->GetDeformer(deformer_index, FbxDeformer::eSkin));
        const int cluster_count = skin->GetClusterCount();
        bind_pose.bones.resize(cluster_count);
        for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
        {
            FbxCluster* cluster = skin->GetCluster(cluster_index);

            skeleton::bone& bone{ bind_pose.bones.at(cluster_index) };
            bone.name = cluster->GetLink()->GetName();
            bone.unique_id = cluster->GetLink()->GetUniqueID();
            bone.parent_index = bind_pose.indexof(cluster->GetLink()->GetParent()->GetUniqueID());
            bone.node_index = scene_view.indexof(bone.unique_id);
            //'reference_global_init_position' is used to convert from local space of model(mesh) to
            // global space of scene.
            FbxAMatrix reference_global_init_position;
            cluster->GetTransformMatrix(reference_global_init_position);
            // 'cluster_global_init_position' is used to convert from local space of bone to
            // global space of scene.
            FbxAMatrix cluster_global_init_position;
            cluster->GetTransformLinkMatrix(cluster_global_init_position);
            // Matrices are defined using the Column Major scheme. When a FbxAMatrix represents a transformation
            // (translation, rotation and scale), the last row of the matrix represents the translation part of
            // the transformation.
            // Compose 'bone.offset_transform' matrix that trnasforms position from mesh space to bone space.
            // This matrix is called the offset matrix.
            bone.offset_transform
                = to_xmfloat4x4(cluster_global_init_position.Inverse() * reference_global_init_position);
        }
    }
}

void SkinnedMesh::fetch_animations(FbxScene* fbx_scene,
    std::vector<animation>& animation_clips, float sampling_rate)
{
    FbxArray<FbxString*> animation_stack_names;
    fbx_scene->FillAnimStackNameArray(animation_stack_names);
    const int animation_stack_count{ animation_stack_names.GetCount() };
    for (int animation_stack_index = 0; animation_stack_index < animation_stack_count; ++animation_stack_index)
    {
        animation& animation_clip{ animation_clips.emplace_back() };
        animation_clip.name = animation_stack_names[animation_stack_index]->Buffer();

        FbxAnimStack* animation_stack{ fbx_scene->FindMember<FbxAnimStack>(animation_clip.name.c_str()) };
        fbx_scene->SetCurrentAnimationStack(animation_stack);

        const FbxTime::EMode time_mode{ fbx_scene->GetGlobalSettings().GetTimeMode() };
        FbxTime one_second;
        one_second.SetTime(0, 0, 1, 0, 0, time_mode);
        animation_clip.sampling_rate = sampling_rate > 0 ?
            sampling_rate : static_cast<float>(one_second.GetFrameRate(time_mode));
        const FbxTime sampling_interval
        { static_cast<FbxLongLong>(one_second.Get() / animation_clip.sampling_rate) };
        const FbxTakeInfo* take_info{ fbx_scene->GetTakeInfo(animation_clip.name.c_str()) };
        const FbxTime start_time{ take_info->mLocalTimeSpan.GetStart() };
        const FbxTime stop_time{ take_info->mLocalTimeSpan.GetStop() };
        for (FbxTime time = start_time; time < stop_time; time += sampling_interval)
        {
            animation::keyframe& keyframe{ animation_clip.sequence.emplace_back() };

            const size_t node_count{ scene_view.nodes.size() };
            keyframe.nodes.resize(node_count);
            for (size_t node_index = 0; node_index < node_count; ++node_index)
            {
                FbxNode* fbx_node{ find_fbxNode_by_uniqueId(fbx_scene, scene_view.nodes.at(node_index).unique_id) };
                //FbxNode* fbx_node{ fbx_scene->FindNodeByName(scene_view.nodes.at(node_index).name.c_str()) };
                if (fbx_node)
                {
                    animation::keyframe::node& node{ keyframe.nodes.at(node_index) };
                    // 'global_transform' is a transformation matrix of a node with respect to
                    // the scene's global coordinate system.
                    node.global_transform = to_xmfloat4x4(fbx_node->EvaluateGlobalTransform(time));
                    // 'local_transform' is a transformation matrix of a node with respect to
                    // its parent's local coordinate system.
                    const FbxAMatrix& local_transform{ fbx_node->EvaluateLocalTransform(time) };
                    node.scaling = to_xmfloat3(local_transform.GetS());
                    node.rotation = to_xmfloat4(local_transform.GetQ());
                    node.translation = to_xmfloat3(local_transform.GetT());
                }
            }
        }
    }
    for (int animation_stack_index = 0; animation_stack_index < animation_stack_count; ++animation_stack_index)
    {
        delete animation_stack_names[animation_stack_index];
    }
}

bool SkinnedMesh::append_animations(const char* animation_filename, float sampling_rate)
{
    FbxManager* fbx_manager{ FbxManager::Create() };
    FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

    FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
    bool import_status{ false };
    import_status = fbx_importer->Initialize(animation_filename);
    _ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());
    import_status = fbx_importer->Import(fbx_scene);
    _ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());

    fetch_animations(fbx_scene, animation_clips, sampling_rate);

    fbx_manager->Destroy();

    return true;
}

void SkinnedMesh::blend_animations(const animation::keyframe* keyframes[2], float factor, animation::keyframe& keyframe)
{
    using namespace DirectX;
    size_t node_count{ keyframes[0]->nodes.size() };
    keyframe.nodes.resize(node_count);
    for (size_t node_index = 0; node_index < node_count; ++node_index)
    {
        XMVECTOR S[2]{
            XMLoadFloat3(&keyframes[0]->nodes.at(node_index).scaling),
            XMLoadFloat3(&keyframes[1]->nodes.at(node_index).scaling) };
        XMStoreFloat3(&keyframe.nodes.at(node_index).scaling, XMVectorLerp(S[0], S[1], factor));

        XMVECTOR R[2]{
            XMLoadFloat4(&keyframes[0]->nodes.at(node_index).rotation),
            XMLoadFloat4(&keyframes[1]->nodes.at(node_index).rotation) };
        XMStoreFloat4(&keyframe.nodes.at(node_index).rotation, XMQuaternionSlerp(R[0], R[1], factor));

        XMVECTOR T[2]{ XMLoadFloat3(&keyframes[0]->nodes.at(node_index).translation),
            XMLoadFloat3(&keyframes[1]->nodes.at(node_index).translation) };
        XMStoreFloat3(&keyframe.nodes.at(node_index).translation, XMVectorLerp(T[0], T[1], factor));
    }
}

void SkinnedMesh::create_com_objects(ID3D11Device* device, const char* fbx_filename)
{
    for (mesh& mesh : meshes)
    {
        HRESULT hr{ S_OK };
        D3D11_BUFFER_DESC buffer_desc{};
        D3D11_SUBRESOURCE_DATA subresource_data{};
        buffer_desc.ByteWidth = static_cast<UINT>(sizeof(vertex) * mesh.vertices.size());
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        buffer_desc.CPUAccessFlags = 0;
        buffer_desc.MiscFlags = 0;
        buffer_desc.StructureByteStride = 0;
        subresource_data.pSysMem = mesh.vertices.data();
        subresource_data.SysMemPitch = 0;
        subresource_data.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&buffer_desc, &subresource_data,
            mesh.vertex_buffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        subresource_data.pSysMem = mesh.indices.data();
        hr = device->CreateBuffer(&buffer_desc, &subresource_data,
            mesh.index_buffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#if 0
        mesh.vertices.clear();
        mesh.indices.clear();
#endif
    }
    // シェーダーリソースビューオブジェクト生成
    {
        for (std::unordered_map<uint64_t, material>::iterator iterator = materials.begin();
            iterator != materials.end(); ++iterator)
        {
            for (size_t texture_index = 0; texture_index < material::TEXTURE_COUNT; ++texture_index)
            {
                if (iterator->second.texture_filenames[texture_index].size() > 0)
                {
                    std::filesystem::path path(fbx_filename);
                    path.replace_filename(iterator->second.texture_filenames[texture_index]);
                    D3D11_TEXTURE2D_DESC texture2d_desc;
                    load_texture_from_file(device, path.c_str(),
                        iterator->second.shader_resource_views[texture_index].GetAddressOf(), &texture2d_desc);
                }
                else
                {
                    DWORD tex_color = 0xFFFFFFFF;
                    if (texture_index == 1) { tex_color = 0xFFFF7F7F; }
                    if (texture_index == 5) { tex_color = 0x0000; }
                    if (texture_index == 7) { tex_color = 0x0000; }
                    make_dummy_texture(device, iterator->second.shader_resource_views[texture_index].GetAddressOf(), tex_color, 16);
                }
            }
        }
    }
    HRESULT hr = S_OK;
    // 定数バッファの作成
    geometry_constants = std::make_unique<Constants<GeometryConstants>>(device);
}

void SkinnedMesh::update_blend_animation(animation::keyframe& keyframe)
{
    using namespace DirectX;

    size_t node_count{ keyframe.nodes.size() };
    for (size_t node_index = 0; node_index < node_count; ++node_index)
    {
        animation::keyframe::node& node{ keyframe.nodes.at(node_index) };
        XMMATRIX S{ XMMatrixScaling(node.scaling.x, node.scaling.y, node.scaling.z) };
        XMMATRIX R{ XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation)) };
        XMMATRIX T{ XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z) };

        int64_t parent_index{ scene_view.nodes.at(node_index).parent_index };
        XMMATRIX P{ parent_index < 0 ? XMMatrixIdentity() :
            XMLoadFloat4x4(&keyframe.nodes.at(parent_index).global_transform) };

        XMStoreFloat4x4(&node.global_transform, S * R * T * P);
    }
}

void SkinnedMesh::play_animation(int animation_index, bool is_loop, bool interpolation, float blend_seconds, float playback_speed)
{
    assert("!!アニメーションが無いのにplay_animation関数を呼び出しています!!"
        && animation_clips.size() > 0);
    assert("!!存在しないインデックスのアニメーションを再生しようとしています!!"
        && animation_index >= 0 && animation_index <= animation_clips.size() - 1);
    // アニメーションのパラメーターリセット
    anim_para.animation_blend_seconds = blend_seconds;
    anim_para.animation_blend_time    = 0.0f;
    anim_para.current_anim_index      = animation_index;
    anim_para.frame_index             = 0;
    anim_para.animation_tick          = 0;
    anim_para.do_loop                 = is_loop;
    anim_para.stop_animation          = false;
    anim_para.end_of_animation        = false;
    anim_para.interpolation           = interpolation;
    anim_para.playback_speed          = playback_speed;

    anim_para.old_keyframe = anim_para.current_keyframe;
}

void SkinnedMesh::play_animation(anim_Parameters& para, int animation_index, bool is_loop, bool interpolation, float blend_seconds, float playback_speed)
{
    assert("!!アニメーションが無いのにplay_animation関数を呼び出しています!!"
        && animation_clips.size() > 0);
    assert("!!存在しないインデックスのアニメーションを再生しようとしています!!"
        && animation_index >= 0 && animation_index <= animation_clips.size() - 1);
    // アニメーションのパラメーターリセット
    para.animation_blend_seconds = blend_seconds;
    para.animation_blend_time    = 0.0f;
    para.current_anim_index      = animation_index;
    para.frame_index             = 0;
    para.animation_tick          = 0;
    para.do_loop                 = is_loop;
    para.stop_animation          = false;
    para.end_of_animation        = false;
    para.interpolation           = interpolation;
    para.playback_speed          = playback_speed;

    para.old_keyframe = para.current_keyframe;
}

void SkinnedMesh::update_animation(float elapsed_time)
{
    // アサート
    if (end_of_animation()) return;
    assert("!!アニメーションが無いのにupdate_animation関数を呼び出しています!!"
        && animation_clips.size() > 0);

    // アニメーション＆アニメーションのフレーム算出
    anim_para.animation   = animation_clips.at(anim_para.current_anim_index);
    anim_para.frame_index = static_cast<int>(anim_para.animation_tick * anim_para.animation.sampling_rate * anim_para.playback_speed);
    // アニメーション間の補完割合算出
    float blendRate = 1.0f;
    if (anim_para.interpolation && anim_para.animation_blend_time < anim_para.animation_blend_seconds)
    {
        anim_para.animation_blend_time += elapsed_time;
        if (anim_para.animation_blend_time >= anim_para.animation_blend_seconds) { anim_para.animation_blend_time = anim_para.animation_blend_seconds; }
        blendRate = anim_para.animation_blend_time / anim_para.animation_blend_seconds;
        blendRate *= blendRate;
    }

    // アニメーション間の補完
    if (blendRate < 1.0f)
    {
        const animation::keyframe* keyframes[2] {
          &anim_para.old_keyframe,
          &animation_clips.at(anim_para.current_anim_index).sequence.at(0)
        };
        blend_animations(keyframes, blendRate, anim_para.current_keyframe);
        update_blend_animation(anim_para.current_keyframe);
    }
    // 通常時
    else
    {
        // 再生が1ループ終了
        if (anim_para.frame_index > anim_para.animation.sequence.size() - 1)
        {
            // ループ再生
            if (anim_para.do_loop)
            {
                anim_para.frame_index = 0;
                anim_para.animation_tick = 0;
            }
            // 非ループ再生
            else
            {
                anim_para.end_of_animation = true;
                anim_para.frame_index = static_cast<int>(anim_para.animation.sequence.size()) - 1;
            }
        }
        // 再生中
        else
        {
            if (!anim_para.stop_animation) {
                anim_para.animation_tick += elapsed_time;
            }
        }

        anim_para.current_keyframe = anim_para.animation.sequence.at(anim_para.frame_index);
    }
}

void SkinnedMesh::update_animation(anim_Parameters& para, float elapsed_time)
{
    // アサート
    if (end_of_animation()) return;
    assert("!!アニメーションが無いのにupdate_animation関数を呼び出しています!!"
        && animation_clips.size() > 0);

    // アニメーション＆アニメーションのフレーム算出
    para.animation = animation_clips.at(para.current_anim_index);
    para.frame_index = static_cast<int>(para.animation_tick * para.animation.sampling_rate * para.playback_speed);
    // アニメーション間の補完割合算出
    float blendRate = 1.0f;
    if (para.interpolation && para.animation_blend_time < para.animation_blend_seconds)
    {
        para.animation_blend_time += elapsed_time;
        if (para.animation_blend_time >= para.animation_blend_seconds) { para.animation_blend_time = para.animation_blend_seconds; }
        blendRate = para.animation_blend_time / para.animation_blend_seconds;
        blendRate *= blendRate;
    }

    // アニメーション間の補完
    if (blendRate < 1.0f)
    {
        const animation::keyframe* keyframes[2]{
          &para.old_keyframe,
          &animation_clips.at(para.current_anim_index).sequence.at(0)
        };
        blend_animations(keyframes, blendRate, para.current_keyframe);
        update_blend_animation(para.current_keyframe);
    }
    // 通常時
    else
    {
        // 再生が1ループ終了
        if (para.frame_index > para.animation.sequence.size() - 1)
        {
            // ループ再生
            if (para.do_loop)
            {
                para.frame_index = 0;
                para.animation_tick = 0;
            }
            // 非ループ再生
            else
            {
                para.end_of_animation = true;
                para.frame_index = static_cast<int>(para.animation.sequence.size()) - 1;
            }
        }
        // 再生中
        else
        {
            if (!para.stop_animation) {
                para.animation_tick += elapsed_time;
            }
        }

        para.current_keyframe = para.animation.sequence.at(para.frame_index);
    }
}
