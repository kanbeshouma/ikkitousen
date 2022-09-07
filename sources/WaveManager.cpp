#include"WaveManager.h"
#include "Operators.h"
#include "scene_game.h"
#include "scene_title.h"
#include "scene_loading.h"
#include "scene_manager.h"
#include "post_effect.h"

#define ProtoType

void WaveManager::fInitialize(GraphicsPipeline& graphics_,AddBulletFunc Func_)
{
    // 初期化
    mEnemyManager.fInitialize(graphics_,Func_);

    mWaveState = WaveState::Start;
    mStartGame = true;

    //----------------------------------
    // TODO:藤岡が書いたところ2
    //----------------------------------
    // ステージのロード
    WaveFile::get_instance().load();
    current_stage = static_cast<STAGE_IDENTIFIER>(WaveFile::get_instance().get_stage_to_start());

    // ステージ情報の登録
    DirectX::XMFLOAT2 stage_points[STAGE_IDENTIFIER::STAGE_COUNT] =
    {
        { 1128, 1310 },{ 784, 974 },{ 1518, 974 },{ 484, 564 },{ 1070, 564 },
        { 1520, 564 },{ 1070, 176 }
    };
    for (int i = 0; i < STAGE_IDENTIFIER::STAGE_COUNT; ++i) { stage_details[i].position = stage_points[i]; }
    /*1-1*/
    {
        STAGE_IDENTIFIER index = STAGE_IDENTIFIER::S_1_1;
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::LEFT, STAGE_IDENTIFIER::S_2_1));
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::RIGHT, STAGE_IDENTIFIER::S_2_2));
    }
    /*2-1*/
    {
        STAGE_IDENTIFIER index = STAGE_IDENTIFIER::S_2_1;
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::LEFT, STAGE_IDENTIFIER::S_3_1));
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::RIGHT, STAGE_IDENTIFIER::S_3_2));
    }
    /*2-2*/
    {
        STAGE_IDENTIFIER index = STAGE_IDENTIFIER::S_2_2;
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::UP, STAGE_IDENTIFIER::S_3_3));
    }
    /*3-1*/
    {
        STAGE_IDENTIFIER index = STAGE_IDENTIFIER::S_3_1;
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::UP, STAGE_IDENTIFIER::BOSS));
    }
    /*3-2*/
    {
        STAGE_IDENTIFIER index = STAGE_IDENTIFIER::S_3_2;
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::UP, STAGE_IDENTIFIER::BOSS));
    }
    /*3-3*/
    {
        STAGE_IDENTIFIER index = STAGE_IDENTIFIER::S_3_3;
        stage_details[index].journeys.insert(std::make_pair(StageDetails::ROUTE::UP, STAGE_IDENTIFIER::BOSS));
    }
    /*BOSS*/
    {
        /* 次のルートなし */
    }

    // map
    map.sprite = std::make_unique<SpriteDissolve>(graphics_.get_device().Get(),
        L".\\resources\\Sprites\\clear_wave\\map.png", L".\\resources\\Sprites\\clear_wave\\map_mask.png", 1);
    map.arg.texsize = { (float)map.sprite->get_texture2d_desc().Width, (float)map.sprite->get_texture2d_desc().Height };
    // player_icon
    player_icon.sprite = std::make_unique<SpriteDissolve>(graphics_.get_device().Get(),
        L".\\resources\\Sprites\\clear_wave\\player_icon.png", L".\\resources\\Sprites\\clear_wave\\player_icon_mask.png", 1);
    player_icon.arg.texsize = { (float)player_icon.sprite->get_texture2d_desc().Width, (float)player_icon.sprite->get_texture2d_desc().Height };
    player_icon.arg.pivot = player_icon.arg.texsize * DirectX::XMFLOAT2{ 0.5f, 0.5f };
    // arrows
    {
        arrow_sprite = std::make_unique<SpriteDissolve>(graphics_.get_device().Get(),
            L".\\resources\\Sprites\\clear_wave\\arrow.png", L".\\resources\\Sprites\\clear_wave\\player_icon_mask.png", 4);
        Arrow arrow;
        arrow.arg.texsize = { (float)arrow_sprite->get_texture2d_desc().Width, (float)arrow_sprite->get_texture2d_desc().Height };
        arrow.arg.pivot = arrow.arg.texsize * DirectX::XMFLOAT2{ 0.5f, 0.5f };
        /*LEFT*/
        {
            arrow.arg.angle = 180.0f;
            arrows.insert(std::make_pair(StageDetails::ROUTE::LEFT, arrow));
        }
        /*RIGHT*/
        {
            arrow.arg.angle = 0.0f;
            arrows.insert(std::make_pair(StageDetails::ROUTE::RIGHT, arrow));
        }
        /*UP*/
        {
            arrow.arg.angle = -90.0f;
            arrows.insert(std::make_pair(StageDetails::ROUTE::UP, arrow));
        }
        /*DOWN*/
        {
            arrow.arg.angle = 90.0f;
            arrows.insert(std::make_pair(StageDetails::ROUTE::DOWN, arrow));
        }
    }

    // clear_parameters
    clear_parameters.sprite_clear = std::make_unique<SpriteDissolve>(graphics_.get_device().Get(),
        L".\\resources\\Sprites\\stageclear.png", L".\\resources\\Sprites\\clear_wave\\player_icon_mask.png", 1);
    const int FRAMES_X = 4;   const int FRAMES_Y = 2;
    clear_parameters.clear.texsize = { (float)clear_parameters.sprite_clear->get_texture2d_desc().Width / FRAMES_X,
                                       (float)clear_parameters.sprite_clear->get_texture2d_desc().Height / FRAMES_Y };
    clear_parameters.clear.pivot = player_icon.arg.texsize * DirectX::XMFLOAT2{ 0.5f, 0.5f };
    clear_parameters.clear.scale = { 1,1 };
    clear_parameters.clear.pos   = { 540,265 };

    clear_parameters.initialize();


    game_clear = false;

    transition_reduction();
    //---ここまで--//
}

