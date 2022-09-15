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
    //ダメージを受ける関数を関数ポインタに格納
    damage_func = [=](int damage, float invincible)->void {DamagedCheck(damage, invincible); };

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

    //-----アニメーション遷移登録-----//
    RegisterAnimationFunctions();
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
        if (is_update_animation)model->update_animation(anim_parm, elapsed_time * animation_speed);
        //ExecFuncUpdate(elapsed_time, sky_dome, enemies, graphics);
        update_animation(elapsed_time, sky_dome);
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
     update_animation(elapsed_time, sky_dome);

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

    if (is_update_animation)model->update_animation(anim_parm,elapsed_time * animation_speed);
    threshold_mesh = Math::clamp(threshold_mesh, 0.0f, 1.0f);


#ifdef USE_IMGUI
    std::string obj_id{ "ClientPlayer : object_id :" + std::to_string(object_id) };
    imgui_menu_bar("Player", obj_id.c_str(), display_scape_imgui);
    if (display_scape_imgui)
    {
        if (ImGui::Begin(obj_id.c_str()))
        {
            ImGui::PushID(object_id);
            ImGui::Separator();
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

            if (ImGui::TreeNode("action_state"))
            {
                static int min = ActionState::ActionIdle;
                static int max = ActionState::ActionStageMoveEnd;
                static int state{ 0 };
                ImGui::SliderInt("action_state", &state, min, max);
                action_state = static_cast<ActionState>(state);
                ImGui::TreePop();
            }
            ImGui::PopID();
            ImGui::End();
        }
    }
#endif // USE_IMGUI
    debug_figure->create_capsule(just_avoidance_capsule_param.start, just_avoidance_capsule_param.end, just_avoidance_capsule_param.rasius, { 1.0f,1.0f,1.0f,1.0f });

}

void ClientPlayer::Render(GraphicsPipeline& graphics, float elapsed_time)
{
    glow_time += 1.0f * elapsed_time;
    if (glow_time >= 3.0f) glow_time = 0;
    graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::PBR);
    SkinnedMesh::mesh_tuple armor_r_mdl = std::make_tuple("armor_R_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple armor_l_mdl = std::make_tuple("armor_L_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple wing_r_mdl = std::make_tuple("wing_R_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple wing_l_mdl = std::make_tuple("wing_L_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple largeblade_r_mdl = std::make_tuple("largeblade_R_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple largeblade_l_mdl = std::make_tuple("largeblade_L_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple prestarmor_mdl = std::make_tuple("prestarmor_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple backpack_mdl = std::make_tuple("backpack_mdl", threshold_mesh);
    SkinnedMesh::mesh_tuple camera_mdl = std::make_tuple("camera_mesh", threshold_camera_mesh);

    //-----ワールド行列を計算-----//
    const DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(scale, orientation, position);

    //-----もしキャッシュデータが設定以下ならそのまま追加-----//
    if (transform.size() < SkinnedMesh::MAX_CASHE_SIZE) transform.emplace_back(world);
    //-----もしキャッシュデータが多かったら先頭データを削除して挿入-----//
    else if (transform.size() >= SkinnedMesh::MAX_CASHE_SIZE)
    {
        transform.erase(transform.begin());
        transform.emplace_back(world);
    }
    model->render(graphics.get_dc().Get(), anim_parm,transform.at(0), { 1.0f,1.0f,1.0f,1.0f }, threshold, glow_time, emissive_color, 0.8f, armor_r_mdl, armor_l_mdl, wing_r_mdl, wing_l_mdl, largeblade_r_mdl, largeblade_l_mdl, prestarmor_mdl, backpack_mdl, camera_mdl);


    graphics.set_pipeline_preset(RASTERIZER_STATE::CULL_NONE, DEPTH_STENCIL::DEON_DWON, SHADER_TYPES::PBR);
    if (is_awakening)
    {
        mSwordTrail[0].fRender(graphics.get_dc().Get());
        mSwordTrail[1].fRender(graphics.get_dc().Get());
    }
    else
    {
        mSwordTrail[0].fRender(graphics.get_dc().Get());
    }

}

void ClientPlayer::StepCapsule()
{
    {
        DirectX::XMFLOAT3 pos = {}, up = {};
        DirectX::XMFLOAT3 end = {}, e_up = {};
        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[10], pos, up);
        step_pos_r = pos;
    }
    {
        DirectX::XMFLOAT3 pos = {}, up = {};
        DirectX::XMFLOAT3 end = {}, e_up = {};

        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[11], pos, up);
        step_pos_l = pos;
    }
}

void ClientPlayer::BodyCapsule()
{
    {
        DirectX::XMFLOAT3 pos = {}, up = {};
        DirectX::XMFLOAT3 end = {}, e_up = {};

        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[0], pos, up);
        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[1], end, e_up);

        body_capsule_param.start = pos;
        body_capsule_param.end = end;
        body_capsule_param.rasius = 1.5f;
    }
    {
        DirectX::XMFLOAT3 pos = {}, up = {};
        DirectX::XMFLOAT3 end = {}, e_up = {};

        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[0], pos, up);
        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[1], end, e_up);

        charge_capsule_param.start = pos;
        charge_capsule_param.end = end;
        charge_capsule_param.rasius = 4.0f;
    }
    {
        DirectX::XMFLOAT3 pos = {}, up = {};
        DirectX::XMFLOAT3 end = {}, e_up = {};

        model->fech_by_bone(anim_parm,Math::calc_world_matrix(scale, orientation, position), player_bones[0], pos, up);
        model->fech_by_bone(anim_parm,Math::calc_world_matrix(scale, orientation, position), player_bones[1], end, e_up);

        just_avoidance_capsule_param.start = pos;
        just_avoidance_capsule_param.end = end;
        just_avoidance_capsule_param.rasius = 10.6f;
    }
}

