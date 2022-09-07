#include "practical_entities.h"

std::unique_ptr<ResourceManager> PracticalEntities::resource_manager = nullptr;
std::unique_ptr<AudioManager>    PracticalEntities::audio_manager    = nullptr;
std::unique_ptr<FontInstances>   PracticalEntities::fonts            = nullptr;
std::unique_ptr<DebugFigures>    PracticalEntities::debug_figure     = nullptr;
std::unique_ptr<Debug2D>         PracticalEntities::debug_2D         = nullptr;
std::unique_ptr<Mouse>           PracticalEntities::mouse            = nullptr;
std::unique_ptr<GamePad>         PracticalEntities::game_pad         = nullptr;
std::unique_ptr<EffectManager>   PracticalEntities::effect_manager   = nullptr;
std::unique_ptr<CameraShake>     PracticalEntities::camera_shake     = nullptr;
std::unique_ptr<HitStop>         PracticalEntities::hit_stop         = nullptr;

void PracticalEntities::entities_initialize(GraphicsPipeline& graphics)
{
    // resource_manager
    resource_manager = std::make_unique<ResourceManager>();
    // audio_manager
    audio_manager = std::make_unique<AudioManager>();
    // fonts
    fonts = std::make_unique<FontInstances>(graphics.get_device().Get());
    // debug_figure
    debug_figure = std::make_unique<DebugFigures>(graphics.get_device().Get());
    // debug_2D
    debug_2D = std::make_unique<Debug2D>(graphics.get_device().Get());
    // mouse
    mouse = std::make_unique<Mouse>();
    // gamepad
    game_pad = std::make_unique<GamePad>();
    // effect_manager
    effect_manager = std::make_unique<EffectManager>();
    effect_manager->initialize(graphics);
    // camera_shake
    camera_shake = std::make_unique<CameraShake>();
    // hit_stop
    hit_stop = std::make_unique<HitStop>();
}

void PracticalEntities::entities_uninitialize(GraphicsPipeline& graphics)
{
    // このクラスは各実体は一つしか持たないがデストラクタは継承した回数分呼ばれるので
    // ここで明示的に解放しないと二回目以降のデストラクタで落ちてしまう。
    if (resource_manager != nullptr) { resource_manager.reset(nullptr); }
    if (audio_manager != nullptr) { audio_manager.reset(nullptr); }
    if (fonts != nullptr) { fonts.reset(nullptr); }
    if (debug_figure != nullptr) { debug_figure.reset(nullptr); }
    if (debug_2D != nullptr) { debug_2D.reset(nullptr); }
    if (mouse != nullptr) { mouse.reset(nullptr); }
    if (game_pad != nullptr) { game_pad.reset(nullptr); }
    if (effect_manager != nullptr)
    {
        effect_manager->finalize();
        effect_manager.reset(nullptr);
    }
    if (camera_shake != nullptr) { camera_shake.reset(nullptr); }
    if (hit_stop != nullptr) { hit_stop.reset(nullptr); }
}