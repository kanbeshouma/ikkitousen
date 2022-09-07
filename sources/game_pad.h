#pragma once

using GamePadButton = unsigned int;

// �Q�[���p�b�h
class GamePad
{
public:
	static const GamePadButton BTN_UP             = (1 << 0);
	static const GamePadButton BTN_RIGHT          = (1 << 1);
	static const GamePadButton BTN_DOWN           = (1 << 2);
	static const GamePadButton BTN_LEFT           = (1 << 3);
	static const GamePadButton BTN_A              = (1 << 4);
	static const GamePadButton BTN_B              = (1 << 5);
	static const GamePadButton BTN_X              = (1 << 6);
	static const GamePadButton BTN_Y              = (1 << 7);
	static const GamePadButton BTN_START          = (1 << 8);
	static const GamePadButton BTN_BACK           = (1 << 9);
	static const GamePadButton BTN_LEFT_THUMB     = (1 << 10);
	static const GamePadButton BTN_RIGHT_THUMB    = (1 << 11);
	static const GamePadButton BTN_LEFT_SHOULDER  = (1 << 12);
	static const GamePadButton BTN_RIGHT_SHOULDER = (1 << 13);
	static const GamePadButton BTN_LEFT_TRIGGER   = (1 << 14);
	static const GamePadButton BTN_RIGHT_TRIGGER  = (1 << 15);
	static const GamePadButton BTN_ATTACK_B       = (1 << 16);
public:
	GamePad() {}
	~GamePad() {}
	// �X�V
	/*�Ăяo����Framework�̂�*/
	void update(float elapsedTime);
	// �X���b�g�ݒ�
	void set_slot(int slot) { this->slot = slot; }
	// �{�^�����͏�Ԃ̎擾
	GamePadButton get_button();
	// �{�^��������Ԃ̎擾
	GamePadButton get_button_down();
	// �{�^�������Ԃ̎擾
	GamePadButton get_button_up();
	// ���X�e�B�b�NX�����͏�Ԃ̎擾
	float get_axis_LX();
	// ���X�e�B�b�NY�����͏�Ԃ̎擾
	float get_axis_LY();
	// �E�X�e�B�b�NX�����͏�Ԃ̎擾
	float get_axis_RX();
	// �E�X�e�B�b�NY�����͏�Ԃ̎擾
	float get_axis_RY();
	// ���g���K�[���͏�Ԃ̎擾
	float get_trigger_L();
	// �E�g���K�[���͏�Ԃ̎擾
	float get_trigger_R();
	// �o�C�u���[�V����
	bool set_vibration(float R, float L, float StopTime);
	// ����L���A����
	void operation_disablement() { operable = false; }
	void operation_activation() { operable = true; }
private:
	GamePadButton		button_state[2] = { 0 };
	GamePadButton		button_down = 0;
	GamePadButton		button_up = 0;
	float				axisLx = 0.0f;
	float				axisLy = 0.0f;
	float				axisRx = 0.0f;
	float				axisRy = 0.0f;
	float				triggerL = 0.0f;
	float				triggerR = 0.0f;
	int					slot = 0;

	float vib_time;
	float vib_stack_sec;
	bool is_vibration = false;

	bool operable = true;
};