void WaveManager::fUpdate(GraphicsPipeline& Graphics_ ,float elapsedTime_, AddBulletFunc Func_)
{
    // 待ってクリア演出へ
    if (clear_flg)
    {
        clear_wait_timer -= elapsedTime_;
        // クリアのアニメーション
        if (current_stage != STAGE_IDENTIFIER::BOSS)
        {
            if (clear_wait_timer < CLEAR_WAIT_TIME - CLEAR_ANIMATION_WAIT_TIME) // 2秒待ってクリアアニメーション
            {
                if (!clear_parameters.se_play)
                {
                    audio_manager->play_se(SE_INDEX::DRAW_PEN);
                    clear_parameters.se_play = true;
                }

                if (clear_wait_timer >= CLEAR_WAIT_TIME - CLEAR_ANIMATION_FADE_WAIT_TIME)
                {
                    clear_parameters.clear.color.w = Math::lerp(clear_parameters.clear.color.w, 1.0f, 4.0f * elapsedTime_);
                }

                //--parameters--//
                const int FRAMW_COUNT_X = 4;
                const int FRAMW_COUNT_Y = 2;
                static float logo_animation_speed = 0.05f;

                int frame_x;
                frame_x = static_cast<int>(clear_parameters.timer / logo_animation_speed) % (FRAMW_COUNT_X + 1);
#ifdef USE_IMGUI
                ImGui::Begin("ClearProto");
                ImGui::DragFloat("speed", &logo_animation_speed, 0.01f);
                ImGui::Text("timer:%f", clear_parameters.timer);
                ImGui::Text("frame_x:%d", frame_x);
                ImGui::Text("frame_y:%d", clear_parameters.frame_y);
                ImGui::End();
#endif // USE_IMGUI
                if (frame_x >= FRAMW_COUNT_X)
                {
                    // 1行下のアニメーションへ
                    if (clear_parameters.frame_y < FRAMW_COUNT_Y - 1)
                    {
                        clear_parameters.timer = 0;
                        ++clear_parameters.frame_y;
                    }
                }
                else
                {
                    // アニメーション
                    clear_parameters.clear.texpos.x = frame_x * clear_parameters.clear.texsize.x;
                    clear_parameters.clear.texpos.y = clear_parameters.frame_y * clear_parameters.clear.texsize.y;
                    clear_parameters.timer += elapsedTime_;
                }
            }
            if (clear_wait_timer < CLEAR_WAIT_TIME - CLEAR_ANIMATION_FADE_WAIT_TIME)
            {
                clear_parameters.clear.color.w = Math::lerp(clear_parameters.clear.color.w, -0.5f, 2.0f * elapsedTime_);
#ifdef USE_IMGUI
                ImGui::Begin("ClearProto");
                ImGui::Text("alpha:%f", clear_parameters.clear.color.w);
                ImGui::End();
#endif // USE_IMGUI
            }
        }
        else // ゲームクリア
        {
            audio_manager->stop_all_bgm();
            game_clear = true;

            return;
        }
        // クリア演出orゲームクリア
        if (clear_wait_timer < 0)
        {
            if (current_stage != STAGE_IDENTIFIER::BOSS) // クリア演出へ
            {
                mWaveState = WaveState::Clear;
                //----------------------------------
                // TODO:藤岡が書いたところ6
                //----------------------------------
                transition_reduction();

                PostEffect::clear_post_effect();

                effect_manager->finalize();
                effect_manager->initialize(Graphics_);

                //---ここまで--//
            }
        }
    }

    switch (mWaveState)
    {
    case WaveState::Start:
        // 一番最初はクリア演出なしでステージ開始するからここでウェーブ開始(ゲーム中最初の一回しか入らない)
        if (mStartGame)
        {
            fStartWave();
            mStartGame = false;
        }
        break;
    case WaveState::Game:
        mEnemyManager.fUpdate(Graphics_,elapsedTime_,Func_);

        // クリア状態に遷移
        if (mEnemyManager.fGetClearWave() || mEnemyManager.fGetBossClear()) { clear_flg = true; }

        break;
    case WaveState::Clear:
        fClearUpdate(elapsedTime_);
        break;
    default: ;
    }
    fGuiMenu();
}

