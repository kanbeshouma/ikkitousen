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
    void PlayerManagerUpdate(GraphicsPipeline& graphics, float elapsed_time);

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

    //-----���O�A�E�g����v���C���[��ID-----//
    static std::vector<int> logout_id;

private:
    //----------�ʐM�֌W(�}���`�X���b�h)----------//
    //-----�v���C���[��TCP�p�̃}���`�X���b�h-----//
    static void ReceiveTcpData();
private:
    //-----UDP�p�̃}���`�X���b�h-----//
    static void ReceiveUdpData();

    ////-----�f�[�^�̎�ނ̊m�F-----//
    //=======================
    //��1���� : �R�}���h
    //��2���� : ��M�f�[�^
    static void CheckDataCommand(char com, char* data);
private:
    //----------�ʐM�֌W----------//

    ////----------�N���C�A���g�����O�C�����ė������Ƀv���C���[��ǉ�����-----------//
    void RegisterPlayer(GraphicsPipeline& graphics);

    ////-----���O�A�E�g�����v���C���[���폜����-----//
    void DeletePlayer();

    ////-----��M�f�[�^��ݒ肷��-----//
    void SetReceiveData();
private:
    //-----��M�f�[�^������-----//
    static AllDataStruct receive_all_data;
};
