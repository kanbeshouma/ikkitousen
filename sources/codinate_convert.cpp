#include "codinate_convert.h"
#include "camera.h"

const DirectX::XMFLOAT2 conversion_2D(ID3D11DeviceContext* dc, const DirectX::XMFLOAT3& coordinate)
{
	using namespace DirectX;
	// ビューポート
	D3D11_VIEWPORT viewport;
	UINT num_viewports = 1;
	dc->RSGetViewports(&num_viewports, &viewport);
	// 変換行列
	const DirectX::XMFLOAT4X4& view = Camera::get_keep_view();
	XMMATRIX view_mat = XMLoadFloat4x4(&view);
	const DirectX::XMFLOAT4X4& projection = Camera::get_keep_projection();
	XMMATRIX projection_mat = XMLoadFloat4x4(&projection);
	XMMATRIX world_mat = DirectX::XMMatrixIdentity();
	XMVECTOR coordinate_vec = XMLoadFloat3(&coordinate);
	// ビューポート左上X座標
	float viewport_x = viewport.TopLeftX;
	// ビューポート左上Y座標
	float viewport_y = viewport.TopLeftY;
	// ビューポート幅/高さ
	float viewport_w = viewport.Width;
	float viewport_h = viewport.Height;
	// 深度値の範囲を表す最小値(0.0でよい)
	float viewport_min_z = viewport.MinDepth;
	// 深度値の範囲を表す最大値(1.0でよい)
	float viewport_max_z = viewport.MaxDepth;
	// ワールド座標からスクリーン座標に変換する
	XMVECTOR screen_position_vec = DirectX::XMVector3Project(
		coordinate_vec, viewport_x, viewport_y, viewport_w, viewport_h,
		viewport_min_z, viewport_max_z, projection_mat, view_mat, world_mat);
	// スクリーン座標
	XMFLOAT2 screen_position;
	XMStoreFloat2(&screen_position, screen_position_vec);
	return screen_position;
}