#include "codinate_convert.h"
#include "camera.h"

const DirectX::XMFLOAT2 conversion_2D(ID3D11DeviceContext* dc, const DirectX::XMFLOAT3& coordinate)
{
	using namespace DirectX;
	// �r���[�|�[�g
	D3D11_VIEWPORT viewport;
	UINT num_viewports = 1;
	dc->RSGetViewports(&num_viewports, &viewport);
	// �ϊ��s��
	const DirectX::XMFLOAT4X4& view = Camera::get_keep_view();
	XMMATRIX view_mat = XMLoadFloat4x4(&view);
	const DirectX::XMFLOAT4X4& projection = Camera::get_keep_projection();
	XMMATRIX projection_mat = XMLoadFloat4x4(&projection);
	XMMATRIX world_mat = DirectX::XMMatrixIdentity();
	XMVECTOR coordinate_vec = XMLoadFloat3(&coordinate);
	// �r���[�|�[�g����X���W
	float viewport_x = viewport.TopLeftX;
	// �r���[�|�[�g����Y���W
	float viewport_y = viewport.TopLeftY;
	// �r���[�|�[�g��/����
	float viewport_w = viewport.Width;
	float viewport_h = viewport.Height;
	// �[�x�l�͈̔͂�\���ŏ��l(0.0�ł悢)
	float viewport_min_z = viewport.MinDepth;
	// �[�x�l�͈̔͂�\���ő�l(1.0�ł悢)
	float viewport_max_z = viewport.MaxDepth;
	// ���[���h���W����X�N���[�����W�ɕϊ�����
	XMVECTOR screen_position_vec = DirectX::XMVector3Project(
		coordinate_vec, viewport_x, viewport_y, viewport_w, viewport_h,
		viewport_min_z, viewport_max_z, projection_mat, view_mat, world_mat);
	// �X�N���[�����W
	XMFLOAT2 screen_position;
	XMStoreFloat2(&screen_position, screen_position_vec);
	return screen_position;
}