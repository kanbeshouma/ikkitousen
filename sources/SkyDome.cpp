#include "SkyDome.h"
#include"imgui_include.h"

float SkyDome::purple_threshold = 0.0f;
float SkyDome::red_threshold = 0.0f;

SkyDome::SkyDome(GraphicsPipeline& graphics)
{
    std::string sub_colors[2] =
    {
        "sky.fbm\\sky_purple.png",
        "sky.fbm\\sky_red.png",
    };

    model = std::make_shared<SkinnedMesh>(graphics.get_device().Get(), ".\\resources\\Models\\stage\\sky.fbx", sub_colors, true, 0.0f);

    field = resource_manager->load_model_resource(graphics.get_device().Get(), ".\\resources\\Models\\stage\\field.fbx", true);

    // constants
    constants = std::make_unique<Constants<FieldConstants>>(graphics.get_device().Get());
}

SkyDome::~SkyDome()
{
}

void SkyDome::Render(GraphicsPipeline& graphics, float elapsed_time)
{

#ifdef USE_IMGUI
    ImGui::Begin("SkyDome");
    ImGui::DragFloat4("emissive_color", &emissive_color.x);
    ImGui::DragFloat3("angle", &angle.x);
    ImGui::DragFloat("purple_threshold", &purple_threshold, 0.01f);
    ImGui::DragFloat("red_threshold", &red_threshold, 0.01f);
    ImGui::End();
#endif // USE_IMGUI

    graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::PBR);

    std::string sub_color_mesh_name = "pSphere1";
    model->render(graphics.get_dc().Get(), Math::calc_world_matrix(scale, angle, position),
        { 1,1,1,1 }, 0, 0, emissive_color, 0.8f, purple_threshold, red_threshold, sub_color_mesh_name);

    static float scale_factor = 0.88f;
    static float glow_speed = 1.0f;
    static DirectX::XMFLOAT4 emissive_color = { 1,1,1,5.0f };
    static float glow_thickness = 1.5f;
#ifdef USE_IMGUI
    ImGui::Begin("SkyDome");
    if (ImGui::TreeNode("field"))
    {
        ImGui::DragFloat("scale_factor", &scale_factor, 0.01f);
        ImGui::DragFloat("glow_speed", &glow_speed, 0.01f);
        ImGui::ColorEdit3("emissive_color", &emissive_color.x);
        ImGui::DragFloat("emissive_strength", &emissive_color.w, 0.01f);
        ImGui::DragFloat("glow_thickness", &glow_thickness, 0.01f);
        ImGui::DragFloat2("field_resolution", &constants->data.field_resolution.x, 0.1f);
        ImGui::Text("field_time:%.2f", constants->data.field_time);

        ImGui::TreePop();
    }
    ImGui::End();
#endif // USE_IMGUI

    field_glow_time += elapsed_time * glow_speed;

#if 0
    constants->data.field_time += elapsed_time;
    constants->bind(graphics.get_dc().Get(), 10);
    graphics.set_pipeline_preset(SHADER_TYPES::FIELD);
#endif // 0

    DirectX::XMFLOAT3 field_scale = { scale.x * scale_factor, scale.y * scale_factor, scale.z * scale_factor };
    field->render(graphics.get_dc().Get(), Math::calc_world_matrix(field_scale, angle, position),
        { 1,1,1,1 }, 0, field_glow_time, emissive_color, glow_thickness);
}

bool SkyDome::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
    return Collision::ray_vs_model(start, end, model.get(), position, scale, angle, hit);
}