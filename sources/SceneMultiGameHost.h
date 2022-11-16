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
#include"DebugConsole.h"
#include"NetWorkInformationStucture.h"

class SceneMultiGameHost : public Scene, public PracticalEntities
{
public:
    SceneMultiGameHost();
    ~SceneMultiGameHost();
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
    //-----ホストになるプレイヤーの番号-----//
    static const int PlayerPrivateObjectId = 0;
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

    //-----今のステージ-----//
    static WaveManager::STAGE_IDENTIFIER current_stage;

    //-----クライアントが選択したステージ-----//
    static std::vector<WaveManager::STAGE_IDENTIFIER> client_select_stage;


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

    //-----ゲームオーバーの時にタイトルを戻るを押した-----//
    bool game_over_select_title{ false };

    //-----再挑戦を選択した-----//
    bool game_over_trying_again{ false };

    //-----再挑戦選択数-----//
    int trying_again_count{};

    //-----ゲーム再挑戦を押したときの文字列-----//
    std::wstring game_over_idle = L"";

    StepFontElement game_over_select_idle;

    bool StepString(float elapsed_time, std::wstring full_text, StepFontElement& step_font_element,float speed = 1.0f, bool loop = false);


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
    void PlayerManagerUpdate(GraphicsPipeline& graphics,float elapsed_time);

    //---------プレイヤー関係の当たり判定----------//
    void PlayerManagerCollision(GraphicsPipeline& graphics, float elapsed_time);

    //-----リトライ時に再初期化をする関数-----//
    void RestartInitialize();

private:
    //-----ゲームを開始するかどうか(敵を出現させてもいいか)-----//
    bool is_start_game{ false };

    //ゲームを開始するときの変数
    float game_start_timer{ 0 };

    std::unique_ptr<SpriteDissolve> game_start_gauge{ nullptr };
    struct GameStartGauge
    {
        DirectX::XMFLOAT2 pos{ -13.9f,439.6f };
        DirectX::XMFLOAT2 scale{ 3.8f,2.9f };
        float threshold{ 1.0f };
    };
    //-----スプライトのパラメータ-----//
    GameStartGauge game_start_gauge_parm;

    std::wstring game_start_text = L"バックボタン長押しでゲームを開始";
    struct GameStartTxt
    {
        DirectX::XMFLOAT2 position{ 19.0f,445.1f };
        DirectX::XMFLOAT2 scale{ 0.5f,0.5f };
        float angle;
        DirectX::XMFLOAT4 color{ 1.0f,1.0f,0.0f,1.0f };
    };
    //-----フォントのパラメータ-----//
    GameStartTxt game_start_txt_parm;


    std::unique_ptr<SpriteBatch> controller_on__back_keys{ nullptr };
    Element controller_back_pram;

private:
    void StartGameUi(float elapsed_time);

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

    //-----追加したプレイヤーの色-----//
    static int register_player_color;

    //-----ログアウトするプレイヤーのID-----//
    static std::vector<int> logout_id;


private:
    //----------通信関係(マルチスレッド)----------//
    //-----プレイヤーのTCP用のマルチスレッド-----//
    static void ReceiveTcpData();

    ////------プレイヤーのログイン処理------//
    //===========================
    //第1引数 : 通信相手の番号
    static void Login(int client_id, char* data);

    ////-----プレイヤーのログアウト処理-----//
    //==============================
    //第1引数 : 受信データ
    static void Logout(char* data);

private:
    //-----UDP用のマルチスレッド-----//
    static void ReceiveUdpData();

    ////-----データの種類の確認-----//
    //=======================
    //第1引数 : コマンド
    //第2引数 : 受信データ
    static void CheckDataCommand(char com, char* data);

    ////-----データの種類の確認(プレイヤーのアクションデータ用)-----//
    //=======================
    //第1引数 : コマンド
    //第2引数 : 受信データ
    static void CheckPlayerActionCommand(char com, char* data);

private:
    //----------通信関係----------//

    ////----------クライアントがログインして来た時にプレイヤーを追加する-----------//
    void RegisterPlayer(GraphicsPipeline& graphics);

    ////-----ログアウトしたプレイヤーを削除する-----//
    void DeletePlayer();

    ////-----受信データを設定する-----//
    void SetReceiveData();

    //-----敵のダメージデータを設定-----//
    void SetEnemyDamageData(GraphicsPipeline& graphics);

    //-----敵の状態データを設定-----//
    void SetEnemyConditionData();

    //-----プレイヤーの体力のデータの設定-----//
    void ReceivePlayerHealthData();

    ////-----敵の受信データを削除する-----//
    void ClearEnemyReceiveData();

    ////-----敵のホスト権譲渡関係の処理-----//
    void TransferEnenyControlProcessing();

    ////-----敵のホスト権が帰ってきた時の処理-----//
    void ReturnEnemyControl();
private:
    //-----受信データを入れる-----//
    static PlayerAllDataStruct receive_all_data;

    //-----敵の受信データを入れる-----//
    static EnemyAllDataStruct receive_all_enemy_data;

    //-----敵のホスト権の譲渡リクエストを受けたかどうか-----//
    static bool transfer_enemy_host_request;

    //-----リクエストして来たプレイヤーの番号-----//
    static int transfer_enemy_request_id;

    //-----敵のホスト権の譲渡されたかどうか-----//
    static bool transfer_enemy_host_result;

    //-----敵のホスト権が帰って来たかどうか-----//
    static bool return_enemy_control;

    //-----再挑戦を選択した時に増える-----//
    static std::vector<int> select_trying_again;
};
