#include "scene_manager.h"
#include "user.h"

Scene* SceneManager::storage_scene = nullptr;
DISSOLVE_TYPE SceneManager::dissolve_type = DISSOLVE_TYPE::DOT;
float SceneManager::dissolve_speed = 1.0f;

SceneManager::SceneManager(GraphicsPipeline& graphics)
{
    SpriteDissolve* sprite_dissolve;
    // TYPE1
    sprite_dissolve = new SpriteDissolve(graphics.get_device().Get(), L".\\resources\\Sprites\\mask\\black_mask.png",
        L".\\resources\\Sprites\\mask\\PushOver.png", 1);
    dissolve_sprites.insert(std::make_pair(DISSOLVE_TYPE::TYPE1, sprite_dissolve));
    // TYPE2
    sprite_dissolve = new SpriteDissolve(graphics.get_device().Get(), L".\\resources\\Sprites\\mask\\black_mask.png",
        L".\\resources\\Sprites\\mask\\MoveUp.png", 1);
    dissolve_sprites.insert(std::make_pair(DISSOLVE_TYPE::TYPE2, sprite_dissolve));
    // DOT
    sprite_dissolve = new SpriteDissolve(graphics.get_device().Get(), L".\\resources\\Sprites\\load\\load_back.png",
        L".\\resources\\Sprites\\mask\\dot.png", 1);
    dissolve_sprites.insert(std::make_pair(DISSOLVE_TYPE::DOT, sprite_dissolve));
    // HORIZON
    sprite_dissolve = new SpriteDissolve(graphics.get_device().Get(), L".\\resources\\Sprites\\load\\load_back.png",
        L".\\resources\\Sprites\\mask\\horizon1.png", 1);
    dissolve_sprites.insert(std::make_pair(DISSOLVE_TYPE::HORIZON, sprite_dissolve));
}

SceneManager::~SceneManager()
{
    for (auto& p : dissolve_sprites)
    {
        SpriteDissolve* dissolve_sprite = p.second;
        safe_delete(dissolve_sprite);
    }
}

void SceneManager::update(GraphicsPipeline& graphics, float elapsed_time)
{
    if (storage_scene != nullptr && dissolve_state == 0)
    {
        /*フェードアウトへ移行*/
        dissolve_state = 1;
    }
    // ディゾルブ
    switch (dissolve_state)
    {
    case 0: /*通常状態*/
        break;
    case 1: /*フェードアウト関連*/
        // シーン切り替わり時の1フレームはelapsed_timeが暴走するので初期化
        elapsed_time = 0;
        ++dissolve_state;
        /*fallthrough*/
    case 2: /*フェードアウト*/
        threshold -= dissolve_speed * elapsed_time;
        if(threshold <= 0.0f)
        {
            threshold = 0.0f;
            next_scene = storage_scene;
            storage_scene = nullptr;
        }
        break;
    case 3: /*フェードイン関連*/
        // シーン切り替わり時の1フレームはelapsed_timeが暴走するので初期化
        elapsed_time = 0;

        ++dissolve_state;
        /*fallthrough*/
    case 4: /*フェードイン*/
        // シーンの更新処理(モデルの姿勢やカメラなどを計算するための処理)
        current_scene->update(graphics, elapsed_time);
        mouse->operation_disablement();
        game_pad->operation_disablement();

        threshold += dissolve_speed * elapsed_time;
        if (threshold >= 1.0f)
        {
            threshold = 1.0f;
            /*通常状態へ移行*/
            dissolve_state = 0;
        }
        break;
    }
    // シーン切り替え処理
    if (next_scene != nullptr)
    {
        change_scene(graphics, next_scene);
        next_scene = nullptr;
    }
    // シーンの更新処理
    if (current_scene != nullptr && dissolve_state == 0)
    {
        current_scene->update(graphics, elapsed_time);
    }
}

void SceneManager::render(GraphicsPipeline& graphics, float elapsed_time)
{
    if (current_scene != nullptr)
    {
        current_scene->render(graphics, elapsed_time);
    }
    // ディゾルブ
    if (dissolve_sprites.count(dissolve_type))
    {
        // 描画ステート設定
        graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
        dissolve_sprites.at(dissolve_type)->begin(graphics.get_dc().Get());
        dissolve_sprites.at(dissolve_type)->render(graphics.get_dc().Get(), { 0,0 }, { 1,1 }, threshold);
        dissolve_sprites.at(dissolve_type)->end(graphics.get_dc().Get());
    }
}


void SceneManager::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
    if (current_scene != nullptr)
    {
        current_scene->register_shadowmap(graphics, elapsed_time);
    }
}

void SceneManager::clear(GraphicsPipeline& graphics)
{
    if (current_scene != nullptr)
    {
        current_scene->effect_liberation(graphics);
        current_scene->uninitialize();
        delete current_scene;
        current_scene = nullptr;
    }
}

void SceneManager::change_scene(GraphicsPipeline& graphics, Scene* s)
{
    clear(graphics);
    current_scene = s;
    // 未初期化の場合は初期化する
    if (!current_scene->is_ready())
    {
        current_scene->initialize(graphics);
    }
    /*フェードインへ移行*/
    dissolve_state = 3;
}