void WaveManager::render(ID3D11DeviceContext* dc, float elapsed_time)
{
    //----------------------------------
    // TODO:藤岡が書いたところ3
    //----------------------------------
    auto r_dissolve = [&](std::string g_name, SpriteDissolve* dissolve, SpriteArg& arg, float& threshold)
    {
#ifdef USE_IMGUI
        ImGui::Begin(g_name.c_str());
        ImGui::DragFloat2("pos", &arg.pos.x);
        ImGui::DragFloat2("scale", &arg.scale.x, 0.1f);
        ImGui::DragFloat("threshold", &threshold, 0.01f);
        ImGui::End();
#endif // USE_IMGUI
        dissolve->begin(dc);
        dissolve->render(dc, arg.pos, arg.scale, arg.pivot, arg.color, arg.angle, arg.texpos, arg.texsize, threshold);
        dissolve->end(dc);
    };
    // clear_parameters
    if (clear_flg && clear_wait_timer < CLEAR_WAIT_TIME - CLEAR_ANIMATION_WAIT_TIME)
    {
        r_dissolve("Clear", clear_parameters.sprite_clear.get(), clear_parameters.clear, clear_parameters.threshold);
    }

    if (mWaveState == WaveState::Clear)
    {
        r_dissolve("Map", map.sprite.get(), map.arg, map.threshold);

        player_icon.sprite->begin(dc);
        player_icon.sprite->render(dc, player_icon.arg.pos + DirectX::XMFLOAT2(0,-0.5f), player_icon.arg.scale,
            player_icon.arg.pivot, player_icon.arg.color, player_icon.arg.angle, player_icon.arg.texpos, player_icon.arg.texsize, player_icon.threshold);
        player_icon.sprite->end(dc);

        for (auto& arrow : arrows)
        {
            for (const auto& journey : stage_details[current_stage].journeys)
            {
                if (arrow.first == journey.first && clear_state == CLEAR_STATE::SELECTION)
                {
                    std::string s = "arrow " + std::to_string((int)arrow.first);
                    r_dissolve(s, arrow_sprite.get(), arrow.second.arg, arrow.second.threshold);
                }
            }
        }

        if (clear_state == CLEAR_STATE::SELECTION)
        {
            auto r_font_render = [&](std::string name, std::wstring string, DirectX::XMFLOAT2& pos, DirectX::XMFLOAT2& scale)
            {
#ifdef USE_IMGUI
                ImGui::Begin("WaveManager");
                if (ImGui::TreeNode(name.c_str()))
                {
                    ImGui::DragFloat2("pos", &pos.x);
                    ImGui::DragFloat2("scale", &scale.x, 0.1f);
                    ImGui::TreePop();
                }
                ImGui::End();
#endif // USE_IMGUI
                fonts->yu_gothic->Draw(string, pos, scale, { 1,1,1,1 }, 0, TEXT_ALIGN::MIDDLE);
            };
            fonts->yu_gothic->Begin(dc);
            {
                static DirectX::XMFLOAT2 pos{ 1266.0f, 634.0f };
                static DirectX::XMFLOAT2 scale{ 0.8f, 0.8f };
                r_font_render("L", L"Lスティック  選択", pos, scale);
            }
            {
                static DirectX::XMFLOAT2 pos{ 1265.0f, 580.0f };
                static DirectX::XMFLOAT2 scale{ 0.8f, 0.8f };
                r_font_render("R", L"Rスティック  移動", pos, scale);
            }
            {
                static DirectX::XMFLOAT2 pos{ 1232.0f, 686.0f };
                static DirectX::XMFLOAT2 scale{ 0.8f, 0.8f };
                r_font_render("B", L"Bボタン  決定", pos, scale);
            }
            fonts->yu_gothic->End(dc);
        }
    }

    //---ここまで--//


}

