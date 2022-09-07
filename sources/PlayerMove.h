#pragma once
#include"MoveBehavior.h"
class PlayerMove : public MoveBehavior
{
public:
    PlayerMove();
    ~PlayerMove();
protected:
     float max_length = 300.0f;
public:
    void UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation , const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right,const DirectX::XMFLOAT3& camera_pos ,SkyDome* sky_dome);
    //��𒆂�UpdateVelocity
    void UpdateAvoidanceVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation , const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right,const DirectX::XMFLOAT3& camera_pos ,SkyDome* sky_dome);
    //��荞�ݒ��̍X�V����
    void UpdateBehindAvoidanceVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation , const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right,const DirectX::XMFLOAT3& camera_pos ,SkyDome* sky_dome);
    //�U�����̍X�V����
    void UpdateAttackVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation , const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right,const DirectX::XMFLOAT3& camera_pos ,SkyDome* sky_dome);
    //�Q�[�W����̓ːi�̍X�V����
    void UpdateSpecialSurgeVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);
    void UpdateRotateToTarget(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_pos);
    //�v���C���[�̈ʒu����
    void PlayerJustification(float elapsed_time, DirectX::XMFLOAT3& pos);
    //�G�ƃv���C���[�̈ʒu����
    void PlayerEnemyJustification(float elapsed_time, DirectX::XMFLOAT3& pos,const float player_radius ,const DirectX::XMFLOAT3 enemy_pos, const float enemy_radius);
    //�}����]�ːi�̎�
    void ChargeTurn(float elapsed_time, DirectX::XMFLOAT3 move_velocity, float speed, DirectX::XMFLOAT3 position, DirectX::XMFLOAT4& orientation);
public:
    //�`���[�g���A���p�̍X�V����
    void MoveTutorialUpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);
private:
    //�������͍X�V����
    void UpdateVerticalVelocity(float elapsedFrame);
    //�����ړ��X�V����
    void UpdateVerticalMove(float elapsedTime, DirectX::XMFLOAT3& position, SkyDome* sky_dome);
    //�������͍X�V����
    void UpdateHrizontalVelocity(float elasedFrame);
    //�����ړ��X�V����
    void UpdateHorizontalMove(float elapsedTime, DirectX::XMFLOAT3& position, SkyDome* sky_dome);
    //�^�[�Q�b�g�Ɍ������ĉ�]
    void RotateToTarget(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation);
    //�s�b�`��]
    void PitchTurn(DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& camera_pos, const DirectX::XMFLOAT3& camera_forward, DirectX::XMFLOAT4& orientation, float elapsed_time);
public:
    //���[����]
    void RollTurn(DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, float elapsed_time);
protected:
    //�G�����b�N�I���������ǂ���
    bool is_lock_on{ false };
    //�G���J�������ɂ��Ȃ���ԂŃ��b�N�I���{�^���������Ă�����true
    bool is_push_lock_on_button{ false };
    //�^�[�Q�b�g
    DirectX::XMFLOAT3 target{};
private:
    //�v���C���[�̑O����
    DirectX::XMFLOAT3 player_forward{};
    //�����擾
    void SetDirections(DirectX::XMFLOAT4 o);
private:
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
        AwaikingTutorial
    };

};