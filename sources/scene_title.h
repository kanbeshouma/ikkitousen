#pragma once

#include <memory>

#include "scene.h"
#include "sprite_batch.h"
#include "practical_entities.h"
#include "camera.h"
#include "CameraManager.h"
#include "post_effect.h"
#include "bloom.h"
#include "practical_entities.h"
#include "skinned_mesh.h"
#include "effect.h"
#include "point_light.h"
#include "constants.h"
#include "Player.h"
#include "SkyDome.h"
#include "shadow_map.h"
#include "point_light.h"

class SceneTitle : public Scene , public PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    SceneTitle() {}
    ~SceneTitle() override {}
    //--------< 関数 >--------//
    //初期化
    void initialize(GraphicsPipeline& graphics) override;
    //終了化
    void uninitialize() override;
    // エフェクト解放
    void effect_liberation(GraphicsPipeline& graphics) override;
    //更新処理
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    //描画処理
    void render(GraphicsPipeline& graphics, float elapsed_time) override;
    //シャドウマップ登録
    void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) override;
private:
    //----<3D関連>----//
    std::unique_ptr<CameraManager> cameraManager;
public:
    enum class CameraTypes
    {
        Title,
        Joint,
    };
private:
    // shadowmap
    std::unique_ptr<ShadowMap> shadow_map;
    // post effect
    std::unique_ptr<PostEffect> post_effect;
    // bloom
    std::unique_ptr<Bloom> bloom_effect;
    struct BloomConstants
    {
        float bloom_extraction_threshold = 1.5f;
        float blur_convolution_intensity = 6.5f;

        float bloom_options[2];
    };
    std::unique_ptr<Constants<BloomConstants>> bloom_constants;
    std::unique_ptr<Player> player{ nullptr };
    std::unique_ptr<SkyDome> sky_dome{ nullptr };
    std::shared_ptr<SkinnedMesh> title_stage_model{ nullptr };

    struct TitleStageParm
    {
        DirectX::XMFLOAT3 pos{};
        DirectX::XMFLOAT3 angle{0,-3.14f,0};
        DirectX::XMFLOAT3 scale{0.06f,0.06f,0.06f};
    };
    TitleStageParm title_stage_parm;

    //--------< 変数 >--------//
    struct StepFontElement
    {
        std::wstring s = L"";
        DirectX::XMFLOAT2 position{};
        DirectX::XMFLOAT2 scale{ 1, 1 };
        DirectX::XMFLOAT4 color{ 1,1,1,1 };
        float angle{};
        DirectX::XMFLOAT2 length{};

        // step string
        float timer = 0;
        int step = 0;
        int index = 0;
    };
    StepFontElement beginning;    // 初めから
    StepFontElement succession;   // 続きから
    StepFontElement exit;         // ゲーム終了
    StepFontElement now_loading;  // ロード中

    bool step_string(float elapsed_time, std::wstring full_text, StepFontElement& step_font_element,
        float speed = 1.0f, bool loop = false);

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
    Element flash;
    std::unique_ptr<SpriteBatch> sprite_flash{ nullptr };
    Element selecter1;
    Element selecter2;
    std::unique_ptr<SpriteBatch> sprite_selecter{ nullptr };
    DirectX::XMFLOAT2 arrival_pos1{};
    DirectX::XMFLOAT2 arrival_pos2{};

    int state = 0;
    int have_tutorial_state = -1; // -1:チュートリアルデータなし 0:チュートリアルあり 1:チュートリアルなし

    //--tutorial tab--//
    struct TutorialTabParameters
    {
        bool display = false;
        DirectX::XMFLOAT2 arrival_posL{};
        DirectX::XMFLOAT2 arrival_posR{};
        Element selecterL;
        Element selecterR;
        StepFontElement headline;
        StepFontElement back;
        StepFontElement yes;
        StepFontElement no;

        Element frame;
        std::unique_ptr<SpriteBatch> sprite_frame{ nullptr };
    };
    TutorialTabParameters tutorial_tab;

    struct LogoParameters
    {
        std::unique_ptr<SpriteBatch> sprite_logo{ nullptr };
        Element logo;
        std::unique_ptr<SpriteBatch> sprite_animation{ nullptr };
        Element animation;
        float timer = 0;
        int frame_y = 0;
        bool start_anim = false;
        float reset_timer = 0.0f;

        float glow_horizon = 0;
    };
    LogoParameters logo_parameters;

    //--slashing post effect--//
    static constexpr float SLASHING_MAX = 0.06f;
    float slashing_power = 0.0f;
    float slashing_wait_timer = 0.0f;

    std::unique_ptr<PointLights> point_lights;
    std::unique_ptr<Effect> fire_effect1;
    std::unique_ptr<Effect> fire_effect2;
    DirectX::XMFLOAT3 fire_pos_1{ 4.3f,3.3f,0.9f };
    DirectX::XMFLOAT3 fire_pos_2{ -4.5f,3.3f,0.9f };

    static constexpr float AXIS_WAIT_TIME = 0.2f;
    bool can_axis = true;
    float axis_wait_timer = 0;

    bool can_play_se[3] = { false, false, false };

    float glow_vertical = {};

    bool has_stageNo_json = false;

    // コンフィグ
    Element config_a;
    std::unique_ptr<SpriteBatch> sprite_config_a{ nullptr };

    Element config_b;
    Element config_b2;
    std::unique_ptr<SpriteBatch> sprite_config_b{ nullptr };
    StepFontElement config_b_font;
    StepFontElement config_b_font2;

    //--------< マルチスレッド >--------//
    static void loading_thread(ID3D11Device* device);
    static bool is_load_ready;
};