void WaveManager::fFinalize()
{
    mEnemyManager.fFinalize();
}

void WaveManager::fStartWave()
{
    mWaveState = WaveState::Game;
    mCurrentWave = current_stage;

    // ファイルにセーブ
    WaveFile::get_instance().set_stage_to_start(mCurrentWave);
    WaveFile::get_instance().save();

    mEnemyManager.fStartWave(mCurrentWave);
}

 EnemyManager* WaveManager::fGetEnemyManager()
{
    return &mEnemyManager;
}

void WaveManager::fGuiMenu()
{
    imgui_menu_bar("System", "WaveManager", mOpenGui);

#ifdef USE_IMGUI
    if (mOpenGui)
    {
        ImGui::Begin("WaveManager");
        ImGui::Text("mCurrentWave:%d", mCurrentWave);
        //----------------------------------
        // TODO:藤岡が書いたところ8
        //----------------------------------
        if (ImGui::Button("ClearGame"))
        {
            clear_flg = true;
        }
        //---ここまで--//

        ImGui::Text("State ");
        ImGui::SameLine();
        switch (mWaveState)
        {
        case WaveState::Start: ImGui::Text("Start"); break;
        case WaveState::Game:  ImGui::Text("Game");  break;
        case WaveState::Clear: ImGui::Text("Clear"); break;
        default: break;
        }

        ImGui::End();
    }
#endif
}

