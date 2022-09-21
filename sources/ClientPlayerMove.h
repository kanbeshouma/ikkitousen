#pragma once
#include"MoveBehavior.h"

class ClientPlayerMove : public MoveBehavior
{
public:
    ClientPlayerMove();
    ~ClientPlayerMove();
protected:
    float max_length = 300.0f;
public:
    void UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& orientation, const DirectX::XMFLOAT3& camera_forward, const DirectX::XMFLOAT3& camera_right, const DirectX::XMFLOAT3& camera_pos, SkyDome* sky_dome);
    //�v���C���[�̈ʒu����
    void PlayerJustification(float elapsed_time, DirectX::XMFLOAT3& pos);
    //-----���͕����̐ݒ�-----//
    void SetMoveVec(DirectX::XMFLOAT3 vec) { movevec = vec; }
    //-----���͕����̃Q�b�^�[-----//
    DirectX::XMFLOAT3 GetMoveVec() { return movevec; }

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
    //�v���C���[�̑O����
    DirectX::XMFLOAT3 player_forward{};
    //�����擾
    void SetDirections(DirectX::XMFLOAT4 o);
};