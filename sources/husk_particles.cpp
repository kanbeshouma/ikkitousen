#include "husk_particles.h"
#include "imgui_include.h"

#include <random>
#include "shader.h"
#include "misc.h"

using namespace DirectX;

HuskParticles::HuskParticles(ID3D11Device* device, size_t max_particle_count) : max_particle_count(max_particle_count)
{
	HRESULT hr{ S_OK };

	D3D11_BUFFER_DESC buffer_desc{};

	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Particle) * max_particle_count);
	buffer_desc.StructureByteStride = sizeof(Particle);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hr = device->CreateBuffer(&buffer_desc, NULL, particle_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Particle) * max_particle_count);
	buffer_desc.StructureByteStride = sizeof(Particle);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hr = device->CreateBuffer(&buffer_desc, NULL, updated_particle_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t));
	buffer_desc.StructureByteStride = sizeof(uint32_t);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	buffer_desc.MiscFlags = 0;
	hr = device->CreateBuffer(&buffer_desc, NULL, particle_count_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.ElementOffset = 0;
	shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(max_particle_count);
	hr = device->CreateShaderResourceView(particle_buffer.Get(), &shader_resource_view_desc, particle_buffer_srv.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.ElementOffset = 0;
	shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(max_particle_count);
	hr = device->CreateShaderResourceView(updated_particle_buffer.Get(), &shader_resource_view_desc, updated_particle_buffer_srv.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc;
	unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	unordered_access_view_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unordered_access_view_desc.Buffer.FirstElement = 0;
	unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(max_particle_count);
	unordered_access_view_desc.Buffer.Flags = 0;
	hr = device->CreateUnorderedAccessView(particle_buffer.Get(), &unordered_access_view_desc, particle_buffer_uav.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	unordered_access_view_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unordered_access_view_desc.Buffer.FirstElement = 0;
	unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(max_particle_count);
	unordered_access_view_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	hr = device->CreateUnorderedAccessView(particle_buffer.Get(), &unordered_access_view_desc, particle_append_buffer_uav.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	unordered_access_view_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unordered_access_view_desc.Buffer.FirstElement = 0;
	unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(max_particle_count);
	unordered_access_view_desc.Buffer.Flags = 0;
	hr = device->CreateUnorderedAccessView(updated_particle_buffer.Get(), &unordered_access_view_desc, updated_particle_buffer_uav.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	particle_constants = std::make_unique<Constants<ParticleConstants>>(device);

	create_vs_from_cso(device, "./shaders/husk_particles_vs.cso", vertex_shader.ReleaseAndGetAddressOf(), nullptr, nullptr, 0);
	create_ps_from_cso(device, "./shaders/husk_particles_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
	create_gs_from_cso(device, "./shaders/husk_particles_gs.cso", geometry_shader.ReleaseAndGetAddressOf());
	create_cs_from_cso(device, "./shaders/husk_particles_cs.cso", compute_shader.ReleaseAndGetAddressOf());

	create_ps_from_cso(device, "./shaders/accumulate_husk_particles_ps.cso", accumulate_husk_particles_ps.ReleaseAndGetAddressOf());
}

UINT align(UINT num, UINT alignment)
{
	return (num + (alignment - 1)) & ~(alignment - 1);
}
void HuskParticles::integrate(ID3D11DeviceContext* dc, float delta_time)
{
	particle_constants->data.particle_count = (std::min)(particle_constants->data.particle_count, (int)max_particle_count);
	if (particle_constants->data.particle_count == 0) { return; }

	dc->CSSetUnorderedAccessViews(0, 1, particle_buffer_uav.GetAddressOf(), nullptr);
	dc->CSSetUnorderedAccessViews(1, 1, updated_particle_buffer_uav.GetAddressOf(), nullptr);

	particle_constants->data.delta_time = delta_time;

	particle_constants->bind(dc, 9, CB_FLAG::CS);

	dc->CSSetShader(compute_shader.Get(), NULL, 0);

	UINT num_threads = align(particle_constants->data.particle_count, 1024);
	dc->Dispatch(num_threads / 1024, 1, 1);

	ID3D11UnorderedAccessView* null_unordered_access_view{};
	dc->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, nullptr);
	dc->CSSetUnorderedAccessViews(1, 1, &null_unordered_access_view, nullptr);
}

void HuskParticles::render(ID3D11DeviceContext* dc)
{
	particle_constants->data.particle_count = (std::min)(particle_constants->data.particle_count, (int)max_particle_count);
	if (particle_constants->data.particle_count == 0) { return; }

	dc->VSSetShader(vertex_shader.Get(), NULL, 0);
	dc->PSSetShader(pixel_shader.Get(), NULL, 0);
	dc->GSSetShader(geometry_shader.Get(), NULL, 0);
	dc->GSSetShaderResources(9, 1, updated_particle_buffer_srv.GetAddressOf());

	particle_constants->bind(dc, 9, CB_FLAG::PS_VS_GS);

	dc->IASetInputLayout(NULL);
	dc->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	dc->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	dc->Draw(static_cast<UINT>(particle_constants->data.particle_count), 0);

	ID3D11ShaderResourceView* null_shader_resource_view{};
	dc->GSSetShaderResources(9, 1, &null_shader_resource_view);
	dc->VSSetShader(NULL, NULL, 0);
	dc->PSSetShader(NULL, NULL, 0);
	dc->GSSetShader(NULL, NULL, 0);
}
void HuskParticles::accumulate_husk_particles(ID3D11DeviceContext* dc, std::function<void()> drawcallback)
{
	HRESULT hr{ S_OK };

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> cached_unordered_access_view;
	dc->OMGetRenderTargetsAndUnorderedAccessViews(
		1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.GetAddressOf(),
		1, 1, cached_unordered_access_view.GetAddressOf()
	);

	ID3D11RenderTargetView* nulll_render_target_view{};
	UINT initial_count{ 0 };
	dc->OMSetRenderTargetsAndUnorderedAccessViews(
		1, &nulll_render_target_view, NULL,
		1, 1, particle_append_buffer_uav.GetAddressOf(), &initial_count
	);

	dc->PSSetShader(accumulate_husk_particles_ps.Get(), nullptr, 0);
	drawcallback();

	dc->OMSetRenderTargetsAndUnorderedAccessViews(
		1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.Get(),
		1, 1, cached_unordered_access_view.GetAddressOf(), NULL
	);

	dc->CopyStructureCount(particle_count_buffer.Get(), 0, particle_append_buffer_uav.Get());
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	hr = dc->Map(particle_count_buffer.Get(), 0, D3D11_MAP_READ, 0, &mapped_subresource);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	UINT count{ *reinterpret_cast<UINT*>(mapped_subresource.pData) };
	dc->Unmap(particle_count_buffer.Get(), 0);
	dc->CopyResource(updated_particle_buffer.Get(), particle_buffer.Get());

	particle_constants->data.particle_count = count;
}