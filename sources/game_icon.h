#pragma once

#include <cereal/cereal.hpp>
#include "icon_base.h"
#include "practical_entities.h"
#include "number.h"

class GameFile
{
private:
    //--------<constructor/destructor>--------//
    GameFile() {}
    ~GameFile() {}
    //--------< �\���� >--------//
    struct GameSource
    {
        //--------< �ϐ� >--------//
        bool shake        = true;
        bool vibration    = true;
        //--------< �֐� >--------//
        void initialize()
        {
            shake       = true;
            vibration   = true;
        }
        // �V���A���C�Y
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("shake", shake),
                cereal::make_nvp("vibration", vibration)
            );
        }
    };
    //--------< �ϐ� >--------//
    GameSource source;
    const char* file_name = "./resources/Data/game.json";
public:
    //--------< singleton >--------//
    static GameFile& get_instance()
    {
        static GameFile instance;
        return instance;
    }
    //--------< �֐� >--------//
    void load();
    void save();
    //--------<getter/setter>--------//
    // shake
    bool get_shake() const { return source.shake; }
    void set_shake(bool s) { source.shake = s; }
    // vibration
    bool get_vibration() const { return source.vibration; }
    void set_vibration(bool v) { source.vibration = v; }
};

class GameIcon : public IconBase, PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    GameIcon(ID3D11Device* device);
    ~GameIcon() override;
    //--------< �֐� >--------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    void render(std::string gui, ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos) override;
    void vs_cursor(const DirectX::XMFLOAT2& cursor_pos) override;
private:
    void save_source();
    //--------< �萔 >--------//
    enum ChoicesType
    {
        SHAKE,
        VIBRATION,

        OPERATION, // ����m�F
    };
    static constexpr float INTERVAL = 0.1f;
    static const int BUTTON_COUNT = 2;
    //--------< �ϐ� >--------//
    FontElement shake;
    FontElement vibration;
    FontElement operation;
    FontElement operation_back;
    bool display_operation = false;
    std::unique_ptr<SpriteBatch> sprite_operation{ nullptr };
    std::unique_ptr<SpriteBatch> sprite_mask{ nullptr };
    //--button--//
    FontElement choices[BUTTON_COUNT][2]; // 0:ON 1:OFF
    Element selecter[BUTTON_COUNT][2]; // 0:L 1:R
    DirectX::XMFLOAT2 selecter_arrival_pos[BUTTON_COUNT][2]; // 0:L 1:R
    bool setup[BUTTON_COUNT];
};