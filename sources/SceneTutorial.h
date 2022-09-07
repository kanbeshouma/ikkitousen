#pragma once
#include<vector>
#include "Scene.h"
#include "camera.h"
#include "CameraManager.h"
#include "shadow_map.h"
#include "post_effect.h"
#include "bloom.h"
#include "practical_entities.h"
#include "sprite_batch.h"
#include "skinned_mesh.h"
#include "effect.h"
#include "point_light.h"
#include "husk_particles.h"
#include "constants.h"
#include "imgui_include.h"
#include"WaveManager.h"
#include "mini_map.h"
#include"EnemyManager.h"
#include"Player.h"
#include"SkyDome.h"
#include "enemy_hp_gauge.h"
#include "reticle.h"
#include "counter.h"
#include "scene_option.h"
#include "BulletManager.h"
#include "tunnel.h"
#include "sprite_dissolve.h"

//コントローラーの画像のボタンのプリセットbit演算では_を最後に付ける
enum class BottunPriset : uint16_t
{
    A_ = (1 << 0),
    B_ = (1 << 1),
    X_ = (1 << 2),
    Y_ = (1 << 3),
    RB_ = (1 << 4),
    RT_ = (1 << 5),
    LB_ = (1 << 6),
    LT_ = (1 << 7),
    RightStick_ = (1 << 8),
    LeftStick_ = (1 << 9),
    Cross_ = (1 << 10),
    Menu_ = (1 << 11),
    Back_ = (1 << 12),

    MoveTutorialPriset = RightStick_ | LeftStick_,
    AvoidanceTutorialPriset = RB_ | RT_ | LeftStick_,
    LockOnTutorialPriset = LT_,
    AttackTutorialPriset = B_,
    BehindAvoidanceTutorialPriset = RB_ | RT_,
    ChainAttackTutorialPriset = LB_,
    AwaikingTutorialPriset = A_,
};

inline bool operator&(BottunPriset lhs, BottunPriset rhs)
{
    return static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs);
}

class TutorialScene : public Scene, public PracticalEntities
{
public:
    TutorialScene() {}
    ~TutorialScene() override {}
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
    void TutorialUpdate(GraphicsPipeline& graphics, float elapsed_time);
    //チュートリアルの文字とかを描画する
    void TutorialRender(GraphicsPipeline& graphics, float elapsed_time);
private:
    //--------< 変数 >--------//
    // camera
    //std::unique_ptr<Camera> camera;
    std::unique_ptr<CameraManager> cameraManager;
public:
    enum class CameraTypes
    {
        Game,
        Tunnel,
        Joint,
    };

    bool set_joint_camera{ false };
    //1が最初で大きくなっていくようにする
    enum class TutorialState
    {
        //移動
        MoveTutorial = 1,
        //回避(通常)
        AvoidanceTutorial,
        //ロックオン
        LockOnTutorial,
        //攻撃
        AttackTutorial,
        //回り込み回避
        BehindAvoidanceTutorial,
        //チェイン攻撃
        ChainAttackTutorial,
        //覚醒
        AwaikingTutorial,
        //自由時間
        FreePractice,
    };

    TutorialState tutorial_state{ TutorialState::MoveTutorial };
private:
    // shadowmap
    std::unique_ptr<ShadowMap> shadow_map;
    // post effect
    std::unique_ptr<PostEffect> post_effect;
    // bloom
    std::unique_ptr<Bloom> bloom_effect;
    struct BloomConstants
    {
        float bloom_extraction_threshold = 0.800f;
        float blur_convolution_intensity = 0.500f;

        float bloom_options[2];
    };
    std::unique_ptr<Constants<BloomConstants>> bloom_constants;
    //// スキンメッシュの実体
    //std::shared_ptr<SkinnedMesh> sky_dome;
    //****************************************************************
    //
    // スピック開始後追加した変数
    //
    //****************************************************************
    //EnemyManager mEnemyManager{};
    WaveManager mWaveManager{};
    std::unique_ptr<Player> player{ nullptr };
    std::unique_ptr<SkyDome> sky_dome{ nullptr };
    //skydome
    float dimension{ 0.1f };
    // enemy_hp_gauge
    std::unique_ptr<EnemyHpGauge> enemy_hp_gauge{ nullptr };
    // reticle
    std::unique_ptr<Reticle> reticle{ nullptr };
    // counter
    std::unique_ptr<Counter> wave{ nullptr };
    // option
    std::unique_ptr<Option> option{ nullptr };
    std::unique_ptr<MiniMap> minimap{ nullptr };

    bool is_next{ false };
    //次のチュートリアルに行くまでの時間
    float Judea_timer{ 0 };
    // クリア演出
    bool during_clear = false;
    float tunnel_alpha = 0.0f;
    std::unique_ptr<Tunnel> tunnel{ nullptr };
private:
    //チュートリアルのテキスト
    //説明文用の構造体
    struct StepFontElement
    {
        std::wstring s = L"";
        DirectX::XMFLOAT2 position{};
        DirectX::XMFLOAT2 scale{ 0.7f, 0.7f };
        DirectX::XMFLOAT4 color{ 1.0f,1.0f,1.0f,1.0f };
        float angle{};
        DirectX::XMFLOAT2 length{};
        //説明文のテキスト
        std::wstring tutorial_text=L"UI";

