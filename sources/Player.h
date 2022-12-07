#pragma once
#include<memory>
#include<functional>
#include<tuple>
#include <utility>
#include<map>

#include"BasePlayer.h"
#include"PlayerMove.h"
#include"collision.h"
#include"BaseEnemy.h"
#include"player_config.h"
#include"SwordTrail.h"
#include"post_effect.h"
#include"graphics_pipeline.h"
#include "Common.h"
#include"game_icon.h"
#include "effect.h"
#include "reticle.h"
#include"player_condition.h"

enum class SePriset : uint16_t
{
    None = (0 << 0),
    First = (1 << 0),
    Second = (2 << 0),
    Third = (3 << 0),
    Forth = (4 << 0),
    Fifth = (5 << 0),
    Sixth = (6 << 0),
    Seventh = (7 << 0),

    Se0 = None,
    Se1 = First,
    Se2 = First | Second,
    Se3 = First | Second | Third,
    Se4 = First | Second | Third | Forth,
    Se5 = First | Second | Third | Forth | Fifth,
    Se6 = First | Second | Third | Forth | Fifth | Sixth,
    Se7 = First | Second | Third | Forth | Fifth | Sixth | Seventh,

};
inline bool operator&(SePriset lhs, SePriset rhs)
{
    return static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs);
}

