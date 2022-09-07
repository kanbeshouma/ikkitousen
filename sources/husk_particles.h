#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#include <vector>
#include "skinned_mesh.h"
#include "constants.h"

struct HuskParticles
{
	size_t max_particle_count;
	struct Particle
	{
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 velocity;
		float age{};
		int state{};
	};

	struct ParticleConstants
	{
		int particle_count{};
		float particle_size{ 0.001f };
		float particle_option{};
		float delta_time{};
		DirectX::XMFLOAT4 position_on_near_plane{ 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT4 eye_position{ 0.0f, 0.0f, 0.0f, 1.0f };
	};
	std::unique_ptr<Constants<ParticleConstants>> particle_constants;

	Microsoft::WRL::ComPtr<ID3D11Buffer> particle_buffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particle_buffer_uav;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particle_append_buffer_uav;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particle_buffer_srv;

	Microsoft::WRL::ComPtr<ID3D11Buffer> updated_particle_buffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> updated_particle_buffer_uav;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> updated_particle_buffer_srv;

	Microsoft::WRL::ComPtr<ID3D11Buffer> particle_count_buffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometry_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> compute_shader;

	HuskParticles(ID3D11Device* pDevice, size_t max_particle_count = 1000000);
	HuskParticles(const HuskParticles&) = delete;
	HuskParticles& operator=(const HuskParticles&) = delete;
	HuskParticles(HuskParticles&&) noexcept = delete;
	HuskParticles& operator=(HuskParticles&&) noexcept = delete;
	virtual ~HuskParticles() = default;

	void integrate(ID3D11DeviceContext* dc, float delta_time);
	void render(ID3D11DeviceContext* dc);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> accumulate_husk_particles_ps;
	void accumulate_husk_particles(ID3D11DeviceContext* dc, std::function<void()> drawcallback);
};