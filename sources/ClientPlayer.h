#pragma once

#include<memory>
#include<functional>
#include<tuple>
#include <utility>
#include<map>

#include"BasePlayer.h"
#include"ClientPlayerMove.h"
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

class ClientPlayer : public BasePlayer, private ClientPlayerMove
{
public:
    ClientPlayer(GraphicsPipeline& graphics, int object_id = 0);
    ~ClientPlayer();
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


    enum ActionState
    {
        //-----�ҋ@-----//
        ActionIdle,
        //-----�ړ�-----//
        ActionMove,
        //-----���-----//
        ActionAvoidance,
        //-----�U��1-----//
        ActionAttack1,
        //-----�U��2-----//
        ActionAttack2,
        //-----�U��3-----//
        ActionAttack3,
        //-----�ːi�J�n-----//
        ActionChargeInit,
        //-----�ːi��-----//
        ActionCharge,
        //-----�_���[�W-----//
        ActionDamage,
        //-----�o��-----//
        ActionAwaking,
        //-----�ʏ��ԂɂȂ�-----//
        ActionInvAwaking,
        //-----���S-----//
        ActionDie,
        //-----���S��-----//
        ActionDying,
        //-----��s�@���[�h�ҋ@-----//
        ActionIdleWing,
        //-----�X�e�[�W�ړ�-----//
        ActionStageMove,
        //-----�X�e�[�W�ړ���-----//
        ActionStageMoveIdle,
        //-----�X�e�[�W�ړ��I��-----//
        ActionStageMoveEnd
    };

    ////-----�A�j���[�V�����̃X�e�[�g-----//
    ActionState action_state{ ActionState::ActionIdle };
public:
    void Initialize()override;
    void Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)override;
    bool EnemiesIsStun(std::vector<BaseEnemy*> enemies)override { return false; };
    void Render(GraphicsPipeline& graphics, float elapsed_time)override;
    void ConfigRender(GraphicsPipeline& graphics, float elapsed_time)override {};
    void ChangePlayerJustificationLength()override {};
    void SetReceiveData(PlayerMoveData data) override;
    void SetReceivePositionData(PlayerPositionData data)override;
    void SetPlayerActionData(PlayerActionData data)override;
    void RestartInitialize(int health) override;
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
private:
    //�^�[�Q�b�g�̓G
    BaseEnemy* target_enemy;
    float enemy_length{ 0 };//�G�ƃv���C���[�̋���
    int target_count{ 0 };
    int old_target_count{ 0 };
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
    //����ł邩�ǂ���
    void PlayerAlive();

    //-----�p�����[�^�̍X�V-----//
    void InflectionParameters(float elapesd_time);

    enum class ConditionState
    {
        Alive,//�����Ă���
        Die//����ł���
    };
    //�����Ă��邩�ǂ���
    ConditionState condition_state{ ConditionState::Alive };


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
    bool GetCameraReset()override { return false; }
    bool GetCameraLockOn()override { return false; }
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
    bool GetBehaindCharge() override { return behaind_avoidance_recharge; }
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
    std::vector<DirectX::XMFLOAT3> GetBehindPoint() override { return {}; }
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
    void TutorialDamagedCheck(int damage, float InvincibleTime)override {};
    void PlayerKnocKback(float elapsed_time)override {};
    //�v���C���[�̃W���X�g���p�̓����蔻��ɓ���������
    void PlayerJustAvoidance(bool hit)override {};
    int GetHealth() override { return player_health; }
    void SetHealth(int arg) override { player_health = arg; }

public:
    void FalseCameraReset() override {}
    void FalseCameraLockOn() override {}
    void FalseAvoidance() override {}
    DirectX::XMFLOAT3 GetEnentCameraEye()override { return {}; };
    DirectX::XMFLOAT3 GetEnentCameraJoint()override { return {}; };
    bool GetEndClearMotion()override { return false; };
    bool GetStartClearMotion()override { return false; };
    void PlayerClearUpdate(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies)override {};
public:
    bool during_search_time()override { return false; }
    bool during_chain_attack_end()override { return false; }
    bool during_chain_attack()override { return false; }
    void lockon_post_effect(float elapsed_time, std::function<void(float, float)> effect_func, std::function<void()> effect_clear_func) override {}

private:
    void GetPlayerDirections();
public:
    void SetCameraDirection(const DirectX::XMFLOAT3& c_forward, const DirectX::XMFLOAT3& c_right) override
    {
        camera_forward = c_forward;
        camera_right = c_right;
    }

    void SetCameraPosition(DirectX::XMFLOAT3 p) override {}
private:
        AddDamageFunc damage_func;
        bool display_scape_imgui;
        float change_normal_timer{ 0.0f };
private:
    enum class Behavior
    {
        //�ʏ���
        Normal,
        //�X�^�������G�ɍU��������
        Chain
    };
    Behavior behavior_state{ Behavior::Normal };
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
    //�U���̉����̐ݒ�
    void SetAccelerationVelocity();
private:
    //�v���C���[�̊e����
    DirectX::XMFLOAT3 forward;
    DirectX::XMFLOAT3 right;
    DirectX::XMFLOAT3 up;
private:
    DirectX::XMFLOAT3 camera_forward{};//�J�����̑O����
    DirectX::XMFLOAT3 camera_right{};//�J�����̉E����
    DirectX::XMFLOAT3 camera_position{};//�J�����̉E����
private:
    //-----�{�^���̓��͏��-----//
    GamePadButton		button_state[2] = {};
    GamePadButton		button_down = 0;
    GamePadButton		button_up = 0;
    //----RT�{�^���̓��͒l-----//
    float	triggerR = 0.0f;
public:
    //-----�{�^���̓��͏���ݒ�-----//
    void SetSendButton(GamePadButton input);
    //-----RT�̓��͏��ݒ�-----//
    void SetTriggerR(float input) { triggerR = input; }

private:
    //-----------�A�j���[�V�����Ɋ֌W����֐�,�ϐ�------------//
    //�A�j���[�V�����J�ڂ̊֐��|�C���^//
    //�֐��|�C���^
    typedef void(ClientPlayer::* PlayerActivity)(float elapsed_time, SkyDome* sky_dome);
    //�֐��|�C���^�̕ϐ�
    PlayerActivity player_activity = &ClientPlayer::IdleUpdate;

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
private:
    //-----��M�������͕���-----//
    DirectX::XMFLOAT3 receive_action_vec{};
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
    DirectX::XMFLOAT2 offset_pos{};

    //-----�t���X�^���J�����O�p�̕ϐ�-----//
    float cube_half_size{};

    //-----�����̃I�u�W�F�N�g�ԍ�����ʂɕ\������-----//
    void RenderObjectId(GraphicsPipeline& graphics);

    //-----�X�N���[�����W�ɕϊ�-----//
    void ConversionScreenPosition(GraphicsPipeline& graphics);

    //-----�t���X�^���J�����O�����蔻��-----//
    bool FrustumVsCuboid();
private:
    //-----�����̖��O-----//
    std::string name;

    //-----�v���C���[�Ƃ̋���������-----//
    float  player_length{};

    float max_length{ 75.0f };
    float min_length{ 30.0f };

public:
    void SetName(std::string n);

    void SetPlayerToClientLength(float l)override { player_length = l; }
private:
    void LockOn();
};