void ClientPlayer::SwordCapsule()
{
    DirectX::XMFLOAT3 pos = {}, up = {};
    DirectX::XMFLOAT3 pos_2 = {}, up_2 = {};
    DirectX::XMFLOAT3 end = {}, e_up = {};
    DirectX::XMFLOAT3 end_2 = {}, e_up_2 = {};
    if (is_awakening)
    {

        model->fech_by_bone(anim_parm,Math::calc_world_matrix(scale, orientation, position), player_bones[2], pos, up);
        model->fech_by_bone(anim_parm,Math::calc_world_matrix(scale, orientation, position), player_bones[3], end, e_up);
        model->fech_by_bone(anim_parm,Math::calc_world_matrix(scale, orientation, position), player_bones[4], pos_2, up_2);
        model->fech_by_bone(anim_parm,Math::calc_world_matrix(scale, orientation, position), player_bones[5], end_2, e_up_2);

        sword_capsule_param[0].start = pos;
        sword_capsule_param[0].end = end;
        sword_capsule_param[0].rasius = 1.7f;

        sword_capsule_param[1].start = pos_2;
        sword_capsule_param[1].end = end_2;
        sword_capsule_param[1].rasius = 1.7f;

    }
    else
    {
        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[6], pos, up);
        model->fech_by_bone(anim_parm, Math::calc_world_matrix(scale, orientation, position), player_bones[7], end, e_up);

        sword_capsule_param[0].start = pos;
        sword_capsule_param[0].end = end;
        sword_capsule_param[0].rasius = 1.7f;
        sword_capsule_param[1].start = pos;
        sword_capsule_param[1].end = end;
        sword_capsule_param[1].rasius = 1.7f;
    }
}

void ClientPlayer::StunSphere()
{
    //ジャスト回避中なら
    if (is_just_avoidance)
    {
        //覚醒状態なら
        if (is_awakening)sphere_radius = 17.0f;
        //覚醒状態じゃないなら
        else sphere_radius = 4.0f;
    }
    else sphere_radius = 0;

}

void ClientPlayer::InflectionParameters(float elapesd_time)
{
    player_config->set_hp_percent(static_cast<float>(static_cast<float>(player_health) / MAX_HEALTH));
    player_config->set_mp_percent(combo_count / MAX_COMBO_COUNT);
    //足元のカプセル
    StepCapsule();
    //体の大きさのカプセルパラメータ設定
    BodyCapsule();
    //剣の大きさのカプセルのパラメータ
    SwordCapsule();
    //ジャスト回避の範囲スタンのパラメータ
    StunSphere();
    //無敵時間の減少
    invincible_timer -= 1.0f * elapesd_time;
    //回り込み回避のクールタイム
    behaind_avoidance_cool_time -= 1.0f * elapesd_time;

}

void ClientPlayer::AddCombo(int count, bool& block)
{
    //ブロックされたかどうかを
    is_block = block;
    if (is_block)return;
    if (count != 0)
    {
        //もしブロックされていたら怯む
        combo_count += static_cast<float>(count);
        is_enemy_hit = true;
    }
    combo_count = Math::clamp(combo_count, 0.0f, MAX_COMBO_COUNT);
}

void ClientPlayer::AwakingAddCombo(int hit_count1, int hit_count2, bool& block)
{
    //ブロックされたかどうかを
    is_block = block;
    if (is_block)return;
    if (hit_count1 != 0 || hit_count2 != 0)
    {
        //もしブロックされていたら怯む
        combo_count += static_cast<float>(hit_count1 + hit_count2);
        is_enemy_hit = true;
    }
    combo_count = Math::clamp(combo_count, 0.0f, MAX_COMBO_COUNT);

}

void ClientPlayer::GetPlayerDirections()
{
}