        // step string
        float timer = 0;
        int step = 0;
        int index = 0;
        //スピード
        float speed{ 25.0f };
    };
    StepFontElement tutorial_text_element[8];
    //チュートリアルの説明の最後まで表示されたかどうか
    bool end_tutorial_text{ false };
    float end_tutorial_text_timer{ 0.0f };
    //画像のチュートリアルの時のBボタンの補完
    float b_bottun_rate{ 0.0f };
    bool b_button_change{ false };
    //同じチュートリアル中にテキストを変えたらtrueになる
    bool change_text{ false };

    bool StepString(float elapsed_time, StepFontElement& step_font_element, bool loop = false);
    bool is_end_text{ false };

    struct TutorialCheckText
    {
        DirectX::XMFLOAT2 position{ 103.0f,332.3f };
        DirectX::XMFLOAT2 scale{ 0.7f,0.7f };
        float angle;
        DirectX::XMFLOAT4 color{ 1.0f,1.0f,1.0f,1.0f };
    };
    TutorialCheckText tutorial_check_text_parm;
    //ｔチェックボックスのテキスト
    std::wstring tutorial_check_text = L"UI";
private:
    //チェックボックスの画像
    struct CheckMarkParm
    {
        DirectX::XMFLOAT2 pos{ 16.1f,329.5f };
        DirectX::XMFLOAT2 scale{ 0.2f,0.2f };
        float threshold{1.0f};
        bool is_threshold{ false };
    };
    CheckMarkParm check_mark_parm;
    std::unique_ptr<SpriteDissolve> check_mark{ nullptr };
    std::unique_ptr<SpriteBatch> check_box{ nullptr };
    //別枠で画像を表示してる時にtrueになる
    bool sprite_tutorial{ false };
    //画像のチュートリアルが終わったらtrue
    bool end_sprite_tutorial{ false };
    //画像チュートリアル用のタイマー
    float sprite_tutorial_timer{ 0.0f };
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

    //画面を黒くする
    std::unique_ptr<SpriteBatch> brack_back{ nullptr };
    //画面黒くするパラメータ
    Element brack_back_pram;
    //画像のチュートリアルの後ろに出てるフレーム
    std::unique_ptr<SpriteBatch> sprite_frame{ nullptr };
    Element sprite_frame_parm;
    //ゲージをさす矢印
    std::unique_ptr<SpriteBatch> arrow_mark{ nullptr };
    //ゲージをさす矢印のパラメータ
    Element arrow_mark_pram;
    float arrow_rate{ 0 };
    bool arrow_move_change{ false };

    //画像のチュートリアルのジャスト回避
    std::unique_ptr<SpriteBatch> just_avoidance{ nullptr };
    Element just_avoidance_parm;
    //画像のチュートリアルの覚醒
    std::unique_ptr<SpriteBatch> awaiking_chain{ nullptr };
    Element awaiking_chain_parm;

    StepFontElement sprite_tutorial_text;
    StepFontElement tutorial_count_text;
private:
    //シーン変更するときの変数
    float change_scene_timer{ 0 };
    std::unique_ptr<SpriteDissolve> change_scene_gauge{ nullptr };
    struct ChangeSceneGauge
    {
        DirectX::XMFLOAT2 pos{ -13.9f,439.6f };
        DirectX::XMFLOAT2 scale{ 3.8f,2.9f };
        float threshold{ 1.0f };
    };
    ChangeSceneGauge change_gauge_parm;
    std::wstring tutorial_skip_text = L"バックボタン長押しでチュートリアルスキップ";
    struct ChangeSceneTxt
    {
        DirectX::XMFLOAT2 position{ 19.0f,445.1f };
        DirectX::XMFLOAT2 scale{ 0.5f,0.5f };
        float angle;
        DirectX::XMFLOAT4 color{ 1.0f,1.0f,0.0f,1.0f };
    };
    ChangeSceneTxt change_scene_txt;

private:
    float glow_vertical = {};

    //文字のフレーム
    std::unique_ptr<SpriteBatch> sprite_tutorial_frame{ nullptr };
    //フレームのパラメータ
    Element frame_pram;
private:
    //コントローラーのボタンの画像
    enum ControllerSprite
    {
        A,//Aボタン
        B,//Bボタン
        X,//Xボタン
        Y,//Yボタン
        RT,//RTボタン
        RB,//RBボタン
        LT,//LTボタン
        LB,//LBボタン
        RightStick,//RightStickボタン
        LeftStick,//LeftStickボタン
        Cross,//十字ボタン
        Menu,//メニューボタン
        Back,//バックボタン
        Back2,//バックボタン

        End
    };
    BottunPriset button_priset = BottunPriset::MoveTutorialPriset;
    //コントローラーの画像
    std::unique_ptr<SpriteBatch> controller_base{ nullptr };
    std::unique_ptr<SpriteBatch> controller_keys[ControllerSprite::End]{ nullptr };
    std::unique_ptr<SpriteBatch> controller_on_keys[ControllerSprite::End]{ nullptr };
    //コントローラーの画像のパラメータ
    Element controller_pram;
    Element controller_back_pram;
    Element controller_b_pram;
    Element controller_back_button_pram;
    float back_button_rate{ 0 };
    bool back_button_move_change{ false };

private:
    //スタンしている敵がいなかったら増えていく
    float is_stun_timer{ 0 };
    bool reset_text{ false };
private:
    bool slow{ false };
    float slow_timer{ 0.0f };
    float slow_rate{ 0.05f };
private:
    //フリーの練習かどうか
    bool free_practice{ false };
};