void WaveManager::fClearUpdate(float elapsedTime_)
{
    //----------------------------------
    // TODO:藤岡が書いたところ4
    //----------------------------------
    if (!close)
    {
        map.threshold = Math::lerp(map.threshold, -0.5f, 2.0f * elapsedTime_);
        player_icon.threshold = Math::lerp(player_icon.threshold, -0.5f, 2.0f * elapsedTime_);
    }
    else
    {
        map.threshold = Math::lerp(map.threshold, 1.5f, 2.0f * elapsedTime_);
        player_icon.threshold = Math::lerp(player_icon.threshold, 1.5f, 2.0f * elapsedTime_);
        if (Math::equal_check(player_icon.threshold, 1.0f, 0.1f))
        {
            PostEffect::clear_post_effect();

            clear_parameters.initialize();
            // クリア演出終了、次のステージへ
            fStartWave();
        }
    }
    // state別の更新処理
    switch (clear_state)
    {
    case CLEAR_STATE::REDUCTION:   // 縮小
        update_reduction(elapsedTime_);
        break;
    case CLEAR_STATE::SELECTION:   // 選択
        update_selection(elapsedTime_);
        break;
    case CLEAR_STATE::MOVE:   // 選択
        update_move(elapsedTime_);
        break;
    case CLEAR_STATE::ENLARGEMENT: // 拡大
        update_enlargement(elapsedTime_);
        break;
    }
    // 矢印の位置
    {
        DirectX::XMFLOAT2 offset = player_icon.arg.texsize * player_icon.arg.scale * 0.8f;
        if (arrows.count(StageDetails::ROUTE::LEFT))  { arrows.at(StageDetails::ROUTE::LEFT).arg.pos   = player_icon.arg.pos + DirectX::XMFLOAT2(-offset.x, 0); }
        if (arrows.count(StageDetails::ROUTE::RIGHT)) { arrows.at(StageDetails::ROUTE::RIGHT).arg.pos = player_icon.arg.pos + DirectX::XMFLOAT2(offset.x, 0); }
        if (arrows.count(StageDetails::ROUTE::UP))    { arrows.at(StageDetails::ROUTE::UP).arg.pos       = player_icon.arg.pos + DirectX::XMFLOAT2(0, -offset.y); }
        if (arrows.count(StageDetails::ROUTE::DOWN))  { arrows.at(StageDetails::ROUTE::DOWN).arg.pos   = player_icon.arg.pos + DirectX::XMFLOAT2(0, offset.y); }
    }
#ifdef USE_IMGUI
    ImGui::Begin("ClearProto");
    const char* elems_names[STAGE_IDENTIFIER::STAGE_COUNT] = { "S_1_1", "S_2_1", "S_2_2", "S_3_1", "S_3_2", "S_3_3", "BOSS" };
    {
        static int elem = current_stage;
        const char* elem_name = (elem >= 0 && elem < STAGE_IDENTIFIER::STAGE_COUNT) ? elems_names[elem] : "Unknown";
        if (ImGui::SliderInt("change stage", &elem, 0, STAGE_IDENTIFIER::STAGE_COUNT - 1, elem_name))
        {
            current_stage = (STAGE_IDENTIFIER)elem;
            transition_reduction();
        }
        ImGui::Separator();
    }
    if (ImGui::Button("transition")) { transition_reduction(); }
    ImGui::DragFloat2("viewpoint", &viewpoint.x);
    ImGui::End();
#endif

    //---ここまで--//


#ifdef USE_IMGUI
    ImGui::Begin("ClearProto");
    if (ImGui::Button("NextWave"))
    {
        fStartWave();
    }
    ImGui::End();
#endif
}

