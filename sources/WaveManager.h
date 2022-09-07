#pragma once

#include <cereal/cereal.hpp>

#include "EnemyManager.h"
#include "sprite_dissolve.h"
#include "sprite_batch.h"
#include "practical_entities.h"



//****************************************************************
//
// ウェーブを管理するクラス
//
//****************************************************************
class WaveManager final : public PracticalEntities
{
    //****************************************************************
    //
    // 構造体
    //
    //****************************************************************
public:
    enum class WaveState
    {
        Start, // ゲーム開始前
        Game,  // ゲーム中
        Clear, // ゲームクリア
    };

    //****************************************************************
    //
    //  関数
    //
    //****************************************************************
public:
    WaveManager() = default;
    ~WaveManager() = default;

    // 初期化
    void fInitialize(GraphicsPipeline& graphics_, AddBulletFunc Func_);
    // 更新
    void fUpdate(GraphicsPipeline& Graphics_,float elapsedTime_, AddBulletFunc Func_);
    //----------------------------------
    // TODO:藤岡が書いたところ
    //----------------------------------
    void render(ID3D11DeviceContext* dc, float elapsed_time);
    bool during_clear_performance() { return mWaveState == WaveState::Clear; }
    //---ここまで--//

    // 終了処理
    void fFinalize();
    // ウェーブ開始
    void fStartWave();


    //--------------------<ゲッター関数>--------------------//
    [[nodiscard]] EnemyManager* fGetEnemyManager();

    //--------------------<セッター関数>--------------------//

    //--------------------<ImGui>--------------------//
    void fGuiMenu();

private:

    void fClearUpdate(float elapsedTime_);

    //****************************************************************
    //
    //  変数
    //
    //****************************************************************
    // 現在のウェーブ
    int mCurrentWave{};

    // 最小の演出が終了したかどうか
    bool mStartGame{};

    EnemyManager mEnemyManager{};
    WaveState mWaveState;

    //--------------------<ImGui>--------------------//
    bool mOpenGui{};

    //----------------------------------
    // TODO:藤岡が書いたところ1
    //----------------------------------
    enum class CLEAR_STATE
    {
        REDUCTION,    // 縮小
        SELECTION,    // ステージ選択
        MOVE,         // 移動
        ENLARGEMENT,  // 拡大
    };
    CLEAR_STATE clear_state = CLEAR_STATE::REDUCTION;
    // reduction
    void transition_reduction();
    void update_reduction(float elapsed_time);
    // selection
    void transition_selection();
    void update_selection(float elapsed_time);
    // move
    void transition_move();
    void update_move(float elapsed_time);
    // enlargement
    void transition_enlargement();
    void update_enlargement(float elapsed_time);

    bool close = false;
    float wait_timer = 1.0f;

    DirectX::XMFLOAT2 middle_point;
    DirectX::XMFLOAT2 terminus_point;
    DirectX::XMFLOAT2 interpolated_point;
    bool can_moves[2] = { false, false };

    DirectX::XMFLOAT2 view_middle_point;

public:
    enum STAGE_IDENTIFIER /*S_到達数_左からの数*/
    {
        S_1_1,
        S_2_1,
        S_2_2,
        S_3_1,
        S_3_2,
        S_3_3,
        BOSS,

        STAGE_COUNT,
    };
private:
    struct StageDetails
    {
        enum class ROUTE
        {
            LEFT,
            RIGHT,
            UP,
            DOWN,
        };
        DirectX::XMFLOAT2 position = {}; // 各ステージの位置
        // 各ステージのルートとその行先
        std::map<ROUTE, STAGE_IDENTIFIER> journeys;
    };
    StageDetails stage_details[STAGE_IDENTIFIER::STAGE_COUNT];
    STAGE_IDENTIFIER current_stage      = STAGE_IDENTIFIER::S_1_1;
    STAGE_IDENTIFIER next_stage         = STAGE_IDENTIFIER::S_1_1;
    STAGE_IDENTIFIER candidate_stage    = STAGE_IDENTIFIER::S_1_1; // 次に選べるルート候補
    DirectX::XMFLOAT2 viewpoint         = {};
    DirectX::XMFLOAT2 arrival_viewpoint = {};
    DirectX::XMFLOAT2 arrival_scale     = {};

    struct SpriteArg
    {
        DirectX::XMFLOAT2 pos = {};
        DirectX::XMFLOAT2 scale = { 1,1 };
        DirectX::XMFLOAT2 pivot = {};
        DirectX::XMFLOAT4 color = { 1,1,1,1 };
        float angle = 0;
        DirectX::XMFLOAT2 texpos = {};
        DirectX::XMFLOAT2 texsize = {};
    };
    struct Icon
    {
        std::unique_ptr<SpriteDissolve> sprite = nullptr;
        float threshold = 10.0f;
        SpriteArg arg = {};
    };
    std::unique_ptr<SpriteDissolve> arrow_sprite = nullptr;
    struct Arrow
    {
        float threshold = 1.0f;
        SpriteArg arg = {};
    };
    Icon map;
    Icon player_icon;
    std::map<StageDetails::ROUTE, Arrow> arrows;

    StageDetails::ROUTE route_state = StageDetails::ROUTE::LEFT;

    static constexpr float CLEAR_WAIT_TIME = 5.0f;
    static constexpr float CLEAR_ANIMATION_WAIT_TIME = 2.0f;
    static constexpr float CLEAR_ANIMATION_FADE_WAIT_TIME = 4.0f;
    float clear_wait_timer = CLEAR_WAIT_TIME;
    bool clear_flg = false;

    struct ClearParameters
    {
        std::unique_ptr<SpriteDissolve> sprite_clear = nullptr;
        SpriteArg clear;
        float threshold = -0.5f;

        float timer     = 0;
        int frame_y     = 0;
        bool start_anim = false;

        bool se_play = false;

        void initialize()
        {
            clear.color.w = 0.0f;

            threshold  = -0.5f;

            timer      = 0;
            frame_y    = 0;
            start_anim = false;
            se_play    = false;
        }
    };
    ClearParameters clear_parameters;

    bool game_clear = false;
public:
    // getter
    const STAGE_IDENTIFIER& get_current_stage() { return current_stage; }
    const bool get_game_clear() const { return game_clear; }

    //---ここまで--//
};



class WaveFile
{
private:
    //--------<constructor/destructor>--------//
    WaveFile() {}
    ~WaveFile() {}
    //--------< 構造体 >--------//
    struct WaveSource
    {
        //--------< 変数 >--------//
        int stage_to_start = 0;
        //--------< 関数 >--------//
        void initialize()
        {
#if 0
            stage_to_start = 0;
#else
            stage_to_start = WaveManager::STAGE_IDENTIFIER::S_3_1;
#endif
        }
        // シリアライズ
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("stage_to_start", stage_to_start)
            );
        }
    };
    //--------< 変数 >--------//
    WaveSource source;
    const char* file_name = "./resources/Data/stage_to_start.json";
public:
    //--------< singleton >--------//
    static WaveFile& get_instance()
    {
        static WaveFile instance;
        return instance;
    }
    //--------< 関数 >--------//
    void load();
    void save();
    //--------<getter/setter>--------//
    // stage_to_start
    int get_stage_to_start() const { return source.stage_to_start; }
    void set_stage_to_start(int s) { source.stage_to_start = s; }
};