class Player :public BasePlayer, private PlayerMove
{
public:
    Player(GraphicsPipeline& graphics,int object_id = 0);
    ~Player();
private:
    enum AnimationClips
    {
        Idle,//�ҋ@
        TransformWing,//��s�@���[�h�ɂȂ�
        IdleWing,//�ό`�ҋ@
        TransformHum,//�l�^���[�h�ɂȂ�
        Move,//�ړ�
        Avoidance,//���
        ChargeInit,//�ːi�J�n
        Charge,//�ːi��
        AttackType1,//�U��1����
        AttackType2,//�U��2����
        AttackType3,//�U��3����
        Damage,//�_���[�W
        Awaking,//�o����ԂɂȂ�
        InvAwaking,//�ʏ��ԂɂȂ�
        AwakingIdle,//�o����Ԃ̑ҋ@
        AwakingMove,//�o����Ԃ̈ړ�
        AwakingAvoidance,//�o����Ԃ̉��
        AwakingChargeInit,//�o����Ԃ̓ːi�J�n
        AwakingCharge,//�o����Ԃ̓ːi
        AwakingAttackType1,//�o����Ԃ̍U��1����
        AwakingAttackType2,//�o����Ԃ̍U��2����
        AwakingAttackType3,//�o����Ԃ̍U��3����
        AwakingDamage,//�o����Ԃ̃_���[�W
        WingDashStart,//��s�@���[�h�̓ːi�J�n
        WingDashIdle,//��s�@���[�h�̓ːi��
        WingDashEnd,//��s�@���[�h�̓ːi�I��
        Die,//���S
        Dying,//���S��
        AwakingDie,//���S
        AwakingDying,//���S��
        NamelessMotion,//���[�V����
        NamelessMotionIdle,//���[�V����
        TitleAnimationReadyIdle,//�^�C�g�����[�V����1
        TitleAnimationStart,//�^�C�g�����[�V����2
        TitleAnimationStartIdle,//�^�C�g�����[�V����3
        TitleAnimationEnd,//�^�C�g�����[�V����4
        TitleAnimationEndIdle,//�^�C�g�����[�V����5
        AwaikingScene,//�`���[�g���A���̊o���̂Ƃ��ɗ���
        AwaikingSceneIdle
    };
public:
    void Initialize()override;
    void Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)override;
    //�`���[�g���A���p�̃A�b�v�f�[�g
    void UpdateTutorial(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    //�^�C�g���p�̃A�b�v�f�[�g
    void UpdateTitle(float elapsed_time);
    //�X�^�����Ă���G�����邩�ǂ���
    bool EnemiesIsStun(std::vector<BaseEnemy*> enemies)override;
    void Render(GraphicsPipeline& graphics, float elapsed_time)override;
    void ConfigRender(GraphicsPipeline& graphics, float elapsed_time)override;
    void TutorialConfigRender(GraphicsPipeline& graphics, float elapsed_time, bool condition);
    void TitleRender(GraphicsPipeline& graphics, float elapsed_time);
    void ChangePlayerJustificationLength()override;
    void SetReceiveData(PlayerMoveData data) override {}
    void SetReceivePositionData(PlayerPositionData data)override {}
    void SetPlayerActionData(PlayerActionData data)override {}

    void RestartInitialize(int health) override;
private:
    //-----�f�[�^�𑗐M����-----//
    void SendPlayerData(float elapsed_time);

    //-----MoveData��ݒ�-----//
    void SendMoveData();

    //-----PositionData��ݒ�-----//
    void SendPositionData();

    //-----PlayerActionData��ݒ�-----//
    void SendPlayerActionData(GamePadButton button, DirectX::XMFLOAT3 vec = {});

    //-----�~���b-----//
    float milliseconds{};

    //-----�b-----//
    float send_position_timer{};

    //-----�f�[�^���M�p�x(�~���b)-----//
    const float PlayerMoveDataFrequency = 200.0f;

    //-----�f�[�^���M�p�x(�b)-----//
    const float PlayerPositionDataFrequency = 2.0f;

private:
    //�G�t�F�N�g
    //��荞�݉��
    std::unique_ptr<Effect> player_behind_effec;
    //���
    std::unique_ptr<Effect> player_air_registance_effec;
    float air_registance_offset_y{4.0f};
    //�`�F�C���U���̃q�b�g
    std::unique_ptr<Effect> player_slash_hit_effec;
    DirectX::XMFLOAT3 slash_effec_pos{};
    std::unique_ptr<Effect> player_awaiking_effec;
    std::unique_ptr<Effect> just_stun;
    std::unique_ptr<Effect> player_move_effec_r;
    std::unique_ptr<Effect> player_move_effec_l;
    std::unique_ptr<Effect> player_behaind_effec_2;

    //USE_IMGUI
private:
    //�v���C���[�̗̑͂̉񕜗�
    static constexpr int RECOVERY_HEALTH = 100;
    //�ːi����
    static constexpr float CHARGE_MAX_TIME = 1.0f;
    //�ːi�̐i�s�����ϊ���
    static constexpr int CHARGE_DIRECTION_COUNT = 3;
    //�U��1���ڂ̗P�\����
    static constexpr float ATTACK_TYPE1_MAX_TIME = 0.3f;
    //�U��2���ڂ̗P�\����
    static constexpr float ATTACK_TYPE2_MAX_TIME = 0.2f;
    //�U��3���ڂ̗P�\����
    static constexpr float ATTACK_TYPE3_MAX_TIME = 0.2f;
    //���b�N�I���ł��鋗��
    static constexpr float LOCK_ON_LANGE = 100.0f;

public:
    //���ɉ�荞�߂鋗��
    static constexpr float  BEHIND_LANGE_MAX = 45.0f;
    static constexpr float  BEHIND_LANGE_MIN = 5.0f;
    //-----�v���C���[�̓��͒l�̍ŏ��l-----//
    static constexpr float PLAYER_INPUT_MIN = 2.0f;
private:
    //�R���{�̍ő吔
    static constexpr float MAX_COMBO_COUNT = 80.0f;
    //����̎��̃A�j���[�V�����X�s�[�h
    static constexpr float AVOIDANCE_ANIMATION_SPEED = 1.5f;
    //�ːi�J�n�̎��̃A�j���[�V�����X�s�[�h
    static constexpr float CHARGEINIT_ANIMATION_SPEED = 5.5f;
    //�ːi�̎��̃A�j���[�V�����X�s�[�h
    static constexpr float CHARGE_ANIMATION_SPEED = 2.0f;
    //�U��1�̎��̃A�j���[�V�����X�s�[�h
    static constexpr float ATTACK1_ANIMATION_SPEED = 2.5f;
    //�U��2�̎��̃A�j���[�V�����X�s�[�h
    static constexpr float ATTACK2_ANIMATION_SPEED = 4.5f;
    //�U��3�̎��̃A�j���[�V�����X�s�[�h
    static constexpr float ATTACK3_ANIMATION_SPEED = 4.5f;
    //�Q�[�W�����(�ːi)
    static constexpr float GAUGE_CONSUMPTION = 5.0f;
    //�l�^�ɖ߂�Ƃ��̃A�j���[�V�����X�s�[�h
    static constexpr float TRANSFORM_HUM_ANIMATION_SPEED = 2.0f;
    //��s�@���[�h�ɂȂ�Ƃ��̃A�j���[�V�����X�s�[�h
    static constexpr float TRANSFORM_WING_ANIMATION_SPEED = 2.0f;
    //�`�F�C���U���̎��ɃR���{�Q�[�W�������
    static constexpr float COMBO_COUNT_SUB = 7.0f;
    //�v���C���[���U�����Ƀ_���[�W���󂯂��Ƃ��̌�����
    static constexpr int ATTACK_DAMAGE_INV = 1;
    //�v���C���[����𒆂Ƀ_���[�W���󂯂��Ƃ��̌�����
    static constexpr int AVOIDANCE_DAMAGE_INV = 2;
    //�v���C���[�̃W���X�g����̉�
    static constexpr int JUST_AVOIDANCE_HEALTH = 10;
    //�W���X�g����̎��̃R���{�Q�[�W��
    static constexpr float JUST_AVOIDANCE_COMBO = 5.0f;
    //�v���C���[�̓ːi�̍U����
    static constexpr int CHARGE_NORMAL_ATTACK_POWER = 1;
    static constexpr int CHARGE_AWAIKING_ATTACK_POWER = 1;
    //1���ڂ̍U����
    static constexpr int ATTACK_TYPE1_NORMAL_ATTACK_POWER = 1;
    static constexpr int ATTACK_TYPE1_AWAIKING_ATTACK_POWER = 4;
    //2���ڂ̍U����
    static constexpr int ATTACK_TYPE2_NORMAL_ATTACK_POWER = 2;
    static constexpr int ATTACK_TYPE2_AWAIKING_ATTACK_POWER = 6;
    //3���ڂ̍U����
    static constexpr int ATTACK_TYPE3_NORMAL_ATTACK_POWER = 4;
    static constexpr int ATTACK_TYPE3_AWAIKING_ATTACK_POWER = 9;


private:
    DirectX::XMFLOAT3 camera_forward{};//�J�����̑O����
    DirectX::XMFLOAT3 camera_right{};//�J�����̉E����
    DirectX::XMFLOAT3 camera_position{};//�J�����̉E����
private:
    //�v���C���[�̊e����
    DirectX::XMFLOAT3 forward;
    DirectX::XMFLOAT3 right;
    DirectX::XMFLOAT3 up;
private:
    //�J�����̌��������Z�b�g
    bool camera_reset{ false };
    //�G�����b�N�I���������ǂ���(�J�����p�̃��b�N�I���������ǂ���)
    bool is_camera_lock_on{ false };
private:
    //�G�ɓ����������ǂ���
    bool is_enemy_hit{ false };
private:
    //�ːi���̎���(�ːi���n�܂�����^�C�}�[������)
    float charge_time{ 0 };
    //�ːi���̎��Ԃ��ǂꂾ�����₷��
    float charge_add_time{ 1.0f };
    //�ːi���̐i�s�����ϊ���
    int charge_change_direction_count{ CHARGE_DIRECTION_COUNT };
    //�U���̎���
    float attack_time{ 0 };
    //�U�����̎��Ԃ��ǂꂾ�����₷��
    float attack_add_time{ 1.0f };
private:
    //������n�܂�������
    float avoidance_boost_time{ 0 };
    //����̕����]���̉�
    int avoidance_direction_count{ 3 };
    //����̃{�^���������ė��������ǂ���
    bool avoidance_buttun{ false };
    //����
    float  avoidance_velocity{ 15.0f };
    //����̃A�j���[�V�������X�^�[�g����
    bool avoidance_animation_start{ false };
    //�C�[�W���O�̌��ʎ���
    float easing_time{ 1.0f };
    //����u�[�X�g�J�n�Ɠ��B
    DirectX::XMFLOAT3 avoidance_start{};
    DirectX::XMFLOAT3 avoidance_end{};
    float avoidance_easing_time{ 0.6f };
    //�u�[�X�g�̔{��
    float leverage{ 15.0f };
    //�J�����p�̉�������u��
    bool is_avoidance{ false };
    //��荞�݉�����ǂ���
    bool is_behind_avoidance{ false };
    //��荞�݉���̃N�[���^�C��
    float behaind_avoidance_cool_time{ 0 };
    //�N�[���^�C�����������ꂽ���ǂ��� true :��������Ă��Ȃ�
    bool behaind_avoidance_recharge{ false };
    //�W���X�g������Ă��邩�ǂ���
    bool is_just_avoidance{ false };
    //�W���X�g����ł���J�v�Z���ɓ����Ă��邩�ǂ���
    bool is_just_avoidance_capsul{ false };
    //�|�����G�̈ʒu��ۑ�
    DirectX::XMFLOAT3 old_target{};
    //�J�����̕�Ԃ̃S�[���n�_
    DirectX::XMFLOAT3 end_target{};
    //��ԗ�
    float target_lerp_rate{ 0 };
    //���b�N�I�����Ă��Ȃ��Ƃ��̃J�����̃^�[�Q�b�g
    DirectX::XMFLOAT3 camera_target{};
    //�J�����̃^�[�Q�b�g�̕��
    void LerpCameraTarget(float elapsed_time);
private:
    //�^�[�Q�b�g�̓G
    BaseEnemy* target_enemy;
    float enemy_length{ 0 };//�G�ƃv���C���[�̋���
    int target_count{ 0 };
    int old_target_count{ 0 };
private:
    //���ɉ�荞�ނ��߂̌v�Z����֐�
    void BehindAvoidancePosition();
    //�X�v���C���Ȑ����g�����߂̓r���̓_
    DirectX::XMFLOAT3 behind_point_0{};//�X�^�[�g
    DirectX::XMFLOAT3 behind_point_1{};//���p�n�_
    DirectX::XMFLOAT3 behind_point_2{};//���p�n�_
    DirectX::XMFLOAT3 behind_point_3{};//�S�[��

       //��荞�ރX�s�[�h
    float behind_speed{ 0.0f };
    float behind_test_timer{ 0.0f };

    int behind_transit_index = 0;
    std::vector<DirectX::XMFLOAT3> behind_way_points;
    std::vector<DirectX::XMFLOAT3> behind_interpolated_way_points;

    //�w��ɉ�荞�ޓ_
    std::vector<DirectX::XMFLOAT3> behind_point{};
    bool BehindAvoidanceMove(float elapsed_time, int& index, DirectX::XMFLOAT3& position, float speed,
        const std::vector<DirectX::XMFLOAT3>& points, float play);
    //�w��ɉ�荞�ނƂ��ɐi�ރ^�C�}�[
    float behind_timer{};
    //�w��ɉ�荞�ނƂ��̃��[�g
    float behind_late{};
private:
    //�v���C���[�̍U����(�R���{�ɂ���ĕω����Ă���)
    int player_attack_power{ 3 };
    //�R���{��
    float combo_count{ 0 };
    //�R���{�Q�[�WSE
    bool combo_max_se{ false };
    //�Q�[�W����̓ːi���ɓ���������
    float special_surge_combo_count{ 0 };
    //�v���C���[�����U�����������łȂ���
    bool is_attack{ false };
    //�v���C���[���Q�[�W����̓ːi�����Ă��邩
    bool is_special_surge{ false };
    //�Q�[�W����̓ːi�̌�
    float special_surge_opportunity{ 1.5f };
    //float special_surge_timer{ 0 };
    float opportunity_timer{ 0 };
    //�v���C���[���񂾂��ǂ���
    bool is_alive{ true };
    //���G����
    float invincible_timer{};
    //�R���{�̎�������
    float duration_combo_timer{};
    //���b�N�I�����Ă��Ȃ��Ƃ��̓ːi�̃^�[�Q�b�g
    DirectX::XMFLOAT3 charge_point{};
    //�_�b�V���G�t�F�N�g�J�n
    bool start_dash_effect{ false };
    //�_�b�V���G�t�F�N�g�I��
    bool end_dash_effect{ false };
    //�o����Ԃ��ǂ���
    bool is_awakening{ false };
    //�ːi�����ǂ���
    bool is_charge{ false };
    //�u���b�N���ꂽ���ǂ���
    bool is_block{ false };
    //�v���C���[�̃A�j���[�V�����X�s�[�h
    float animation_speed{ 1.0f };
    //�ːi�̉����p��velocity
    DirectX::XMFLOAT3 acceleration_velocity;
    //�ːi��velocity�̕�Ԃ̃��[�g
    float lerp_rate{ 1.0f };
    //�ːi�̃^�[�Q�b�g�܂ł̋����̔{��(�ǂꂾ���L�΂���)
    float charge_length_magnification{ 100.0f };
    //�U���̃A�j���[�V�����X�s�[�h�̃f�o�b�O�p
    DirectX::XMFLOAT4 attack_animation_speeds{ 1.0f,1.0f,1.0f,1.0f };
    //�U���̃A�j���[�V������Ԃ̎���
    DirectX::XMFLOAT4 attack_animation_blends_speeds{ 0.3f,0.0f,0.0f,0.0f };
    //�A�j���[�V���������Ă������ǂ���
    bool is_update_animation{ true };
    //�N���A���o��
    bool during_clear{ false };
    //�{�X�̉��o��
    bool boss_camera{ false };
    //�v���C���[�̃p�����[�^
    std::unique_ptr<PlayerConfig> player_config{ nullptr };
    std::unique_ptr<PlayerCondition> player_condition{ nullptr };
    //--------------------<SwordTrail�`���̋O�Ձ`>--------------------//
    SwordTrail mSwordTrail[2]{};
    float mTrailEraseTimer{};

    skeleton::bone player_bones[12];
private:
    //�v���C���[�̃p�����[�^�̕ω�
    void InflectionParameters(float elpased_time);
    void TutorialInflectionParameters(float elpased_time);
    //�R���{�̕ω�
    void InflectionCombo(float elapsed_time);
    //����ł邩�ǂ���
    void PlayerAlive();
    //�`���[�g���A���̎��̎���ł邩�ǂ���
    void TutorialPlayerAlive();
private:
    float sphere_radius{ 0.0f };
    CapsuleParam sword_capsule_param[2]{};
    CapsuleParam charge_capsule_param{};
    CapsuleParam body_capsule_param{};
    CapsuleParam just_avoidance_capsule_param{};
    DirectX::XMFLOAT3 step_pos_r{};
    DirectX::XMFLOAT3 step_pos_l{};
    DirectX::XMFLOAT3 capsule_body_start{ 0,2.6f,0 };
    DirectX::XMFLOAT3 capsule_body_end{ 0,0.2f,0 };
    //�����̃J�v�Z��
    void StepCapsule();
    void BodyCapsule();
    //���̃J�v�Z������
    void SwordCapsule();
    //�͈̓X�^���̃p�����[�^�ݒ�
    void StunSphere();
    enum class ConditionState
    {
        Alive,//�����Ă���
        Die//����ł���
    };
    //�����Ă��邩�ǂ���
    ConditionState condition_state{ ConditionState::Alive };
public:
    void SetEndDashEffect(bool a)override { end_dash_effect = a; }
    void SetCameraTarget(DirectX::XMFLOAT3 p)override { camera_target = p; }
    void SetBossCamera(bool boss_c) override { boss_camera = boss_c; }
    void SetPosition(DirectX::XMFLOAT3 pos) override { position = pos; }
    DirectX::XMFLOAT3 GetForward()override { return forward; }
    DirectX::XMFLOAT3 GetRight()override { return right; }
    DirectX::XMFLOAT3 GetUp()override { return up; }
    DirectX::XMFLOAT3 GetPosition()override { return position; }
    DirectX::XMFLOAT3 GetVelocity()override { return velocity; }
    HitResult& GetPlayerHitResult()override { return hit; }
    bool GetCameraReset()override { return camera_reset; }
    bool GetCameraLockOn()override { return is_camera_lock_on; }
    bool GetEnemyLockOn()override { return is_lock_on; }
    bool GetAvoidance()override { return is_avoidance; }
    bool GetBehindAvoidance()override { return is_behind_avoidance; }
    bool GetIsPlayerAttack()override { return is_attack; }
    bool GetIsCharge()override { return is_charge; }
    bool GetIsSpecialSurge() override { return is_special_surge; }
    bool GetStartDashEffect() override { return start_dash_effect; }
    bool GetEndDashEffect() override { return end_dash_effect; }
    bool GetIsAwakening() override { return is_awakening; }
    bool GetIsAlive() override { return is_alive; }
    bool GetIsJustAvoidance() override { return is_just_avoidance; }
    bool GetBehaindCharge() override { return behaind_avoidance_recharge;}
    CapsuleParam GetBodyCapsuleParam() override { return body_capsule_param; }
    CapsuleParam GetJustAvoidanceCapsuleParam() override { return just_avoidance_capsule_param; }
    CapsuleParam GetSwordCapsuleParam(int i)override
    {
        //�����ːi���Ȃ�ːi���̓����蔻���Ԃ�
        if (is_charge)
        {
            return charge_capsule_param;
        }
        //�o����ԂȂ�����Ŏ󂯎�����l��n��
        if (is_awakening)
        {
            return sword_capsule_param[i];
        }
        //���ʂ̌��̈ʒu��n��
        return sword_capsule_param[0];
    }
    float GetStunRadius() override { return sphere_radius; }
    std::vector<DirectX::XMFLOAT3> GetBehindPoint() override { return behind_point; }
    void SetRaycast(bool r) override { raycast = r; }
    int GetPlayerPower() override { return player_attack_power; }
    [[nodiscard("Not used")]] const AddDamageFunc GetDamagedFunc() override { return damage_func; }

    BaseEnemy* GetPlayerTargetEnemy() const  override
    {
        if (target_enemy != nullptr && target_enemy->fComputeAndGetIntoCamera())
        {
            return target_enemy;
        }
        return nullptr;
    }
    //��ԋ߂��G�������ė��Ă��̈ʒu���Z�b�g����
    void SetTarget(BaseEnemy* target_enemy)override;
    DirectX::XMFLOAT3 GetTarget() override { return target; };
    void AddCombo(int count, bool& block)override;
    //�o����Ԃ̎��͂Q�����蔻�肪���邩��������Q��
    void AwakingAddCombo(int hit_count1, int hit_count2, bool& block)override;
    //--------------------<�G����_���[�W���󂯂�>--------------------//
    void DamagedCheck(int damage, float InvincibleTime)override;
    void TutorialDamagedCheck(int damage, float InvincibleTime)override;
    void PlayerKnocKback(float elapsed_time)override;
    //�v���C���[�̃W���X�g���p�̓����蔻��ɓ���������
    void PlayerJustAvoidance(bool hit)override;
    int GetHealth() override { return player_health; }
    void SetHealth(int arg) override { player_health = arg; }

public:
    void FalseCameraReset() override { camera_reset = false; }
    void FalseCameraLockOn() override { is_camera_lock_on = false; }
    void FalseAvoidance() override { is_avoidance = false; }
private:
    void GetPlayerDirections();
public:
    void SetCameraDirection(const DirectX::XMFLOAT3& c_forward, const DirectX::XMFLOAT3& c_right) override
    {
        camera_forward = c_forward;
        camera_right = c_right;
    }
    void SetCameraPosition(DirectX::XMFLOAT3 p) override { camera_position = p; }
private:
    //����̉���
    void AvoidanceAcceleration(float elapse_time);
    //�ːi�̉���(���`���)SetAccelerationVelocity�œ����悤�ɕύX����
    void ChargeAcceleration(float elapse_time);
    //�U���̉����̐ݒ�
    void SetAccelerationVelocity();
    //�Q�[�W����̓ːi
    void SpecialSurgeAcceleration();
private:
    //���b�N�I��
    void LockOn();
    //�`���[�g���A���ł̃��b�N�I��(�����͕ς��Ȃ�)
    void TutorialLockOn();
    //�`�F�C���U���̎��̃��b�N�I��
    void ChainLockOn();
    //�J�������Z�b�g
    void CameraReset();
private:
    typedef void(Player::* PlayerTitleActivity)(float elapsed_time);
    PlayerTitleActivity player_title_activity = &Player::UpdateTitleAnimationReadyIdle;
    void ExecFuncUpdate(float elapsed_time);
    //�^�C�g���p�A�j���[�V�������Đ�
    bool start_title_animation{ false };
    //�^�C�g���p�A�j���[�V�������I�������
    bool end_title_animation{ false };
    //�^�C�g���p�^�C�}�[
    float title_timer{ 0 };
    //�^�C�g���p�ҋ@
    void UpdateTitleAnimationReadyIdle(float elaosed_time);
    void UpdateTitleAnimationStart(float elaosed_time);
    void UpdateTitleAnimationStartIdle(float elaosed_time);
    void UpdateTitleAnimationEnd(float elaosed_time);
    void UpdateTitleAnimationEndIdle(float elaosed_time);
public:
    void TransitionTitleAnimationReadyIdle();
    void StartTitleAnimation() { start_title_animation = true; }
    bool GetStartTitleAnimation() { return start_title_animation; }
    bool GetEndTitleAnimation() { return end_title_animation; }
private:
    AddDamageFunc damage_func;
    //-----------�A�j���[�V�����Ɋ֌W����֐�,�ϐ�------------//
    //�A�j���[�V�����J�ڂ̊֐��|�C���^//
    //�֐��|�C���^
    typedef void(Player::* PlayerActivity)(float elapsed_time, SkyDome* sky_dome);
    //�֐��|�C���^�̕ϐ�
    PlayerActivity player_activity = &Player::IdleUpdate;

    //�����̃����o�֐��̊֐��|�C���^���Ă�
    void ExecFuncUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    //�ҋ@�A�j���[�V�������̍X�V����
    void IdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //�ړ��A�j���[�V�������̍X�V����
    void MoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //����A�j���[�V�������̍X�V����
    void AvoidanceUpdate(float elapsed_time, SkyDome* sky_dome);
    //���ɉ�荞�މ���̍X�V����
    void BehindAvoidanceUpdate(float elapsed_time, SkyDome* sky_dome);
    //�ːi�J�n�A�j���[�V�������̍X�V����
    void ChargeInitUpdate(float elapsed_time, SkyDome* sky_dome);
    //�ːi���̍X�V����
    void ChargeUpdate(float elapsed_time, SkyDome* sky_dome);
    //�U��1���ڂ̍X�V����
    void AttackType1Update(float elapsed_time, SkyDome* sky_dome);
    //�U��2���ڂ̍X�V����
    void AttackType2Update(float elapsed_time, SkyDome* sky_dome);
    //�U��3���ڂ̍X�V����
    void AttackType3Update(float elapsed_time, SkyDome* sky_dome);
    //�Q�[�W�����ːi
    void SpecialSurgeUpdate(float elapsed_time, SkyDome* sky_dome);
    //�Q�[�W����ːi���I����Ă���̌�
    void OpportunityUpdate(float elapsed_time, SkyDome* sky_dome);
    //�_���[�W�󂯂��Ƃ�
    void DamageUpdate(float elapsed_time, SkyDome* sky_dome);
    //�l�^�ɖ߂�
    void TransformHumUpdate(float elapsed_time, SkyDome* sky_dome);
    //��s�@���[�h
    void TransformWingUpdate(float elapsed_time, SkyDome* sky_dome);
    //�o����Ԃɕό`����Ƃ��̍X�V
    void AwakingUpdate(float elapsed_time, SkyDome* sky_dome);
    //�ʏ��Ԃɕό`����Ƃ��̍X�V
    void InvAwakingUpdate(float elapsed_time, SkyDome* sky_dome);
    //�X�e�[�W�ړ��̎��̍X�V
    void StageMoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //��s�@���[�h�̓ːi�J�n
    void WingDashStartUpdate(float elapsed_time, SkyDome* sky_dome);
    //��s�@���[�h�̓ːi��
    void WingDashIdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //��s�@���[�h�̓ːi�I��
    void WingDashEndUpdate(float elapsed_time, SkyDome* sky_dome);
    //���S
    void DieUpdate(float elapsed_time, SkyDome* sky_dome);
    //���S��
    void DyingUpdate(float elapsed_time, SkyDome* sky_dome);
    //���S�̍X�V�����ɓ�������true
    bool is_dying_update{ false };
    //���[�V����
    void StartMothinUpdate(float elapsed_time, SkyDome* sky_dome);


    void Awaiking();//�o����Ԃ�ON,OFF
    //�A�j���[�V�����J��(1frame���������Ă΂Ȃ�����)
public:
    //�ҋ@�ɑJ��
    void TransitionIdle(float blend_second = 0.3f) override;
private:
    //�ړ��ɑJ��
    void TransitionMove(float blend_second = 0.3f);
    //����ɑJ��
    void TransitionAvoidance();
    //�w��ɉ�荞�މ���ɑJ��
    void TransitionBehindAvoidance();
    //�W���X�g����̉�荞�݉���ɑJ��
    void TransitionJustBehindAvoidance();
    //�ːi�J�n�ɑJ��
    void TransitionChargeInit();
    //�ːi�ɑJ��
    void TransitionCharge(float blend_seconds = 0.3f);
    //�P���ڂɑJ��
    void TransitionAttackType1(float blend_seconds = 0.3f);
    //�Q���ڂɑJ��
    void TransitionAttackType2(float blend_seconds = 0.3f);
    //�R���ڂɑJ��
    void TransitionAttackType3(float blend_seconds = 0.3f);
    //�Q�[�W����̓ːi�ɑJ��
    void TransitionSpecialSurge();
    //�Q�[�W����̓ːi��̌��ɑJ��
    void TransitionOpportunity();
    //�_���[�W�󂯂��Ƃ��ɑJ��
    void TransitionDamage();
    //��s�@���[�h�ɑJ��
    void TransitionTransformWing();
    //�l�^�ɕό`�ɑJ��
    void TransitionTransformHum();
    //�o����ԂɑJ��
    void TransitionAwaking();
    //�ʏ��ԂɑJ��
    void TransitionInvAwaking();
    //��s�@���[�h�̓ːi�J�n
    void TransitionWingDashStart();
    //��s�@���[�h�̓ːi��
    void TransitionWingDashIdle();
    //��s�@���[�h�̓ːi�I��
    void TransitionWingDashEnd();
    //���S
    void TransitionDie();
    //���S��
    void TransitionDying();

public:
    //�X�^�[�g���[�V����
    void TransitionStartMothin()override;
    //�X�e�[�W�ړ��ɑJ��
    void TransitionStageMove()override;
    //�X�e�[�W�J�ڏI��
    void TransitionStageMoveEnd()override;
    //�N���A�Ɋւ��邱��
private:
    //���[�V����
    void NamelessMotionUpdate(float elapsed_time, SkyDome* sky_dome);
    void NamelessMotionIdleUpdate(float elapsed_time, SkyDome* sky_dome);
    void TransitionNamelessMotionIdle();
    //�N���A���[�V�����̎��̉�
    int nameless_motion_se_state = 0;
    //�N���A���[�V�����ɑJ��
    void TransitionNamelessMotion();
    //�N���A�p���[�V�������I�������true
    bool is_end_clear_motion{ false };
    //�N���A�p���[�V�������n�܂�����true
    bool is_start_cleear_motion{ false };
    //�C�x���g�V�[���̍��̘g
    float wipe_parm{ 0.0f };
    DirectX::XMFLOAT3 event_camera_eye{ 0,3.4f,0.0f };
    DirectX::XMFLOAT3 event_camera_joint{};
public:
    DirectX::XMFLOAT3 GetEnentCameraEye() override { return event_camera_eye; }
    DirectX::XMFLOAT3 GetEnentCameraJoint() override { return event_camera_joint; }
    bool GetEndClearMotion() override { return is_end_clear_motion; }
    bool GetStartClearMotion() override { return is_start_cleear_motion; }
    void PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)override;