//----------------------------------
// TODO:藤岡が書いたところ5
//----------------------------------
void WaveManager::transition_reduction()
{
    clear_state = CLEAR_STATE::REDUCTION;

    clear_flg = false;
    clear_wait_timer = CLEAR_WAIT_TIME;

    close = false;
    viewpoint = { 640.0f, 360.0f };
    wait_timer = 1.5f;

    float ratio = (stage_details[current_stage].position.y / map.arg.texsize.y);

    arrival_viewpoint = { 640.0f,  360.0f };
    if (ratio > 0.8f) { arrival_viewpoint.y = 700.0f * ratio; }

    arrival_scale = { 1.0f,1.0f };
    // map
    map.threshold = 10.0f;
    map.arg.pos = viewpoint - stage_details[current_stage].position * map.arg.scale;
    map.arg.scale = { 6.0f, 6.0f };
    // player_icon
    player_icon.threshold = 10.0f; // プレイヤーのアニメーションを待つ
    player_icon.arg.pos = viewpoint;
    player_icon.arg.scale = DirectX::XMFLOAT2(0.3f, 0.3f) * map.arg.scale;
}

void WaveManager::update_reduction(float elapsed_time)
{
    wait_timer -= elapsed_time;
    wait_timer = (std::max)(wait_timer, 0.0f);
    if (wait_timer > 0.0f) return;
    // viewpointの移動
    float lerp_rate = 2.0f;
    viewpoint = Math::lerp(viewpoint, arrival_viewpoint, lerp_rate * elapsed_time);
    // positionの移動
    map.arg.pos = viewpoint - stage_details[current_stage].position * map.arg.scale;
    player_icon.arg.pos = viewpoint;
    // scaleの変更
    map.arg.scale = Math::lerp(map.arg.scale, arrival_scale, lerp_rate * elapsed_time);
    player_icon.arg.scale = DirectX::XMFLOAT2(0.3f, 0.3f) * map.arg.scale;

    // 選択状態に遷移
    if (Math::equal_check(map.arg.scale.x, arrival_scale.x, 0.01f))
    {
        transition_selection();
    }
}

void WaveManager::transition_selection()
{
    if (current_stage == STAGE_IDENTIFIER::BOSS)
    {
        bool a = true;
        assert(!a && "ボス部屋です");
    }

    clear_state = CLEAR_STATE::SELECTION;
    route_state = stage_details[current_stage].journeys.begin()->first;

    // 矢印のスケール
    for (auto& arrow : arrows)
    {
        arrow.second.arg.scale = player_icon.arg.scale;
        arrow.second.threshold = 1.0f;
    }
}

