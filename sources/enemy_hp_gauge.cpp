#include "enemy_hp_gauge.h"
#include "Operators.h"
#include "codinate_convert.h"

EnemyHpGauge::EnemyHpGauge(GraphicsPipeline& graphics)
{
    frame = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\enemy_hp_flame.png", 1);
    back  = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\enemy_hp_back.png", 1);
    body  = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\enemy_hp_gauge.png", 1);

    gauge.scale = { 0.2f, 0.2f };
    gauge.texsize = { static_cast<float>(frame->get_texture2d_desc().Width), static_cast<float>(frame->get_texture2d_desc().Height) };
    gauge.pivot = gauge.texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
    gauge.color.w = 0.0f;
}

void EnemyHpGauge::update(GraphicsPipeline& graphics, float elapsed_time)
{
    static bool is_display_imgui = false;
    imgui_menu_bar("UI", "enemygauge", is_display_imgui);
#ifdef USE_IMGUI
    if (is_display_imgui)
    {
        ImGui::Begin("enemy gauge");
        ImGui::DragFloat2("pos", &gauge.position.x);
        ImGui::DragFloat2("scale", &gauge.scale.x, 0.01f);
        ImGui::DragFloat2("offset", &offset.x, 0.1f);
        ImGui::DragFloat("hp_percent", &hp_percent, 0.1f, 0.0f, 1.0f);
        ImGui::Text("length:%f", length_player_to_enemy);
        ImGui::End();
    }
#endif // USE_IMGUI

    if (animation)
    {
        gauge.position = conversion_2D(graphics.get_dc().Get(), focus_position);
        gauge.color.w = Math::lerp(gauge.color.w, 1.0f, 10.0f * elapsed_time);

        float scale_rate = BASE_RATE_LENGTH / length_player_to_enemy;
        scale_rate = Math::clamp(scale_rate, 0.5f, 1.0f);
        // offset
        DirectX::XMFLOAT2 arrival_offset = { 0.0f * scale_rate, -150.0f * scale_rate };
        offset = Math::lerp(offset, arrival_offset, 10.0f * elapsed_time);
        // scale
        DirectX::XMFLOAT2 arrival_scale = { 0.2f * scale_rate, 0.2f * scale_rate };
        gauge.scale = Math::lerp(gauge.scale, arrival_scale, 10.0f * elapsed_time);
    }
    else
    {
        gauge.color.w = Math::lerp(gauge.color.w, 0.0f, 10.0f * elapsed_time);
    }
}

void EnemyHpGauge::render(ID3D11DeviceContext* dc)
{
    //--back--//
    back->begin(dc);
    back->render(dc, gauge.position + offset, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos, gauge.texsize);
    back->end(dc);
    //--body--//
    body->begin(dc);
    body->render(dc, gauge.position + offset, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos,
        { gauge.texsize.x * hp_percent, gauge.texsize.y });
    body->end(dc);
    //--frame--//
    frame->begin(dc);
    frame->render(dc, gauge.position + offset, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos, gauge.texsize);
    frame->end(dc);
}

void EnemyHpGauge::focus(const BaseEnemy* target_enemy, bool lockon)
{
    if (target_enemy != nullptr)
    {
        animation              = lockon;
        focus_position         = target_enemy->fGetPosition();
        hp_percent             = target_enemy->fGetPercentHitPoint();
        length_player_to_enemy = target_enemy->fGetLengthFromPlayer();
    }
    else
    {
        animation = false;
        hp_percent = 1.0f;
    }
}


float BossHpGauge::hp_percent = 1.0f;

BossHpGauge::BossHpGauge(GraphicsPipeline& graphics)
{
    frame = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\boss_hp_flame.png", 1);
    back = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\boss_hp_back.png", 1);
    body = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\ui\\boss_hp_gauge.png", 1);

    gauge.position = { 640,40 };
    gauge.scale = { 0.7f, 0.4f };
    gauge.texsize = { static_cast<float>(frame->get_texture2d_desc().Width), static_cast<float>(frame->get_texture2d_desc().Height) };
    gauge.pivot = gauge.texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
    gauge.color.w = 0.0f;
}

void BossHpGauge::update(GraphicsPipeline& graphics, float elapsed_time)
{
    static bool is_display_imgui = false;
    imgui_menu_bar("UI", "boss gauge", is_display_imgui);
#ifdef USE_IMGUI
    if (is_display_imgui)
    {
        ImGui::Begin("boss gauge");
        ImGui::DragFloat2("pos", &gauge.position.x);
        ImGui::DragFloat2("scale", &gauge.scale.x, 0.01f);
        ImGui::DragFloat("hp_percent", &hp_percent, 0.1f, 0.0f, 1.0f);
        ImGui::End();
    }
#endif // USE_IMGUI

    if (animation) { gauge.color.w = Math::lerp(gauge.color.w, 1.0f, 10.0f * elapsed_time); }
    else { gauge.color.w = Math::lerp(gauge.color.w, 0.0f, 10.0f * elapsed_time); }
}

void BossHpGauge::render(ID3D11DeviceContext* dc)
{
    //--back--//
    back->begin(dc);
    back->render(dc, gauge.position, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos, gauge.texsize);
    back->end(dc);
    //--body--//
    body->begin(dc);
    body->render(dc, gauge.position, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos,
        { gauge.texsize.x * hp_percent, gauge.texsize.y });
    body->end(dc);
    //--frame--//
    frame->begin(dc);
    frame->render(dc, gauge.position, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos, gauge.texsize);
    frame->end(dc);
}
