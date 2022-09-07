#include "user.h"
#include "mouse.h"
#include "debug_flags.h"

static const int key_map[] =
{
	VK_LBUTTON,	// ���{�^��
	VK_MBUTTON,	// ���{�^��
	VK_RBUTTON, // �E�{�^��
	VK_SPACE,
	VK_ESCAPE,
	'I',
	'K',
	'M',
	'Z',
	'Q',
	'E',
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12,
	'W',
	'A',
	'S',
	'D',
	VK_UP,
	VK_LEFT,
	VK_DOWN,
	VK_RIGHT,
};

void Mouse::update(HWND hwnd)
{
	update_cursor(hwnd);
	//---�L�[�{�[�h----//
	// �X�C�b�`���
	MouseButton newButtonState = 0;
	// key_map
	for (int i = 0; i < ARRAYSIZE(key_map); ++i)
	{
		if (::GetAsyncKeyState(key_map[i]) & 0x8000)
		{
			newButtonState |= (1 << i);
		}
	}
	// �{�^�����X�V
	if (operable)
	{
		button_state[1] = button_state[0];	// �X�C�b�`����
		button_state[0] = newButtonState;

		button_down = ~button_state[1] & newButtonState;	// �������u��
		button_up = ~newButtonState & button_state[1];	// �������u��
	}
	else
	{
		button_state[1] = 0; // �X�C�b�`����
		button_state[0] = 0;

		button_down = 0; // �������u��
		button_up = 0; // �������u��
	}
}

void Mouse::update_cursor(HWND hwnd)
{
	// �}�E�X
	GetCursorPos(&point);           // �X�N���[�����W���擾����
	ScreenToClient(hwnd, &point);   // �N���C�A���g���W�ɕϊ�����
	old_cursor_pos = cursor_pos;
	cursor_pos.x = (float)point.x - 15.0f;
	cursor_pos.y = (float)point.y - 15.0f;
	//�@�J�[�\���͈̔�
	cursor_pos.x = Math::clamp(cursor_pos.x, 0.0f, 1280.0f);
	cursor_pos.y = Math::clamp(cursor_pos.y, 0.0f, 720.0f);
	// �}�E�X�̑���
	switch (state)
	{
	case 0: // ������
		SetCursorPos((int)cursor_pos.x, (int)cursor_pos.y);
		++state;
		/*fallthrough*/
	case 1: // �ʏ펞
		ShowCursor(do_show);
		break;
	}
}

// �{�^�����͏�Ԃ̎擾
MouseButton Mouse::common_button() { return button_state[0]; }
MouseButton Mouse::game_button() { return DebugFlags::get_perspective_switching() ? 0 : button_state[0]; }
MouseButton Mouse::debug_button() { return DebugFlags::get_perspective_switching() ? button_state[0] : 0; }
// �{�^��������Ԃ̎擾
MouseButton Mouse::common_button_down() { return button_down; }
MouseButton Mouse::game_button_down() { return DebugFlags::get_perspective_switching() ? 0 : button_down; }
MouseButton Mouse::debug_button_down() { return DebugFlags::get_perspective_switching() ? button_down : 0; }
// �{�^�������Ԃ̎擾
MouseButton Mouse::common_button_up() { return button_up; }
MouseButton Mouse::game_button_up() { return DebugFlags::get_perspective_switching() ? 0 : button_up; }
MouseButton Mouse::debug_button_up() { return DebugFlags::get_perspective_switching() ? button_up : 0; }