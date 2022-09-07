#pragma once

#include <DirectXMath.h>
#include "graphics_pipeline.h"
#include "sprite_batch.h"
#include "practical_entities.h"
#include "transition_icon.h"
#include "volume_icon.h"
#include "game_icon.h"

class Option : public PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    Option(GraphicsPipeline& graphics);
    ~Option() override {}
    //--------< ä÷êî >--------//
    //èâä˙âªèàóù
    void initialize();
    //çXêVèàóù
    void update(GraphicsPipeline& graphics, float elapsed_time);
    //ï`âÊèàóù
    void render(GraphicsPipeline& graphics, float elapsed_time);
    //--------<getter/setter>--------//
    static bool get_validity() { return validity; }
    static void set_validity(bool v) { validity = v; }
    static bool get_switching() { return switching; }
    static void set_switching(bool s) { switching = s; }
    static bool get_home_disabled() { return home_disabled; }
    static void set_home_disabled(bool d) { home_disabled = d; }
private:
    //--------< íËêî >--------//
    enum class IconType
    {
        VOLUME,
        GAME,
        TRANSITION,

        ICON_COUNT,
    };
    //--------< ïœêî >--------//
    struct Element
    {
        DirectX::XMFLOAT2 position{};
        DirectX::XMFLOAT2 scale{ 1, 1 };
        DirectX::XMFLOAT2 pivot{};
        DirectX::XMFLOAT4 color{ 1,1,1,1 };
        float angle{};
        DirectX::XMFLOAT2 texpos{};
        DirectX::XMFLOAT2 texsize{};
    };
    Element back;
    std::unique_ptr<SpriteBatch> sprite_back{ nullptr };
    Element tab;
    std::unique_ptr<SpriteBatch> sprite_tab{ nullptr };
    Element frame;
    DirectX::XMFLOAT2 frame_arrival_pos{};
    DirectX::XMFLOAT2 frame_arrival_scale{};
    std::unique_ptr<SpriteBatch> sprite_frame{ nullptr };

    std::map<IconType, Element> icon_elements;
    std::map<IconType, std::unique_ptr<SpriteBatch>> icon_sprites;
    std::map<IconType, std::unique_ptr<IconBase>> icon_map;

    IconType state = IconType::VOLUME;

    static bool validity;
    static bool switching;
    static bool home_disabled;

    DirectX::XMFLOAT2 add_position{};
    DirectX::XMFLOAT2 tab_add_position{};

    Element cursor{};
    DirectX::XMFLOAT2 cursor_velocity{};
    std::unique_ptr<SpriteBatch> sprite_cursor{ nullptr };

    float glow_vertical = {};
};