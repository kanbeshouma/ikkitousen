#pragma once
#include<memory>
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
#include"SpriteBatch.h"
#include"PlayerManager.h"

class SceneMultiGameClient : public Scene, public PracticalEntities
{
public:
    SceneMultiGameClient();
    ~SceneMultiGameClient() override;
public:
    ////----------初期化---------//
    void initialize(GraphicsPipeline& graphics) override;

    ////----------終了化----------//
    void uninitialize() override;

    ////----------- エフェクト解放----------//
    void effect_liberation(GraphicsPipeline& graphics) override;

    ////----------更新処理----------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;

    ////----------描画処理----------//
    void render(GraphicsPipeline& graphics, float elapsed_time) override;

    ////----------シャドウマップ登録----------//
    void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) override;
private:
    std::unique_ptr<CameraManager> cameraManager;
public:
    enum class CameraTypes
    {
        Game,
        Tunnel,
        Joint,
        BossPerformance,
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
        float bloom_extraction_threshold = 0.800f;
        float blur_convolution_intensity = 0.500f;

        float bloom_options[2];
    };
    std::unique_ptr<Constants<BloomConstants>> bloom_constants;
private:
    WaveManager mWaveManager{};
    std::unique_ptr<PlayerManager> player_manager{ nullptr };
    std::unique_ptr<SkyDome> sky_dome{ nullptr };
    //skydome
    float dimension{ 0.1f };
    // enemy_hp_gauge
    std::unique_ptr<EnemyHpGauge> enemy_hp_gauge{ nullptr };
    // boss_hp_gauge
    std::unique_ptr<BossHpGauge> boss_hp_gauge{ nullptr };
    // reticle
    std::unique_ptr<Reticle> reticle{ nullptr };
    // counter
    std::unique_ptr<Counter> wave{ nullptr };
    // option
    std::unique_ptr<Option> option{ nullptr };
    std::unique_ptr<MiniMap> minimap{ nullptr };
    // bgm切り替え用変数
    LastBoss::Mode old_last_boss_mode = LastBoss::Mode::None;
    LastBoss::Mode last_boss_mode = LastBoss::Mode::None;
    // スカイボックスの色切り替え用変数
    float purple_threshold = 0;
    float red_threshold = 0;
    // クリア演出
    bool during_clear = false;
    float tunnel_alpha = 0.0f;
    std::unique_ptr<Tunnel> tunnel{ nullptr };

    // ボスの変数
    bool mIsBossCamera{};
private:
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
    Element selecter1;
    Element selecter2;
    std::unique_ptr<SpriteBatch> sprite_selecter{ nullptr };
    //入力を許可するかどうか
    bool can_axis{ true };
    //入力してからの時間
    float axis_wait_timer{};
    //入力許可時間
    static constexpr float AXIS_WAIT_TIME = 0.2f;

    //---------ゲームオーバー--------//
    void GameOverAct(float elapsed_time);
    bool is_game_over{ false };

    float glow_vertical = {};

    // ゲームオーバー
    StepFontElement game_over_text;
    // タイトルにもどる
    StepFontElement back_title;
    // 再挑戦
    StepFontElement again;
    int game_over_state{ 0 };
    //ゲームオーバーの文字の後ろに出てるフレーム
    std::unique_ptr<SpriteBatch> sprite_back{ nullptr };
    //フレームのパラメータ
    Element game_over_pram;
    //画面を黒くする
    std::unique_ptr<SpriteBatch> brack_back{ nullptr };
    //画面黒くするパラメータ
    Element brack_back_pram;
    //黒くなり切ったかどうか
    bool is_set_black{ false };
    //ゲームオーバーの画像
    std::unique_ptr<SpriteBatch> game_over_sprite{ nullptr };
    //フレームのパラ
    Element game_over_sprite_pram;
    //ゲームオーバーのスプライトが描画させたかどうか
    bool is_game_over_sprite{ false };

    //---------ゲームクリア-----------//
    void GameClearAct(float elapsed_time, GraphicsPipeline& graphics);
    //イベントカメラのセット
    bool set_joint_camera{ false };
    bool is_game_clear{ false };
    // ゲームクリア
    StepFontElement game_clear_text;
private:
    bool slow{ false };
    float slow_timer{ 0.0f };
    float slow_rate{ 0.05f };
    //----------ゲームのスロウ判定----------//
    void JudgeSlow(float& elapsed_time);

    //-----------ボスの形態でのパラメータの再設定----------//
    void SetBossTypeGameParam();

    //----------ボスのイベントカメラの設定----------//
    void BossEventCamera();

    //----------スカイドームの色----------//
    void SetSkyDomeColor(float elapsed_time);

    //----------プレイヤーの更新処理----------//
    void PlayerManagerUpdate(GraphicsPipeline& graphics, float elapsed_time);

private:
    //-----TCP通信を管理するスレッド-----//
    std::thread tcp_thread;

    //-----UDP通信を管理するスレッド-----//
    std::thread udp_thread;

    //-----排他制御-----//
    static std::mutex mutex;

private:
    //-----TCPスレッドを終了するかのフラグ-----//
    static bool end_tcp_thread;

    //-----UDPスレッドを終了するかのフラグ-----//
    static bool end_udp_thread;

    //-----プレイヤーを追加するかどうか-----//
    static bool register_player;

    //-----追加するプレイヤーの番号-----//
    static int register_player_id;

    //-----ログアウトするプレイヤーのID-----//
    static std::vector<int> logout_id;

private:
    //----------通信関係(マルチスレッド)----------//
    //-----プレイヤーのTCP用のマルチスレッド-----//
    static void ReceiveTcpData();
private:
    //-----UDP用のマルチスレッド-----//
    static void ReceiveUdpData();

    ////-----データの種類の確認-----//
    //=======================
    //第1引数 : コマンド
    //第2引数 : 受信データ
    static void CheckDataCommand(char com, char* data);
private:
    //----------通信関係----------//

    ////----------クライアントがログインして来た時にプレイヤーを追加する-----------//
    void RegisterPlayer(GraphicsPipeline& graphics);

    ////-----ログアウトしたプレイヤーを削除する-----//
    void DeletePlayer();

    ////-----受信データを設定する-----//
    void SetReceiveData();
private:
    //-----受信データを入れる-----//
    static AllDataStruct receive_all_data;
};
