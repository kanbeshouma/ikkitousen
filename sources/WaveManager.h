#pragma once
#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include <cereal/cereal.hpp>

#include "EnemyManager.h"
#include "sprite_dissolve.h"
#include "sprite_batch.h"
#include "practical_entities.h"



//****************************************************************
//
// �E�F�[�u���Ǘ�����N���X
//
//****************************************************************
class WaveManager final : public PracticalEntities
{
    //****************************************************************
    //
    // �\����
    //
    //****************************************************************
public:
    enum class WaveState
    {
        Start, // �Q�[���J�n�O
        Game,  // �Q�[����
        Clear, // �Q�[���N���A
    };

    //****************************************************************
    //
    //  �֐�
    //
    //****************************************************************
public:
    WaveManager() = default;
    ~WaveManager() = default;

    // ������
    void fInitialize(GraphicsPipeline& graphics_, AddBulletFunc Func_);

    //-----�G�̃_���[�W�f�[�^��ݒ�-----//
    void fSetReceiveEnemyDamageData(EnemySendData::EnemyDamageData data, GraphicsPipeline& graphics_);

    //-----�G�̏�ԃf�[�^��ݒ�-----//
    void fSetReceiveEnemyConditionData(EnemySendData::EnemyConditionData data);

    //-----�ʏ펞�ƃ}���`�v���C�̎��ɌĂԍX�V����-----//
    void fUpdate(GraphicsPipeline& Graphics_,float elapsedTime_, AddBulletFunc Func_);

    //-----�}���`�v���C���ɃN���C�A���g�����ĂԍX�V����-----//
    void fClientUpdate(GraphicsPipeline& Graphics_, float elapsedTime_, AddBulletFunc Func_, EnemyAllDataStruct& receive_data);

    //----------------------------------
    // TODO:�������������Ƃ���
    //----------------------------------
    void render(ID3D11DeviceContext* dc, float elapsed_time);
    bool during_clear_performance() { return mWaveState == WaveState::Clear; }
    //---�����܂�--//

    // �I������
    void fFinalize();
    // �E�F�[�u�J�n
    void fStartWave();


    //--------------------<�Q�b�^�[�֐�>--------------------//
    [[nodiscard]] EnemyManager* fGetEnemyManager();

    //--------------------<�Z�b�^�[�֐�>--------------------//

    //--------------------<ImGui>--------------------//
    void fGuiMenu();

private:

    void fClearUpdate(float elapsedTime_);

    //****************************************************************
    //
    //  �ϐ�
    //
    //****************************************************************
    // ���݂̃E�F�[�u
    int mCurrentWave{};

    // �ŏ��̉��o���I���������ǂ���
    bool mStartGame{};

    EnemyManager mEnemyManager{};
    WaveState mWaveState;

    //--------------------<ImGui>--------------------//
    bool mOpenGui{};

    //----------------------------------
    // TODO:�������������Ƃ���1
    //----------------------------------
    enum class CLEAR_STATE
    {
        REDUCTION,    // �k��
        SELECTION,    // �X�e�[�W�I��
        MOVE,         // �ړ�
        ENLARGEMENT,  // �g��
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
    enum STAGE_IDENTIFIER /*S_���B��_������̐�*/
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
        DirectX::XMFLOAT2 position = {}; // �e�X�e�[�W�̈ʒu
        // �e�X�e�[�W�̃��[�g�Ƃ��̍s��
        std::map<ROUTE, STAGE_IDENTIFIER> journeys;
    };
    StageDetails stage_details[STAGE_IDENTIFIER::STAGE_COUNT];
    STAGE_IDENTIFIER current_stage      = STAGE_IDENTIFIER::S_1_1;
    STAGE_IDENTIFIER next_stage         = STAGE_IDENTIFIER::S_1_1;
    STAGE_IDENTIFIER candidate_stage    = STAGE_IDENTIFIER::S_1_1; // ���ɑI�ׂ郋�[�g���
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

    //---�����܂�--//
};



class WaveFile
{
private:
    //--------<constructor/destructor>--------//
    WaveFile() {}
    ~WaveFile() {}
    //--------< �\���� >--------//
    struct WaveSource
    {
        //--------< �ϐ� >--------//
        int stage_to_start = 0;
        //--------< �֐� >--------//
        void initialize()
        {
#if 0
            stage_to_start = 0;
#else
            stage_to_start = WaveManager::STAGE_IDENTIFIER::S_3_1;
#endif
        }
        // �V���A���C�Y
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("stage_to_start", stage_to_start)
            );
        }
    };
    //--------< �ϐ� >--------//
    WaveSource source;
    const char* file_name = "./resources/Data/stage_to_start.json";
public:
    //--------< singleton >--------//
    static WaveFile& get_instance()
    {
        static WaveFile instance;
        return instance;
    }
    //--------< �֐� >--------//
    void load();
    void save();
    //--------<getter/setter>--------//
    // stage_to_start
    int get_stage_to_start() const { return source.stage_to_start; }
    void set_stage_to_start(int s) { source.stage_to_start = s; }
};