void WaveManager::update_selection(float elapsed_time)
{
    auto r_transition_left = [&]()
    {
        if ((game_pad->get_button_down() & GamePad::BTN_LEFT) || game_pad->get_axis_LX() < -0.5f)
        {
            for (const auto& journey : stage_details[current_stage].journeys)
            {
                if (journey.first == StageDetails::ROUTE::LEFT) { route_state = StageDetails::ROUTE::LEFT; break; }
            }
        }
    };
    auto r_transition_right = [&]()
    {
        if ((game_pad->get_button_down() & GamePad::BTN_RIGHT) || game_pad->get_axis_LX() > 0.5f)
        {
            for (const auto& journey : stage_details[current_stage].journeys)
            {
                if (journey.first == StageDetails::ROUTE::RIGHT) { route_state = StageDetails::ROUTE::RIGHT; break; }
            }
        }
    };
    auto r_transition_up = [&]()
    {
        if ((game_pad->get_button_down() & GamePad::BTN_UP) || game_pad->get_axis_LY() > 0.5f)
        {
            for (const auto& journey : stage_details[current_stage].journeys)
            {
                if (journey.first == StageDetails::ROUTE::UP) { route_state = StageDetails::ROUTE::UP; break; }
            }
        }
    };
    auto r_transition_down = [&]()
    {
        if ((game_pad->get_button_down() & GamePad::BTN_DOWN) || game_pad->get_axis_LY() < -0.5f)
        {
            for (const auto& journey : stage_details[current_stage].journeys)
            {
                if (journey.first == StageDetails::ROUTE::DOWN) { route_state = StageDetails::ROUTE::DOWN; break; }
            }
        }
    };
    switch (route_state)
    {
    case StageDetails::ROUTE::LEFT:
        r_transition_right();
        r_transition_up();
        r_transition_down();
        break;
    case StageDetails::ROUTE::RIGHT:
        r_transition_left();
        r_transition_up();
        r_transition_down();
        break;
    case StageDetails::ROUTE::UP:
        r_transition_left();
        r_transition_right();
        r_transition_down();
        break;
    case StageDetails::ROUTE::DOWN:
        r_transition_left();
        r_transition_right();
        r_transition_up();
        break;
    }
    // 選択している矢印は大きく
    for (auto& arrow : arrows)
    {
        arrow.second.threshold = Math::lerp(arrow.second.threshold, -0.5f, 2.0f * elapsed_time);

        if (arrow.first == route_state) { arrow.second.arg.scale = player_icon.arg.scale * 0.5f; }
        else { arrow.second.arg.scale = player_icon.arg.scale * 0.3f; }
    }
    // next_stageのセット
    if (game_pad->get_button_down() & GamePad::BTN_B)
    {
        next_stage = stage_details[current_stage].journeys.at(route_state);
        transition_move();
    }

    DirectX::XMFLOAT2 min_point = map.arg.pos;
    DirectX::XMFLOAT2 max_point = { min_point + map.arg.texsize * map.arg.scale };

    // 右スティックでマップ移動
    if (game_pad->get_axis_RX() < 0)
    {
        if (min_point.x < 0)
        {
            viewpoint.x -= game_pad->get_axis_RX() * 600.0f * elapsed_time;
        }
    }
    else
    {
        if (max_point.x > 1280)
        {
            viewpoint.x -= game_pad->get_axis_RX() * 600.0f * elapsed_time;
        }
    }

    if (game_pad->get_axis_RY() < 0)
    {
        if (max_point.y > 720)
        {
            viewpoint.y += game_pad->get_axis_RY() * 600.0f * elapsed_time;
        }
    }
    else
    {
        if (min_point.y < 0)
        {
            viewpoint.y += game_pad->get_axis_RY() * 600.0f * elapsed_time;
        }
    }

    // 移動
    map.arg.pos = viewpoint - stage_details[current_stage].position * map.arg.scale;
    player_icon.arg.pos = viewpoint;

#ifdef USE_IMGUI
    ImGui::Begin("ClearProto");
    {
        ImGui::Separator();
        int elem = stage_details[current_stage].journeys.at(route_state);
        const char* elems_names[STAGE_IDENTIFIER::STAGE_COUNT] = { "S_1_1", "S_2_1", "S_2_2", "S_3_1", "S_3_2", "S_3_3", "BOSS"};
        const char* elem_name = (elem >= 0 && elem < STAGE_IDENTIFIER::STAGE_COUNT) ? elems_names[elem] : "Unknown";
        ImGui::SliderInt("candidate stage", &elem, 0, STAGE_IDENTIFIER::STAGE_COUNT - 1, elem_name);
        ImGui::Separator();
    }
    ImGui::End();
#endif // USE_IMGUI
}

void WaveManager::transition_move()
{
    clear_state = CLEAR_STATE::MOVE;

    for (auto& can_move : can_moves) { can_move = false; }

    DirectX::XMFLOAT2 current_position = stage_details[current_stage].position;
    DirectX::XMFLOAT2 next_position    = stage_details[next_stage].position;

    if (current_stage == STAGE_IDENTIFIER::S_3_1 || current_stage == STAGE_IDENTIFIER::S_3_2 || current_stage == STAGE_IDENTIFIER::S_3_3)
    {
        middle_point = { current_position.x, next_position.y };
    }
    else
    {
        middle_point = { next_position.x, current_position.y };
    }

    terminus_point     = next_position;
    interpolated_point = current_position;

    view_middle_point = { arrival_viewpoint.x,  stage_details[next_stage].position.y * map.arg.scale.y };
}

