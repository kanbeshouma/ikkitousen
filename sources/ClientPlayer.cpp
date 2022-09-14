#include "ClientPlayer.h"

ClientPlayer::ClientPlayer(GraphicsPipeline& graphics, int object_id)
    :BasePlayer()
{
    model = resource_manager->load_model_resource(graphics.get_device().Get(), ".\\resources\\Models\\Player\\player_twentyfource.fbx", false, 60.0f);

    TransitionIdle();
    scale = { 0.06f,0.06f,0.06f };
    GetPlayerDirections();
    mSwordTrail[0].fInitialize(graphics.get_device().Get(),
        L"./resources/TexMaps/SwordTrail/warp_cut.png",
        L"./resources/TexMaps/SwordTrail/SwordTrail.png");
    mSwordTrail[1].fInitialize(graphics.get_device().Get(),
        L"./resources/TexMaps/SwordTrail/warp_cut.png",
        L"./resources/TexMaps/SwordTrail/SwordTrail.png");
    player_config = std::make_unique<PlayerConfig>(graphics);
    player_condition = std::make_unique<PlayerCondition>(graphics);

    //-----オブジェクトID設定-----//
    this->object_id = object_id;

    player_bones[0] = model->get_bone_by_name("body_joint");
    player_bones[1] = model->get_bone_by_name("face_joint");
    player_bones[2] = model->get_bone_by_name("largeblade_L_joint");
    player_bones[3] = model->get_bone_by_name("largeblade_L_top_joint");
    player_bones[4] = model->get_bone_by_name("largeblade_R_joint");
    player_bones[5] = model->get_bone_by_name("largeblade_R_top_joint");
    player_bones[6] = model->get_bone_by_name("shortsword_joint");
    player_bones[7] = model->get_bone_by_name("shortsword_top_joint");
    player_bones[8] = model->get_bone_by_name("camera_joint");
    player_bones[9] = model->get_bone_by_name("camera_focus_joint");
    player_bones[10] = model->get_bone_by_name("foot_L_top_joint");
    player_bones[11] = model->get_bone_by_name("foot_R_top_joint");
}

ClientPlayer::~ClientPlayer()
{
}

void ClientPlayer::Initialize()
{
}

