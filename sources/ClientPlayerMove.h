#pragma once
#include"MoveBehavior.h"

class ClientPlayerMove : public MoveBehavior
{
public:
    ClientPlayerMove();
    ~ClientPlayerMove();
protected:
    float max_length = 300.0f;

    //-----��M�f�[�^�̈ʒu�̋��e�l-----//
    float allowable_limit_position{ 2.0f };
public:
    void UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, SkyDome* sky_dome);

    //�U�����̍X�V����(�����)
    void UpdateAttackVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);

    //��荞�ݒ��̍X�V����
    void UpdateBehindAvoidanceVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);

    //�v���C���[�̈ʒu����
    void PlayerJustification(float elapsed_time, DirectX::XMFLOAT3& pos);

    //�}����]�ːi�̎�
    void ChargeTurn(float elapsed_time, DirectX::XMFLOAT3 move_velocity, float speed, DirectX::XMFLOAT3 position, DirectX::XMFLOAT4& orientation);

    //-----���͕����̃Q�b�^�[-----//
    DirectX::XMFLOAT3 GetInputMoveVec() { return input_move; }

    //-----���������̗�-----//
    DirectX::XMFLOAT3 GetMoveVecter() { return movevec; }

    //-----�����͂̃Z�b�^�[-----//
    void SetMoveVecter(DirectX::XMFLOAT3 vec) { movevec = vec; }

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
protected:
    //�G�����b�N�I���������ǂ���
    bool is_lock_on{ false };
    //�G���J�������ɂ��Ȃ���ԂŃ��b�N�I���{�^���������Ă�����true
    bool is_push_lock_on_button{ false };
    //�^�[�Q�b�g
    DirectX::XMFLOAT3 target{};
private:
    //-----���͕���-----//
    DirectX::XMFLOAT3 movevec{};
    //-----�f�[�^���M�p�̕ϐ�-----//
    DirectX::XMFLOAT3 input_move{};
    //�v���C���[�̑O����
    DirectX::XMFLOAT3 player_forward{};
    //-----�v���C���[�̕⊮�ʒu-----//
    DirectX::XMFLOAT3 lerp_position{};
    //-----�⊮���Ă������ǂ���-----//
    bool start_lerp{ false };
    //�����擾
    void SetDirections(DirectX::XMFLOAT4 o);
    //-----�ʒu�ۊ�-----//
    void LerpPosition(float elapsed_time , DirectX::XMFLOAT3& position);

public:
    void SetLerpPosition(DirectX::XMFLOAT3 pos);
};