private:
    //�֐��|�C���^
    typedef void(Player::* PlayerChainMoveActivity)(float elapsed_time, SkyDome* sky_dome);
    PlayerChainMoveActivity chain_activity = &Player::ChainIdleUpdate;
    //�ҋ@�A�j���[�V�������̍X�V����
    void ChainIdleUpdate(float elapsed_time, SkyDome* sky_dome);
    //�ړ��A�j���[�V�������̍X�V����
    void ChainMoveUpdate(float elapsed_time, SkyDome* sky_dome);
    //�ҋ@�ɑJ��
    void TransitionChainIdle(float blend_second = 0.3f);
    //�ړ��ɑJ��
    void TransitionChainMove(float blend_second = 0.3f);


private:
    //--------<�����p�[�g>--------//
    //�֐��|�C���^
    typedef void(Player::* PlayerChainActivity)(float elapsed_time, std::vector<BaseEnemy*> enemies,GraphicsPipeline& Graphics_);
    //�֐��|�C���^�̕ϐ�
    PlayerChainActivity player_chain_activity = &Player::chain_search_update;
    // ���G
    void chain_search_update(float elapsed_time, std::vector<BaseEnemy*> enemies,
        GraphicsPipeline& graphics_);
    void transition_chain_search();
    // ���b�N�I������
    void chain_lockon_begin_update(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    void transition_chain_lockon_begin();
    // ���b�N�I��
    void chain_lockon_update(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    void transition_chain_lockon();
    // �ړ�
    void chain_move_update(float elapsed_time, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
    void transition_chain_move();
    // �U��
    void chain_attack_update(float elapsed_time, std::vector<BaseEnemy*> enemies,
        GraphicsPipeline& Graphics_);
    void transition_chain_attack();
    // �w�肵���|�C���g�S�Ă�ʂ�֐�
    bool transit(float elapsed_time, int& index, DirectX::XMFLOAT3& position,
        float speed, const std::vector<DirectX::XMFLOAT3>& points, float play = 0.01f);
    // �i��ł�����ɉ�]����֐�
    void rotate(float elapsed_time, int index, const std::vector<DirectX::XMFLOAT3>& points);
    // behavior�̑J�ڊ֐�
    void transition_chain_behavior()
    {
        player_move_effec_r->stop(effect_manager->get_effekseer_manager());
        player_move_effec_l->stop(effect_manager->get_effekseer_manager());
        player_air_registance_effec->stop(effect_manager->get_effekseer_manager());
        behavior_state = Behavior::Chain;
        transition_chain_search();
    }
    void transition_normal_behavior()
    {
        PostEffect::clear_post_effect();
        behavior_state = Behavior::Normal;
        if (is_tutorial)TransitionTutoriaIdle();
        else TransitionIdle();
    }

    void chain_parm_reset();
    float change_normal_timer{ 0.0f };
public:
    bool during_search_time() override { return search_time < SEARCH_TIME && search_time > 0; }
    bool during_chain_attack_end() override { return behavior_state == Behavior::Chain && is_chain_attack; }  // ���b�N�I����������U���I���܂�true
    bool during_chain_attack() override { return is_chain_attack_aftertaste; }  // ���b�N�I����������U���I����J�������ǂ��������Ƃ�����Ƒ҂���true
    void lockon_post_effect(float elapsed_time, std::function<void(float, float)> effect_func, std::function<void()> effect_clear_func)override;
private:
    //--------< �ϐ� >--------//
    struct LockOnSuggest
    {
        DirectX::XMFLOAT3 position{};
        bool detection = false;
    };
    static constexpr int STEPS = 3;
    static constexpr float AddAttackEndCameraTimer = 1.0f;

    static constexpr float CHRONOSTASIS_TIME = 0.3f;
    float chronostasis_scope = 0.8f;
    float chronostasis_saturation = 1.0f;


    float SEARCH_TIME = 0.5f;
    bool setup_search_time = false;
    float search_time = SEARCH_TIME;
    int transit_index = 0;
    std::vector<int> chain_lockon_enemy_indexes; // ���b�N�I�����ꂽ�̓G�̃C���f�b�N�X
    std::vector<LockOnSuggest> lockon_suggests;  // �v���C���[�ƃ��b�N�I�����ꂽ�G�̏������\�[�g���邽�߂̏��
    std::vector<DirectX::XMFLOAT3> sort_points;  // �\�[�g���ꂽ�|�C���g
    std::vector<DirectX::XMFLOAT3> way_points;   // ���ԓ_���Z�o�����|�C���g
    std::vector<DirectX::XMFLOAT3> interpolated_way_points; // way_points��ʂ�悤�ɕ��������|�C���g
    std::map<std::unique_ptr<Reticle>, BaseEnemy*> reticles; // �`�F�C���U����reticles
    bool is_chain_attack = false; // ���b�N�I����������U���I���܂�true
    bool is_chain_attack_aftertaste = false; // ���b�N�I����������U���I����J�������ǂ��������Ƃ�����Ƒ҂���true
    float is_chain_attack_aftertaste_timer = 0;
    static constexpr float ROCKON_FRAME = 0.3f;
    float frame_time  = 0.0f;
    float frame_scope = 0.5f;
    float frame_alpha = 0.0f;
    bool chain_cancel = false;
    enum class ATTACK_TYPE
    {
        FIRST,
        SECOND,
        THIRD,
    };
    ATTACK_TYPE attack_type = ATTACK_TYPE::FIRST;

    //-----�}���`�v���C�̎��ɑ���`�F�C���U��������G�̔ԍ���ۑ�-----//
    std::vector<char> rock_on_enemy_id;

private:
    //------------------------------------------------------------------------------------------//
    //                        �`���[�g���A���Ɋւ���֐�,�ϐ�
    //------------------------------------------------------------------------------------------//
    //�`���[�g���A�����ǂ��� true�Ń`���[�g���A��
    bool is_tutorial{ false };
    //���̃`���[�g���A�����I���������true�ɂȂ�
    bool is_next_tutorial{ false };
    //���̃`���[�g���A���łǂꂾ�����삵�����ǂ���
    float execution_timer{ 0 };
    //���̃`���[�g���A���ŉ��񂻂̍s����������
    int tutorial_action_count{ 3 };
    //�`���[�g���A���̊֐��|�C���^���Ă�
    void ExecFuncTutorialUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies, GraphicsPipeline& Graphics_);
public:
    //�`���[�g���A���̃X�e�[�g�ύX
    void ChangeTutorialState(int state);
    void SetIsTutorial(bool tutorial) { is_tutorial = tutorial; }
    void FalseNextTutorial() { is_next_tutorial = false; }
    void SetTutorialCount(int count) { tutorial_action_count = count; }
    bool GetNextTutorial() { return is_next_tutorial; }
    int GetTutorialCount() { return tutorial_action_count; }
private:
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
    //�`���[�g���A���̎��̃A�j���[�V�����X�V����
    typedef void(Player::* PlayerTutorialActivity)(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    //�֐��|�C���^�̕ϐ�
    PlayerTutorialActivity player_tutorial_activity = &Player::TutorialIdleUpdate;
    //�e��X�V����
    void TutorialIdleUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialMoveUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAvoidanvceUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialBehindAvoidanceUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialChargeinitUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialChargeUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAttack1Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAttack2Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAttack3Update(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAwaikingUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialInvAwaikingUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialDamageUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAwaikingEventUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    void TutorialAwaikingEventIdleUpdate(float elapsed_time, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);
    //�e�J�ڊ֐�
    void TransitionTutoriaIdle(float blend_second = 0.3f);
    void TransitionTutorialMove(float blend_second = 0.3f);
    void TransitionTutorialAvoidance(float blend_second = 0.3f);
    void TransitionTutorialBehindAvoidance();
    void TransitionTutorialJustBehindAvoidance();
    void TransitionTutorialChargeInit();
    void TransitionTutorialCharge(float blend_second = 0.3f);
    void TransitionTutorialAttack1(float blend_second = 0.3f);
    void TransitionTutorialAttack2(float blend_second = 0.3f);
    void TransitionTutorialAttack3(float blend_second = 0.3f);
    void TransitionTutorialAwaiking();
    void TransitionTutorialInvAwaiking();
    //�C�x���g�V�[���̊o��
    void TransitionTutorialAwaikingEvent();
    void TransitionTutorialAwaikingEventIdle();
    int awaiking_event_state = 0;
    //�`���[�g���A���̊o���C�x���g���n�܂�����true(�P�񂾂�)
    bool tutorial_awaiking{ false };
    bool awaiking_event{ false };
    bool awaiking_se{ false };
    //�_���[�W�󂯂��Ƃ��ɑJ��
    void TransitionTutorialDamage();

    SePriset se_priset = SePriset::Se0;
    void TutorialAwaiking();//�o����Ԃ�ON,OFF
public:
    bool GetTutorialEvent() { return awaiking_event; }
    int GetTutorialState() { return static_cast<int>(tutorial_state); }
    void SetTutorialDamageFunc();

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

    //-----�����̃I�u�W�F�N�g�ԍ���\������-----//
    StepFontElement object_id_font;

    //-----�X�N���[�����W�ɕϊ����鎞�̃I�t�Z�b�g�l-----//
    DirectX::XMFLOAT3 offset_pos{};

    //-----�����̃I�u�W�F�N�g�ԍ�����ʂɕ\������-----//
    void RenderObjectId(GraphicsPipeline& graphics);

    //-----�X�N���[�����W�ɕϊ�-----//
    void ConversionScreenPosition(GraphicsPipeline& graphics);
private:
    std::string name;
public:
    void SetName(char* n) { name = n; }
private:
    //-----�G�̃z�X�g�������n�̃��N�G�X�g-----//
    void SendTransferHost();

};