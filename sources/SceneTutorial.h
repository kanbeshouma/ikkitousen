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

//�R���g���[���[�̉摜�̃{�^���̃v���Z�b�gbit���Z�ł�_���Ō�ɕt����
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
    void TutorialUpdate(GraphicsPipeline& graphics, float elapsed_time);
    //�`���[�g���A���̕����Ƃ���`�悷��
    void TutorialRender(GraphicsPipeline& graphics, float elapsed_time);
private:
    //--------< �ϐ� >--------//
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
    //1���ŏ��ő傫���Ȃ��Ă����悤�ɂ���
    enum class TutorialState
    {
        //�ړ�
        MoveTutorial = 1,
        //���(�ʏ�)
        AvoidanceTutorial,
        //���b�N�I��
        LockOnTutorial,
        //�U��
        AttackTutorial,
        //��荞�݉��
        BehindAvoidanceTutorial,
        //�`�F�C���U��
        ChainAttackTutorial,
        //�o��
        AwaikingTutorial,
        //���R����
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
    //// �X�L�����b�V���̎���
    //std::shared_ptr<SkinnedMesh> sky_dome;
    //****************************************************************
    //
    // �X�s�b�N�J�n��ǉ������ϐ�
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
    //���̃`���[�g���A���ɍs���܂ł̎���
    float Judea_timer{ 0 };
    // �N���A���o
    bool during_clear = false;
    float tunnel_alpha = 0.0f;
    std::unique_ptr<Tunnel> tunnel{ nullptr };
private:
    //�`���[�g���A���̃e�L�X�g
    //�������p�̍\����
    struct StepFontElement
    {
        std::wstring s = L"";
        DirectX::XMFLOAT2 position{};
        DirectX::XMFLOAT2 scale{ 0.7f, 0.7f };
        DirectX::XMFLOAT4 color{ 1.0f,1.0f,1.0f,1.0f };
        float angle{};
        DirectX::XMFLOAT2 length{};
        //�������̃e�L�X�g
        std::wstring tutorial_text=L"UI";

        // step string
        float timer = 0;
        int step = 0;
        int index = 0;
        //�X�s�[�h
        float speed{ 25.0f };
    };
    StepFontElement tutorial_text_element[8];
    //�`���[�g���A���̐����̍Ō�܂ŕ\�����ꂽ���ǂ���
    bool end_tutorial_text{ false };
    float end_tutorial_text_timer{ 0.0f };
    //�摜�̃`���[�g���A���̎���B�{�^���̕⊮
    float b_bottun_rate{ 0.0f };
    bool b_button_change{ false };
    //�����`���[�g���A�����Ƀe�L�X�g��ς�����true�ɂȂ�
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
    //���`�F�b�N�{�b�N�X�̃e�L�X�g
    std::wstring tutorial_check_text = L"UI";
private:
    //�`�F�b�N�{�b�N�X�̉摜
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
    //�ʘg�ŉ摜��\�����Ă鎞��true�ɂȂ�
    bool sprite_tutorial{ false };
    //�摜�̃`���[�g���A�����I�������true
    bool end_sprite_tutorial{ false };
    //�摜�`���[�g���A���p�̃^�C�}�[
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

    //��ʂ���������
    std::unique_ptr<SpriteBatch> brack_back{ nullptr };
    //��ʍ�������p�����[�^
    Element brack_back_pram;
    //�摜�̃`���[�g���A���̌��ɏo�Ă�t���[��
    std::unique_ptr<SpriteBatch> sprite_frame{ nullptr };
    Element sprite_frame_parm;
    //�Q�[�W���������
    std::unique_ptr<SpriteBatch> arrow_mark{ nullptr };
    //�Q�[�W���������̃p�����[�^
    Element arrow_mark_pram;
    float arrow_rate{ 0 };
    bool arrow_move_change{ false };

    //�摜�̃`���[�g���A���̃W���X�g���
    std::unique_ptr<SpriteBatch> just_avoidance{ nullptr };
    Element just_avoidance_parm;
    //�摜�̃`���[�g���A���̊o��
    std::unique_ptr<SpriteBatch> awaiking_chain{ nullptr };
    Element awaiking_chain_parm;

    StepFontElement sprite_tutorial_text;
    StepFontElement tutorial_count_text;
private:
    //�V�[���ύX����Ƃ��̕ϐ�
    float change_scene_timer{ 0 };
    std::unique_ptr<SpriteDissolve> change_scene_gauge{ nullptr };
    struct ChangeSceneGauge
    {
        DirectX::XMFLOAT2 pos{ -13.9f,439.6f };
        DirectX::XMFLOAT2 scale{ 3.8f,2.9f };
        float threshold{ 1.0f };
    };
    ChangeSceneGauge change_gauge_parm;
    std::wstring tutorial_skip_text = L"�o�b�N�{�^���������Ń`���[�g���A���X�L�b�v";
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

    //�����̃t���[��
    std::unique_ptr<SpriteBatch> sprite_tutorial_frame{ nullptr };
    //�t���[���̃p�����[�^
    Element frame_pram;
private:
    //�R���g���[���[�̃{�^���̉摜
    enum ControllerSprite
    {
        A,//A�{�^��
        B,//B�{�^��
        X,//X�{�^��
        Y,//Y�{�^��
        RT,//RT�{�^��
        RB,//RB�{�^��
        LT,//LT�{�^��
        LB,//LB�{�^��
        RightStick,//RightStick�{�^��
        LeftStick,//LeftStick�{�^��
        Cross,//�\���{�^��
        Menu,//���j���[�{�^��
        Back,//�o�b�N�{�^��
        Back2,//�o�b�N�{�^��

        End
    };
    BottunPriset button_priset = BottunPriset::MoveTutorialPriset;
    //�R���g���[���[�̉摜
    std::unique_ptr<SpriteBatch> controller_base{ nullptr };
    std::unique_ptr<SpriteBatch> controller_keys[ControllerSprite::End]{ nullptr };
    std::unique_ptr<SpriteBatch> controller_on_keys[ControllerSprite::End]{ nullptr };
    //�R���g���[���[�̉摜�̃p�����[�^
    Element controller_pram;
    Element controller_back_pram;
    Element controller_b_pram;
    Element controller_back_button_pram;
    float back_button_rate{ 0 };
    bool back_button_move_change{ false };

private:
    //�X�^�����Ă���G�����Ȃ������瑝���Ă���
    float is_stun_timer{ 0 };
    bool reset_text{ false };
private:
    bool slow{ false };
    float slow_timer{ 0.0f };
    float slow_rate{ 0.05f };
private:
    //�t���[�̗��K���ǂ���
    bool free_practice{ false };
};