void ClientPlayer::Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)
{
    if (condition_state == ConditionState::Die)
    {
        if (is_update_animation)model->update_animation(elapsed_time * animation_speed);
        //ExecFuncUpdate(elapsed_time, sky_dome, enemies, graphics);
        return;
    }
    if (boss_camera)
    {
        velocity = {};
        position = { 1.0f,0.0f,-160.0f };
        is_lock_on = false;
        return;
    }

    //ExecFuncUpdate(elapsed_time, sky_dome, enemies, graphics);
    //クリア演出中
    if (during_clear)
    {
        //モデルを映す
        if (threshold_mesh > 0) threshold_mesh -= 2.0f * elapsed_time;
        GetPlayerDirections();

    }
    //クリア演出中じゃないとき
    else
    {
        PlayerJustification(elapsed_time, position);

        GetPlayerDirections();
        //プレイヤーのパラメータの変更
        InflectionParameters(elapsed_time);

        if (is_awakening)
        {
            for (int i = 0; i < 2; ++i)
            {
                mSwordTrail[i].fUpdate(elapsed_time, 10);
                mSwordTrail[i].fEraseTrailPoint(elapsed_time);
            }
        }
        else
        {
            mSwordTrail[0].fUpdate(elapsed_time, 10);
            mSwordTrail[0].fEraseTrailPoint(elapsed_time);
        }

        player_config->update(graphics, elapsed_time);
        player_condition->update(graphics, elapsed_time);

    }

    if (is_update_animation)model->update_animation(elapsed_time * animation_speed);
    threshold_mesh = Math::clamp(threshold_mesh, 0.0f, 1.0f);


#ifdef USE_IMGUI
    static bool display_scape_imgui;
    imgui_menu_bar("Player", "Player", display_scape_imgui);
    if (display_scape_imgui)
    {
        if (ImGui::Begin("Player"))
        {
            ImGui::Separator();
            std::string obj_id{ "object_id :" + std::to_string(object_id) };
            ImGui::Text(obj_id.c_str());
            if (ImGui::TreeNode("transform"))
            {
                ImGui::DragFloat3("position", &position.x);
                ImGui::DragFloat3("scale", &scale.x, 0.001f);
                ImGui::DragFloat4("orientation", &orientation.x);
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("speed"))
            {
                ImGui::DragFloat3("velocity", &velocity.x);
                ImGui::DragFloat3("acceleration_velocity", &acceleration_velocity.x);
                ImGui::DragFloat("max_speed", &move_speed);
                //ImGui::DragFloat("PLAYER_INPUT_MIN", &PLAYER_INPUT_MIN);
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("PlayerFlags"))
            {
                ImGui::Checkbox("is_avoidance", &is_avoidance);
                ImGui::Checkbox("is_behind_avoidance", &is_behind_avoidance);
                ImGui::Checkbox("is_lock_on", &is_lock_on);
                ImGui::Checkbox("is_enemy_hit", &is_enemy_hit);
                ImGui::Checkbox("is_awakening", &is_awakening);
                ImGui::Checkbox("start_dash_effect", &start_dash_effect);
                ImGui::Checkbox("end_dash_effect", &end_dash_effect);
                ImGui::Checkbox("is_push_lock_on_button", &is_push_lock_on_button);
                ImGui::Checkbox("is_charge", &is_charge);
                ImGui::Checkbox("is_attack", &is_attack);
                ImGui::Checkbox("avoidance_buttun", &avoidance_buttun);
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("CapsuleParam"))
            {
                if (ImGui::TreeNode("BodyCapsuleParam"))
                {
                    ImGui::DragFloat3("capsule_parm.start", &body_capsule_param.start.x, 0.1f);
                    ImGui::DragFloat3("capsule_parm.end", &body_capsule_param.end.x, 0.1f);
                    ImGui::DragFloat("body_capsule_param.rasius", &body_capsule_param.rasius, 0.1f);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("just_avoidance_capsule_param"))
                {
                    ImGui::DragFloat3("capsule_parm.start", &just_avoidance_capsule_param.start.x, 0.1f);
                    ImGui::DragFloat3("capsule_parm.end", &just_avoidance_capsule_param.end.x, 0.1f);
                    ImGui::DragFloat("just_avoidance_capsule_param.rasius", &just_avoidance_capsule_param.rasius, 0.1f);
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("easing"))
            {
                ImGui::DragFloat("avoidance_easing_time", &avoidance_easing_time, 0.1f);
                ImGui::DragFloat("avoidance_boost_time", &avoidance_boost_time, 0.1f);
                ImGui::DragFloat("leverage", &leverage, 0.1f);
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("PlayerGameParm"))
            {
                ImGui::DragInt("player_health", &player_health);
                ImGui::DragFloat("combo", &combo_count);
                ImGui::DragFloat("attack_time", &attack_time);

                ImGui::DragFloat("duration_combo_timer", &duration_combo_timer);
                ImGui::DragInt("player_attack_power", &player_attack_power);
                ImGui::DragFloat("invincible_timer", &invincible_timer);
                ImGui::TreePop();
            }
            ImGui::Separator();

            if (ImGui::Button("TransitionStageMove")) TransitionStageMove();
            if (ImGui::Button("TransitionIdle")) TransitionIdle();

            ImGui::DragFloat("threshold", &threshold, 0.01f, 0, 1.0f);
            ImGui::DragFloat("threshold_mesh", &threshold_mesh, 0.01f, 0, 1.0f);
            ImGui::DragFloat("glow_time", &glow_time);
            ImGui::DragFloat4("emissive_color", &emissive_color.x, 0.1f);
            DirectX::XMFLOAT3 p{ position.x,position.y + step_offset_y,position.z };
            float length_radius = Math::calc_vector_AtoB_length(p, { 0,0,0 });//距離(半径)
            ImGui::DragFloat("l", &length_radius);

            ImGui::DragFloat3("charge_point", &charge_point.x);


            ImGui::End();
        }
    }
#endif // USE_IMGUI
    debug_figure->create_capsule(just_avoidance_capsule_param.start, just_avoidance_capsule_param.end, just_avoidance_capsule_param.rasius, { 1.0f,1.0f,1.0f,1.0f });

}

void ClientPlayer::Render(GraphicsPipeline& graphics, float elapsed_time)
{
}

void ClientPlayer::StepCapsule()
{
}

void ClientPlayer::BodyCapsule()
{
}

void ClientPlayer::SwordCapsule()
{
}

void ClientPlayer::StunSphere()
{
}

void ClientPlayer::InflectionParameters(float elapesd_time)
{
}

void ClientPlayer::AddCombo(int count, bool& block)
{
}

void ClientPlayer::AwakingAddCombo(int hit_count1, int hit_count2, bool& block)
{
}

void ClientPlayer::GetPlayerDirections()
{
}
