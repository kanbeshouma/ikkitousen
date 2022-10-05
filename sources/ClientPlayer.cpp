#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "ClientPlayer.h"

ClientPlayer::ClientPlayer(GraphicsPipeline& graphics, int object_id)
    :BasePlayer()
{
    model = resource_manager->load_model_resource(graphics.get_device().Get(), ".\\resources\\Models\\Player\\player_twentyfource.fbx", false, 60.0f);

    TransitionIdle();

    is_update_animation = true;

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
        ExecFuncUpdate(elapsed_time, sky_dome, enemies, graphics);
        return;
    }
    if (boss_camera)
    {
        velocity = {};
        position = { 1.0f,0.0f,-160.0f };
        is_lock_on = false;
        return;
    }

    ExecFuncUpdate(elapsed_time, sky_dome, enemies, graphics);

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

        if (avoidance_buttun)
        {
            if (triggerR < 0.1f && !(button_down & GamePad::BTN_RIGHT_SHOULDER))
            {
                avoidance_buttun = false;
            }
        }


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

    UpdateVelocity(elapsed_time, position, orientation,camera_forward,camera_right ,sky_dome);

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
                ImGui::DragFloat("allowable_limit_position", &allowable_limit_position, 0.1f);
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
            if (ImGui::TreeNode("SendData"))
            {
                DirectX::XMFLOAT3 input{ GetInputMoveVec() };
                ImGui::DragFloat3("movevec", &input.x, 0.1f);
                SetMoveVecter(input);
                GamePadButton newButtonState = 0;
                if (ImGui::Button("B"))
                {
                    newButtonState |= GamePad::BTN_ATTACK_B;
                }
                if (ImGui::Button("BTN_RIGHT_SHOULDER"))
                {
                    newButtonState |= GamePad::BTN_RIGHT_SHOULDER;
                }
                SetSendButton(newButtonState);

                ImGui::DragFloat("triggerR", &triggerR, 0.1f, 0, 1.0f);
                ImGui::DragFloat3("target", &target.x, 0.1f);
                ImGui::Checkbox("is_enemy_hit", &is_enemy_hit);
                ImGui::Checkbox("is_lock_on", &is_lock_on);
                debug_figure->create_sphere(target, 0.2f, { 1.0f,1.0f,0.0f,1.0f });
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

void ClientPlayer::SetReceiveData(PlayerMoveData data)
{
    //-----入力データ設定する-----//
    SetMoveVecter(data.move_vec);

    //-----ボタンの入力情報を入れる-----//
    SetSendButton(data.new_button_state);
}

void ClientPlayer::SetReceivePositionData(PlayerPositionData data)
{
    using namespace DirectX;

    //position = data.position;

    XMVECTOR p1{ XMLoadFloat3(&position) };
    XMVECTOR p2{ XMLoadFloat3(&data.position) };

    XMVECTOR dir{ p2 - p1 };

    XMVECTOR l{ XMVector3Length(dir) };
    float length{};
    XMStoreFloat(&length, l);

    if (length > allowable_limit_position)
    {
        //ここで許容値を超えていたらその位置からの移動速度を考慮した位置をだして
        //そこに向かって補完していく
        SetLerpPosition(data.position);
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
    using namespace DirectX;
    XMVECTOR orientation_vec{ XMLoadFloat4(&orientation) };
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientation_vec);
    DirectX::XMFLOAT4X4 m4x4 = {};
    DirectX::XMStoreFloat4x4(&m4x4, m);
    DirectX::XMVECTOR right_vec, forward_vec, up_vec;

    right_vec = { m4x4._11, m4x4._12, m4x4._13 };
    up_vec = { m4x4._21, m4x4._22, m4x4._23 };
    forward_vec = { m4x4._31, m4x4._32, m4x4._33 };

    XMStoreFloat3(&right, right_vec);
    XMStoreFloat3(&up, up_vec);
    XMStoreFloat3(&forward, forward_vec);
}

void ClientPlayer::BehindAvoidancePosition()
{
    //もし配列の中に何か入っていたら削除する
    if (!behind_way_points.empty()) behind_way_points.clear();
    if (!behind_interpolated_way_points.empty()) behind_interpolated_way_points.clear();

    using namespace DirectX;
    XMFLOAT3 p{ position.x,position.y + step_offset_y,position.z };
    float length_radius = Math::calc_vector_AtoB_length(p, target);//距離(半径)
    float diameter = length_radius * 0.6f;//(直径)
    //どっちのvelocityで左右判定するか
    DirectX::XMFLOAT3 r{ right };
    DirectX::XMFLOAT3 behind_point_4{};
    if (game_pad->get_axis_LX() < 0)
    {
        r.x = -r.x;
        r.y = -r.y;
        r.z = -r.z;
    }
    ////----------------中継１---------------------//
    behind_point_1.x = target.x + (((r.x * cosf(DirectX::XMConvertToRadians(300.0f))) + (forward.x * sinf(DirectX::XMConvertToRadians(300.0f)))) * (length_radius * 0.6f));//敵の後ろ側
    behind_point_1.y = position.y;//敵の後ろ側
    behind_point_1.z = target.z + (((r.z * cosf(DirectX::XMConvertToRadians(300.0f))) + (forward.z * sinf(DirectX::XMConvertToRadians(300.0f)))) * (length_radius * 0.6f));//敵の後ろ側
    ////--------------------------------------------//
    ////----------------中継2---------------------//
    behind_point_2.x = target.x + (((r.x * cosf(DirectX::XMConvertToRadians(340.0f))) + (forward.x * sinf(DirectX::XMConvertToRadians(340.0f)))) * (length_radius * 0.4f));//敵の後ろ側
    behind_point_2.y = position.y;//敵の後ろ側
    behind_point_2.z = target.z + (((r.z * cosf(DirectX::XMConvertToRadians(340.0f))) + (forward.z * sinf(DirectX::XMConvertToRadians(340.0f)))) * (length_radius * 0.4f));//敵の後ろ側
    //--------------------------------------------//
    behind_point_3.x = target.x + (((r.x * cosf(DirectX::XMConvertToRadians(20.0f))) + (forward.x * sinf(DirectX::XMConvertToRadians(20.0f)))) * (length_radius * 0.4f));//敵の後ろ側
    behind_point_3.y = position.y;//敵の後ろ側
    behind_point_3.z = target.z + (((r.z * cosf(DirectX::XMConvertToRadians(20.0f))) + (forward.z * sinf(DirectX::XMConvertToRadians(20.0f)))) * (length_radius * 0.4f));//敵の後ろ側
    //-----------------ゴール地点---------------//
    behind_point_4.x = target.x + (((r.x * cosf(DirectX::XMConvertToRadians(90.0f))) + (forward.x * sinf(DirectX::XMConvertToRadians(90.0f)))) * (length_radius * 1.0f));//敵の後ろ側
    behind_point_4.y = position.y;//敵の後ろ側
    behind_point_4.z = target.z + (((r.z * cosf(DirectX::XMConvertToRadians(90.0f))) + (forward.z * sinf(DirectX::XMConvertToRadians(90.0f)))) * (length_radius * 1.0f));//敵の後ろ側

    behind_way_points.emplace_back(position); // この時点でのプレイヤーの位置
    behind_way_points.emplace_back(behind_point_1); // この時点でのプレイヤーの位置
    behind_way_points.emplace_back(behind_point_2); // プレイヤーの位置とゴールの位置の中間地点
    behind_way_points.emplace_back(behind_point_3); // 中継地点
    behind_way_points.emplace_back(behind_point_4); //ゴールの位置

    {
        const size_t step = 3;
        // way_pointsを通るカーブを作成
        CatmullRomSpline curve(behind_way_points);
        curve.interpolate(behind_interpolated_way_points, step);

        behind_transit_index = 0;
#if 1
        //自分の位置とウェイポイントのベクトルを求める
    //+1しているのはindex番目の値が自分の位置だからその次の値を取得するため
        XMVECTOR vec = XMLoadFloat3(&behind_interpolated_way_points.at(0)) - XMLoadFloat3(&behind_interpolated_way_points.at(1));
        //長さを求めて
        XMVECTOR length_vec = DirectX::XMVector3Length(vec);
        //その値を取得(4dベクトルのxの値を取ってくる関数)
        //一区分当たりの距離
        float length = DirectX::XMVectorGetX(length_vec);
        const float behind_time{ 0.5f };
        //一区分当たりの時間
        const float time = behind_time / (static_cast<float>(step) * 4.0f);
        behind_speed = length / time;

#endif // 1
    }
    //--------------------------------------------//
}

bool ClientPlayer::BehindAvoidanceMove(float elapsed_time, int& index, DirectX::XMFLOAT3& position, float speed, const std::vector<DirectX::XMFLOAT3>& points, float play)
{
    using namespace DirectX;
    assert(!points.empty() && "ポイントのサイズが0です");
    //index(配列の中の自分の位置)がゴールの位置にきたらtrueを返す
    //(この後の計算でindexの次の値を使うからサイズと同じ大きさでもダメ)
    if (index >= points.size() - 1) return true;

    XMFLOAT3 velo{};
    //自分の位置とウェイポイントのベクトルを求める
    //+1しているのはindex番目の値が自分の位置だからその次の値を取得するため
    XMVECTOR vec = XMLoadFloat3(&points.at(index + 1)) - XMLoadFloat3(&position);
    XMVECTOR norm_vec = XMVector3Normalize(vec);
    XMStoreFloat3(&velo, norm_vec);
    //長さを求めて
    XMVECTOR length_vec = DirectX::XMVector3Length(vec);
    //その値を取得(4dベクトルのxの値を取ってくる関数)
    float length = DirectX::XMVectorGetX(length_vec);
    //もし長さが設定されてる値よりも小さくなったら
    if (length <= play)
    {
        //indexを１増やして
        ++index;
        //自分の位置に代入する(+1された値がゴールだったから)
        position = points.at(index);
    }

    position.x += velo.x * speed * elapsed_time;
    position.y += velo.y * speed * elapsed_time;
    position.z += velo.z * speed * elapsed_time;

    return false;
}

void ClientPlayer::SetAccelerationVelocity()
{
    if (is_lock_on)
    {
        DirectX::XMFLOAT3 v{};
        DirectX::XMStoreFloat3(&v, DirectX::XMVector3Normalize(Math::calc_vector_AtoB(position, target)));
        float length{ Math::calc_vector_AtoB_length(position,target) };

        velocity.x = v.x * charge_length_magnification;
        velocity.y = v.y * charge_length_magnification;
        velocity.z = v.z * charge_length_magnification;
        //position = Math::lerp(position, target, 10.0f * elapse_time);
    }
    else
    {
        DirectX::XMFLOAT3 v{};
        DirectX::XMStoreFloat3(&v, DirectX::XMVector3Normalize(Math::calc_vector_AtoB(position, charge_point)));
        float length{ Math::calc_vector_AtoB_length(position,charge_point) };

        velocity.x = v.x * 100.0f;
        velocity.y = v.y * 100.0f;
        velocity.z = v.z * 100.0f;

        //position = Math::lerp(position, charge_point, 7.0f * elapse_time);
    }
}

void ClientPlayer::SetSendButton(GamePadButton input)
{
    //-----ひとつ前のボタンを保存-----//
    button_state[1] = button_state[0];

    //-----設定-----//
    button_state[0] = input;

    //-----押した瞬間-----//
    button_down = ~button_state[1] & input;

    //-----離した瞬間-----//
    button_up = ~input & button_state[1];
}