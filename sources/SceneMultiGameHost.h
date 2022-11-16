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
    ////----------������---------//
    void initialize(GraphicsPipeline& graphics) override;

    ////----------�I����----------//
    void uninitialize() override;

    ////----------- �G�t�F�N�g���----------//
    void effect_liberation(GraphicsPipeline& graphics) override;

    ////----------�X�V����----------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;

    ////----------�`�揈��----------//
    void render(GraphicsPipeline& graphics, float elapsed_time) override;

    ////----------�V���h�E�}�b�v�o�^----------//
    void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) override;
private:
    //-----�z�X�g�ɂȂ�v���C���[�̔ԍ�-----//
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

    //-----���̃X�e�[�W-----//
    static WaveManager::STAGE_IDENTIFIER current_stage;

    //-----�N���C�A���g���I�������X�e�[�W-----//
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
    // bgm�؂�ւ��p�ϐ�
    LastBoss::Mode old_last_boss_mode = LastBoss::Mode::None;
    LastBoss::Mode last_boss_mode = LastBoss::Mode::None;
    // �X�J�C�{�b�N�X�̐F�؂�ւ��p�ϐ�
    float purple_threshold = 0;
    float red_threshold = 0;
    // �N���A���o
    bool during_clear = false;
    float tunnel_alpha = 0.0f;
    std::unique_ptr<Tunnel> tunnel{ nullptr };

    // �{�X�̕ϐ�
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
    //���͂������邩�ǂ���
    bool can_axis{ true };
    //���͂��Ă���̎���
    float axis_wait_timer{};
    //���͋�����
    static constexpr float AXIS_WAIT_TIME = 0.2f;

    //---------�Q�[���I�[�o�[--------//
    void GameOverAct(float elapsed_time);
    bool is_game_over{ false };

    float glow_vertical = {};

    // �Q�[���I�[�o�[
    StepFontElement game_over_text;
    // �^�C�g���ɂ��ǂ�
    StepFontElement back_title;
    // �Ē���
    StepFontElement again;
    int game_over_state{ 0 };

    //-----�Q�[���I�[�o�[�̎��Ƀ^�C�g����߂��������-----//
    bool game_over_select_title{ false };

    //-----�Ē����I������-----//
    bool game_over_trying_again{ false };

    //-----�Ē���I��-----//
    int trying_again_count{};

    //-----�Q�[���Ē�����������Ƃ��̕�����-----//
    std::wstring game_over_idle = L"";

    StepFontElement game_over_select_idle;

    bool StepString(float elapsed_time, std::wstring full_text, StepFontElement& step_font_element,float speed = 1.0f, bool loop = false);


    //�Q�[���I�[�o�[�̕����̌��ɏo�Ă�t���[��
    std::unique_ptr<SpriteBatch> sprite_back{ nullptr };
    //�t���[���̃p�����[�^
    Element game_over_pram;
    //��ʂ���������
    std::unique_ptr<SpriteBatch> brack_back{ nullptr };
    //��ʍ�������p�����[�^
    Element brack_back_pram;
    //�����Ȃ�؂������ǂ���
    bool is_set_black{ false };
    //�Q�[���I�[�o�[�̉摜
    std::unique_ptr<SpriteBatch> game_over_sprite{ nullptr };
    //�t���[���̃p��
    Element game_over_sprite_pram;
    //�Q�[���I�[�o�[�̃X�v���C�g���`�悳�������ǂ���
    bool is_game_over_sprite{ false };

    //---------�Q�[���N���A-----------//
    void GameClearAct(float elapsed_time, GraphicsPipeline& graphics);
    //�C�x���g�J�����̃Z�b�g
    bool set_joint_camera{ false };
    bool is_game_clear{ false };
    // �Q�[���N���A
    StepFontElement game_clear_text;

private:
    bool slow{ false };
    float slow_timer{ 0.0f };
    float slow_rate{ 0.05f };
    //----------�Q�[���̃X���E����----------//
    void JudgeSlow(float& elapsed_time);

    //-----------�{�X�̌`�Ԃł̃p�����[�^�̍Đݒ�----------//
    void SetBossTypeGameParam();

    //----------�{�X�̃C�x���g�J�����̐ݒ�----------//
    void BossEventCamera();

    //----------�X�J�C�h�[���̐F----------//
    void SetSkyDomeColor(float elapsed_time);

    //----------�v���C���[�̍X�V����----------//
    void PlayerManagerUpdate(GraphicsPipeline& graphics,float elapsed_time);

    //---------�v���C���[�֌W�̓����蔻��----------//
    void PlayerManagerCollision(GraphicsPipeline& graphics, float elapsed_time);

    //-----���g���C���ɍď�����������֐�-----//
    void RestartInitialize();

