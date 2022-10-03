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

class SceneTitle : public Scene, public PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    SceneTitle() {}
    ~SceneTitle() override {}
    //--------< �֐� >--------//
    //������
    void initialize(GraphicsPipeline& graphics) override;
    //�I����
    void uninitialize() override;
    // �G�t�F�N�g���
    void effect_liberation(GraphicsPipeline& graphics) override;
    //�X�V����
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    //�`�揈��
    void render(GraphicsPipeline& graphics, float elapsed_time) override;
    //�V���h�E�}�b�v�o�^
    void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) override;
private:
    //-- audio volume --//
    const float BGM_VOLUME = 2.0f;
    const float SE_VOLUME = 2.0f;
    //--logo_parameters--//
    const int FRAMW_COUNT_X = 7;
    const int FRAMW_COUNT_Y = 4;
    const float LOGO_ANIMATION_WAIT_TIME = 2.0f;
private:
    //-----���S�̃A�j���[�V�������x-----//
    float logo_animation_speed = 0.03f;

    //-----���S��texpos.x�̓������[�g-----//
    int frame_x{};

    //-----���S�̃A�j���[�V����-----//
    void LogoAnimation(float elapsed_time);
private:
    //-----�^�C�g�����ڂ̑I��-----//
    void TitleSelectEntry(float elapsed_time);

private:
    //-----�}���`�v���C�̍���-----//
    enum MultiPlayEntry
    {
        //-----�Q�[�����������-----//
        Host,
        //-----�Q�[�������ɓ���-----//
        Client,
        //-----�I�����ڂ��o��-----//
        Back,
    };

    //-----�}���`�v���C���ڃX�e�[�g-----//
    int multi_play_entry_state{ MultiPlayEntry::Host };

    //-----�}���`�v���C�̍��ڂ�I���������ǂ���-----//
    bool select_mulch_play{ false };

private:
    //-----MultiPlayEntry�̃R���g���[���[����-----//

    ////-----�Z���N�g�o�[����ɓ�����-----//
    //============================
    //��1���� : ���̃X�e�[�g
    //��2 , 3���� : �I���o�[�̈ʒu
    void MultiPlayEntryUp(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2);

    ////-----�Z���N�g�o�[�����ɓ�����-----//
    //============================
    //��1���� : ���̃X�e�[�g
    //��2 , 3���� : �I���o�[�̈ʒu
    void MultiPlayEntryDown(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2);
private:
    //----<3D�֘A>----//
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

    //--------< �ϐ� >--------//
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
    // ���߂���
    StepFontElement beginning;
    // �}���`�v���C
    StepFontElement multiplay;
    // ��������
    StepFontElement succession;
    // �Q�[���I��
    StepFontElement exit;
    // ���[�h��
    StepFontElement now_loading;

    //-----�}�b�`���O��-----//
    StepFontElement now_matching;

    //-----�z�X�g�v���C-----//
    StepFontElement host_play_font;

    //-----�N���C�A���g�v���C-----//
    StepFontElement client_play_font;

    //-----�ЂƂO�̍��ڂɖ߂�-----//
    StepFontElement multi_paly_entry_back;

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

    enum TitleEntry
    {
        Beginning,
        Succession,
        Multiplay,
        Exit,
    };
private:
    //-----TitleEntry�̃R���g���[���[����-----//

    ////-----�Z���N�g�o�[����ɓ�����-----//
    //============================
    //��1���� : ���̃X�e�[�g
    //��2 , 3���� : �I���o�[�̈ʒu
    void TitleEntryUp(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2);

    ////-----�Z���N�g�o�[�����ɓ�����-----//
    //============================
    //��1���� : ���̃X�e�[�g
    //��2 , 3���� : �I���o�[�̈ʒu
    void TitleEntryDown(int next_state, DirectX::XMFLOAT2 arrival_pos1, DirectX::XMFLOAT2 arrival_pos2);

private:
    int have_tutorial_state = -1; // -1:�`���[�g���A���f�[�^�Ȃ� 0:�`���[�g���A������ 1:�`���[�g���A���Ȃ�

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

    // �R���t�B�O
    Element config_a;
    std::unique_ptr<SpriteBatch> sprite_config_a{ nullptr };

    Element config_b;
    Element config_b2;
    std::unique_ptr<SpriteBatch> sprite_config_b{ nullptr };
    StepFontElement config_b_font;
    StepFontElement config_b_font2;

    //--------< �}���`�X���b�h >--------//
    static void loading_thread(ID3D11Device* device);
    static bool is_load_ready;

private:
    //-----�}���`�X���b�h-----//
    std::thread standby_thread;

    //-----�r������-----//
    static std::mutex mutex;

private:
    //----------�ʐM�֌W--------//

    //-----���O�C�����������������ꍇ�̃V�[���ؑւ����Ă������ǂ���----//
    static bool change_scene_thread;

    //-----�}�b�`���O�ҋ@����-----//
    static float standby_matching_timer;

    static bool start_matching;
private:
    //------------�}�b�`���O--------------//
    static void StandbyMatching();
};