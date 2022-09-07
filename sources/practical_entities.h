#pragma once

#include "audio_manager.h"
#include "debug_figures.h"
#include "debug_2D.h"
#include "effect_manager.h"
#include "font.h"
#include "game_pad.h"
#include "graphics_pipeline.h"
#include "mouse.h"
#include "resource_manager.h"
#include "camera_shake.h"
#include "hit_stop.h"

class PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    PracticalEntities() {}
    virtual ~PracticalEntities() {}
protected:
    //--------< ä÷êî >--------//
    /*åƒÇ—èoÇµÇÕFrameworkÇÃÇ›*/
    void entities_initialize(GraphicsPipeline& graphics);
    /*åƒÇ—èoÇµÇÕFrameworkÇÃÇ›*/
    void entities_uninitialize(GraphicsPipeline& graphics);
    //--------< ïœêî >--------//
    static std::unique_ptr<ResourceManager> resource_manager;
    static std::unique_ptr<AudioManager> audio_manager;
    static std::unique_ptr<FontInstances> fonts;
    static std::unique_ptr<DebugFigures> debug_figure;
    static std::unique_ptr<Debug2D> debug_2D;
    static std::unique_ptr<Mouse> mouse;
    static std::unique_ptr<GamePad> game_pad;
    static std::unique_ptr<EffectManager> effect_manager;
    static std::unique_ptr<CameraShake> camera_shake;
    static std::unique_ptr<HitStop> hit_stop;
};