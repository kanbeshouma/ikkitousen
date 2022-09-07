#include <thread>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#include "scene_loading.h"
#include "scene_manager.h"
#include "scene_game.h"

void SceneLoading::initialize(GraphicsPipeline& graphics)
{
    // sprite_batch
    load_back = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\load\\load_back.png", 1);
    load_text = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\load\\load_text.png", 1);
    load_icon = std::make_unique<SpriteBatch>(graphics.get_device().Get(), L".\\resources\\Sprites\\load\\load_icon.png", 1);

    loadTextWidth = 400;
    loadIconPosition = { 0,0 };


    //スレッド開始
    std::thread thread(loading_thread, &graphics, this);
    //スレッドの管理を放棄
    thread.detach();
}

void SceneLoading::uninitialize()
{
}

void SceneLoading::update(GraphicsPipeline& graphics, float elapsed_time)
{
    static float bgm_volume = 2.0f;
    static float se_volume = 2.0f;

    audio_manager->set_all_volume_bgm(bgm_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_bgm_volume());
    audio_manager->set_all_volume_se(se_volume * VolumeFile::get_instance().get_master_volume() * VolumeFile::get_instance().get_se_volume());


    // 前シーンのuninitialize等でエフェクトを止めると更新がかからない場合があるのでここで更新する
    effect_manager->update(elapsed_time);


    static float textTime{ 0 };

    textTime += elapsed_time;
    if(textTime > 1.0f)
    {
        textTime = 0;
        loadTextWidth += 75;
        if(loadTextWidth > 650)
        {
            loadTextWidth = 400;
        }
    }

    static float iconTime{ 0 };

    iconTime += elapsed_time;
    if(iconTime > 0.1f)
    {
        iconTime = 0.0f;
        loadIconPosition.x += 512;
        if(loadIconPosition.x > 3070 && loadIconPosition.y < 2500)//最終段じゃない時
        {
            loadIconPosition.x = 0;
            loadIconPosition.y += 512;
        }
        else if(loadIconPosition.x > 1600 && loadIconPosition.y > 2500)
        {
            loadIconPosition = { 0,0 };
        }
    }

    // シーンを切り替える
    if (next_scene->is_ready())
    {
        SceneManager::scene_switching(next_scene);
        //next_scene->set_ready(false);
    }
}

void SceneLoading::render(GraphicsPipeline& graphics, float elapsed_time)
{
    graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
    load_back->begin(graphics.get_dc().Get());
    load_back->render(graphics.get_dc().Get(), { 0,0 }, { 1, 1 });
    load_back->end(graphics.get_dc().Get());

    load_text->begin(graphics.get_dc().Get());
    load_text->render(graphics.get_dc().Get(), { 50,625 }, { 0.75, 0.75 },
        {0,0},{1,1,1,1},0,{0,0},{loadTextWidth,100});
    load_text->end(graphics.get_dc().Get());

    load_icon->begin(graphics.get_dc().Get());
    load_icon->render(graphics.get_dc().Get(), { 1050,450 }, { 0.5, 0.5 },
        {0,0},{1,1,1,1},0,loadIconPosition,{512,512});
    load_icon->end(graphics.get_dc().Get());

}

void SceneLoading::loading_thread(GraphicsPipeline* graphics, SceneLoading* scene)
{
    // 次のシーンの初期化を行う
    scene->next_scene->initialize(*graphics);
    // 次のシーンの準備完了設定
    scene->next_scene->set_ready(true);
}