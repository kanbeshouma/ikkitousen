#pragma once

#include <cereal/cereal.hpp>
#include "icon_base.h"
#include "practical_entities.h"
#include "number.h"

class VolumeFile
{
private:
    //--------<constructor/destructor>--------//
    VolumeFile() {}
    ~VolumeFile() {}
    //--------< 構造体 >--------//
    struct VolumeSource
    {
        //--------< 変数 >--------//
        float master_volume = 0.5f;
        float bgm_volume = 0.5f;
        float se_volume = 0.5f;
        //--------< 関数 >--------//
        void initialize()
        {
            master_volume = 0.5f;
            bgm_volume = 0.5f;
            se_volume = 0.5f;
        }
        // シリアライズ
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("master", master_volume),
                cereal::make_nvp("bgm", bgm_volume),
                cereal::make_nvp("se", se_volume));
        }
    };
    //--------< 変数 >--------//
    VolumeSource source;
    const char* file_name = "./resources/Data/volume.json";
public:
    //--------< singleton >--------//
    static VolumeFile& get_instance()
    {
        static VolumeFile instance;
        return instance;
    }
    //--------< 関数 >--------//
    void load();
    void save();
    //--------<getter/setter>--------//
    // master
    float get_master_volume() const { return source.master_volume; }
    void set_master_volume(float v) { source.master_volume = v; }
    // bgm
    float get_bgm_volume() const { return source.bgm_volume; }
    void set_bgm_volume(float v) { source.bgm_volume = v; }
    // se
    float get_se_volume() const { return source.se_volume; }
    void set_se_volume(float v) { source.se_volume = v; }
};

class VolumeIcon : public IconBase, PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    VolumeIcon(ID3D11Device* device);
    ~VolumeIcon() override;
    //--------< 関数 >--------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    void render(std::string gui, ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos) override;
    void vs_cursor(const DirectX::XMFLOAT2& cursor_pos) override;
private:
    void save_volumes();
    //--------< 定数 >--------//
    enum BarType
    {
        MASTER,
        BGM,
        SE,
    };
    static const int MAX_SCALE_COUNT = 20;
    static const int BAR_COUNT = 3;
    static constexpr float INTERVAL = 0.1f;
    //--------< 変数 >--------//
    FontElement master;
    FontElement bgm;
    FontElement se;

    std::map<BarType, std::unique_ptr<Number>> volume_numbers;

    float interval_LX = 0;

    std::vector<Element> scales[BAR_COUNT];
    std::vector<Element> shell_scales[BAR_COUNT];
    std::unique_ptr<SpriteBatch> sprite_scale = nullptr;
};