void WaveManager::update_move(float elapsed_time)
{
    // 移動
    if (Math::equal_check(viewpoint, arrival_viewpoint, 0.5f)) { can_moves[0] = true; }
    if (!can_moves[0])
    {
        float lerp_rate = 2.0f;
        viewpoint = Math::lerp(viewpoint, arrival_viewpoint, lerp_rate * elapsed_time);

        player_icon.arg.pos = viewpoint;
        map.arg.pos = viewpoint - stage_details[current_stage].position * map.arg.scale;
    }

    if (interpolated_point.y < 335.0f)
    {
        if (!can_moves[1] && Math::equal_check(viewpoint, view_middle_point, 0.5f))
        {
            can_moves[1] = true;
            terminus_point.y -= 20.0f;
            interpolated_point.y = terminus_point.y;
        }
        if (can_moves[0] && !can_moves[1])
        {
            float lerp_rate = 5.0f;
            viewpoint = Math::lerp(viewpoint, view_middle_point, lerp_rate * elapsed_time);

            player_icon.arg.pos = viewpoint;
        }
    }
    else
    {
        if (Math::equal_check(interpolated_point, middle_point, 0.5f)) { can_moves[1] = true; }
        if (can_moves[0] && !can_moves[1])
        {
            float lerp_rate = 5.0f;
            interpolated_point = Math::lerp(interpolated_point, middle_point, lerp_rate * elapsed_time);

            map.arg.pos = viewpoint - interpolated_point * map.arg.scale;
        }
    }

    if (Math::equal_check(interpolated_point, terminus_point, 0.5f)) { transition_enlargement(); }
    if (can_moves[1])
    {
        float lerp_rate = 5.0f;
        interpolated_point = Math::lerp(interpolated_point, terminus_point, lerp_rate * elapsed_time);

        map.arg.pos = viewpoint - interpolated_point * map.arg.scale;
    }

#ifdef USE_IMGUI
    ImGui::Begin("ClearProto");
    {
        ImGui::Separator();
        ImGui::DragFloat2("interpolated_point", &interpolated_point.x, 0.1f);
        ImGui::Separator();
    }
    ImGui::End();
#endif // USE_IMGUI
}

void WaveManager::transition_enlargement()
{
    clear_state = CLEAR_STATE::ENLARGEMENT;

    wait_timer = 0.5f;
    arrival_scale = { 6.0f, 6.0f };

    if (next_stage == STAGE_IDENTIFIER::BOSS) arrival_viewpoint = { 640.0f, 175.0f };
    else  arrival_viewpoint = { 640.0f, 360.0f };
}

void WaveManager::update_enlargement(float elapsed_time)
{
    current_stage = next_stage;

    wait_timer -= elapsed_time;
    wait_timer = (std::max)(wait_timer, 0.0f);

    if (wait_timer > 0.0f) return;

    float lerp_rate = 1.5f;
    viewpoint = Math::lerp(viewpoint, arrival_viewpoint, lerp_rate * elapsed_time);

    player_icon.arg.pos = viewpoint;
    map.arg.pos = viewpoint - stage_details[current_stage].position * map.arg.scale;

    // scaleの変更
    map.arg.scale = Math::lerp(map.arg.scale, arrival_scale, lerp_rate * elapsed_time);
    player_icon.arg.scale = DirectX::XMFLOAT2(0.4f, 0.4f) * map.arg.scale;

    if (Math::equal_check(map.arg.scale.x, arrival_scale.x, 0.1f)) { close = true; }
}


void WaveFile::load()
{
    std::filesystem::path path = file_name;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);
            o_archive(source);
        }
    }
    else
    {
        source.initialize();
    }
}

void WaveFile::save()
{
    std::filesystem::path path = file_name;
    path.replace_extension(".json");
    std::ofstream ifs(path);
    if (ifs)
    {
        cereal::JSONOutputArchive o_archive(ifs);
        o_archive(source);
    }
}

//---ここまで--//