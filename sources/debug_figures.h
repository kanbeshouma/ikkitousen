#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include "constants.h"

class DebugFigures
{
public:
	DebugFigures(ID3D11Device* device);
	~DebugFigures() {}
public:
	// �`����s
	void render_all_figures(ID3D11DeviceContext* context);
	// ���쐬
	void create_sphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color);
	// �~���쐬
	void create_cylinder(const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color);
	// �����̍쐬
	void create_cuboid(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& radius, const DirectX::XMFLOAT4& color);
	// �J�v�Z���쐬
	void create_capsule(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, float radius, const DirectX::XMFLOAT4& color);
private:
	// �����b�V���쐬
	void create_sphere_mesh(ID3D11Device* device, float radius, int slices, int stacks);
	// �~�����b�V���쐬
	void create_cylinder_mesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int slices, int stacks);
	// �����̃��b�V���쐬
	void create_cuboid_mesh(ID3D11Device* device, float radius1, float radius2, float radius3);
	// �J�v�Z�����b�V���쐬
	void create_capsule_mesh(ID3D11Device* device, float radius1, float radius2,
		const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, int slices);
private:
	struct FigureConstants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
	};
	std::unique_ptr<Constants<FigureConstants>> figure_constants;
	struct Sphere
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	center;
		float				radius;
	};
	struct Cylinder
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	position;
		float				radius;
		float				height;
	};
	struct Cuboid
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT3	radius;
	};
	struct Capsule
	{
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT3	start{0,0,0};	// �~���̒��S���̎n�[
		DirectX::XMFLOAT3	end{ 0,1,0 };	// �~���̒��S���̏I�[
		float				radius;	// ���a
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> sphere_vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cylinder_vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cuboid_vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cuboid_index_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> capsule_vertex_buffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

	std::vector<Sphere>	spheres;
	std::vector<Cylinder> cylinders;
	std::vector<Cuboid> cuboids;
	std::vector<Capsule> capsules;

	UINT sphere_vertex_count   = 0;
	UINT cylinder_vertex_count = 0;
	UINT cuboid_vertex_count   = 0;
	UINT cuboid_index_count    = 0;
	UINT capsule_vertex_count  = 0;
};