private:
    //-----�Q�[�����J�n���邩�ǂ���(�G���o�������Ă�������)-----//
    bool is_start_game{ false };

    //�Q�[�����J�n����Ƃ��̕ϐ�
    float game_start_timer{ 0 };

    std::unique_ptr<SpriteDissolve> game_start_gauge{ nullptr };
    struct GameStartGauge
    {
        DirectX::XMFLOAT2 pos{ -13.9f,439.6f };
        DirectX::XMFLOAT2 scale{ 3.8f,2.9f };
        float threshold{ 1.0f };
    };
    //-----�X�v���C�g�̃p�����[�^-----//
    GameStartGauge game_start_gauge_parm;

    std::wstring game_start_text = L"�o�b�N�{�^���������ŃQ�[�����J�n";
    struct GameStartTxt
    {
        DirectX::XMFLOAT2 position{ 19.0f,445.1f };
        DirectX::XMFLOAT2 scale{ 0.5f,0.5f };
        float angle;
        DirectX::XMFLOAT4 color{ 1.0f,1.0f,0.0f,1.0f };
    };
    //-----�t�H���g�̃p�����[�^-----//
    GameStartTxt game_start_txt_parm;


    std::unique_ptr<SpriteBatch> controller_on__back_keys{ nullptr };
    Element controller_back_pram;

private:
    void StartGameUi(float elapsed_time);

private:
    //-----TCP�ʐM���Ǘ�����X���b�h-----//
    std::thread tcp_thread;

    //-----UDP�ʐM���Ǘ�����X���b�h-----//
    std::thread udp_thread;

    //-----�r������-----//
    static std::mutex mutex;
private:
    //-----TCP�X���b�h���I�����邩�̃t���O-----//
    static bool end_tcp_thread;

    //-----UDP�X���b�h���I�����邩�̃t���O-----//
    static bool end_udp_thread;

    //-----�v���C���[��ǉ����邩�ǂ���-----//
    static bool register_player;

    //-----�ǉ�����v���C���[�̔ԍ�-----//
    static int register_player_id;

    //-----�ǉ������v���C���[�̐F-----//
    static int register_player_color;

    //-----���O�A�E�g����v���C���[��ID-----//
    static std::vector<int> logout_id;


private:
    //----------�ʐM�֌W(�}���`�X���b�h)----------//
    //-----�v���C���[��TCP�p�̃}���`�X���b�h-----//
    static void ReceiveTcpData();

    ////------�v���C���[�̃��O�C������------//
    //===========================
    //��1���� : �ʐM����̔ԍ�
    static void Login(int client_id, char* data);

    ////-----�v���C���[�̃��O�A�E�g����-----//
    //==============================
    //��1���� : ��M�f�[�^
    static void Logout(char* data);

private:
    //-----UDP�p�̃}���`�X���b�h-----//
    static void ReceiveUdpData();

    ////-----�f�[�^�̎�ނ̊m�F-----//
    //=======================
    //��1���� : �R�}���h
    //��2���� : ��M�f�[�^
    static void CheckDataCommand(char com, char* data);

    ////-----�f�[�^�̎�ނ̊m�F(�v���C���[�̃A�N�V�����f�[�^�p)-----//
    //=======================
    //��1���� : �R�}���h
    //��2���� : ��M�f�[�^
    static void CheckPlayerActionCommand(char com, char* data);

private:
    //----------�ʐM�֌W----------//

    ////----------�N���C�A���g�����O�C�����ė������Ƀv���C���[��ǉ�����-----------//
    void RegisterPlayer(GraphicsPipeline& graphics);

    ////-----���O�A�E�g�����v���C���[���폜����-----//
    void DeletePlayer();

    ////-----��M�f�[�^��ݒ肷��-----//
    void SetReceiveData();

    //-----�G�̃_���[�W�f�[�^��ݒ�-----//
    void SetEnemyDamageData(GraphicsPipeline& graphics);

    //-----�G�̏�ԃf�[�^��ݒ�-----//
    void SetEnemyConditionData();

    //-----�v���C���[�̗̑͂̃f�[�^�̐ݒ�-----//
    void ReceivePlayerHealthData();

    ////-----�G�̎�M�f�[�^���폜����-----//
    void ClearEnemyReceiveData();

    ////-----�G�̃z�X�g�����n�֌W�̏���-----//
    void TransferEnenyControlProcessing();

    ////-----�G�̃z�X�g�����A���Ă������̏���-----//
    void ReturnEnemyControl();
private:
    //-----��M�f�[�^������-----//
    static PlayerAllDataStruct receive_all_data;

    //-----�G�̎�M�f�[�^������-----//
    static EnemyAllDataStruct receive_all_enemy_data;

    //-----�G�̃z�X�g���̏��n���N�G�X�g���󂯂����ǂ���-----//
    static bool transfer_enemy_host_request;

    //-----���N�G�X�g���ė����v���C���[�̔ԍ�-----//
    static int transfer_enemy_request_id;

    //-----�G�̃z�X�g���̏��n���ꂽ���ǂ���-----//
    static bool transfer_enemy_host_result;

    //-----�G�̃z�X�g�����A���ė������ǂ���-----//
    static bool return_enemy_control;

    //-----�Ē����I���������ɑ�����-----//
    static std::vector<int> select_trying_again;
};
