#include "bloom.h"
#include "texture.h"
#include "misc.h"
#include "imgui_include.h"
#include "user.h"

Bloom::Bloom(ID3D11Device* device, uint32_t width, uint32_t height) : FullScreenQuad(device)
{
    glow_extraction = std::make_unique<FrameBuffer>(device, width, height, FB_FLAG::COLOR);
	for (size_t downsampled_index = 0; downsampled_index < downsampled_count; ++downsampled_index)
	{
		gaussian_blur[downsampled_index][0] = std::make_unique<FrameBuffer>(device, width >> (downsampled_index + 4), height >> (downsampled_index + 4), FB_FLAG::COLOR);
		gaussian_blur[downsampled_index][1] = std::make_unique<FrameBuffer>(device, width >> (downsampled_index + 4), height >> (downsampled_index + 4), FB_FLAG::COLOR);
		gaussian_blur[downsampled_index][2] = std::make_unique<FrameBuffer>(device, width >> (downsampled_index + 4), height >> (downsampled_index + 4), FB_FLAG::COLOR);
	}
	create_ps_from_cso(device, "shaders/bloom_extraction_ps.cso", bloom_extraction_ps.GetAddressOf());
	create_ps_from_cso(device, "shaders/gaussian_blur_horizontal_ps.cso", gaussian_blur_horizontal_ps.GetAddressOf());
	create_ps_from_cso(device, "shaders/gaussian_blur_vertical_ps.cso", gaussian_blur_vertical_ps.GetAddressOf());
	create_ps_from_cso(device, "shaders/gaussian_blur_convolution_ps.cso", gaussian_blur_convolution_ps.GetAddressOf());
	create_ps_from_cso(device, "shaders/gaussian_blur_downsampling_ps.cso", gaussian_blur_downsampling_ps.GetAddressOf());
}

void Bloom::make(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* color_map)
{
	UINT viewport_count{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cached_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;
	dc->RSGetViewports(&viewport_count, cached_viewports);
	dc->OMGetRenderTargets(1, cached_render_target_view.ReleaseAndGetAddressOf(), cached_depth_stencil_view.ReleaseAndGetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> null_shader_resource_view;

	// ‹P“x’Šo
	glow_extraction->clear(dc, FB_FLAG::COLOR, { 0, 0, 0, 1 });
	glow_extraction->activate(dc);
	FullScreenQuad::blit(dc, &color_map, 0, 1, bloom_extraction_ps.Get());
	glow_extraction->deactivate(dc);

	static bool display_brightness_imgui = false;
	float ratio_percentage = 0.2f;
	{
#ifdef USE_IMGUI
		imgui_menu_bar("contents", "brightness", display_brightness_imgui);
		if (display_brightness_imgui)
		{
			ImGui::Begin("brightness");
			// ‰æ‘œ‚Ì•`‰æ
			ImGui::Image((void*)glow_extraction->get_color_map().Get(),
				ImVec2(1280.0f * ratio_percentage, 720.0f * ratio_percentage), ImVec2(0, 0), ImVec2(1, 1), { 1,1,1,1 });
			ImGui::End();
		}
#endif // USE_IMGUI
	}

	// Gaussian blur
	// Efficient Gaussian blur with linear sampling
	// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
	// ƒ_ƒEƒ“ƒTƒ“ƒvƒŠƒ“ƒO
	gaussian_blur[0][0]->clear(dc, FB_FLAG::COLOR, { 0, 0, 0, 1 });
	gaussian_blur[0][0]->activate(dc);
	FullScreenQuad::blit(dc, glow_extraction->get_color_map().GetAddressOf(), 0, 1, gaussian_blur_downsampling_ps.Get());
	gaussian_blur[0][0]->deactivate(dc);
	// ping-pong gaussian blur
	gaussian_blur[0][1]->clear(dc, FB_FLAG::COLOR, { 0, 0, 0, 1 });
	gaussian_blur[0][1]->activate(dc);
	FullScreenQuad::blit(dc, gaussian_blur[0][0]->get_color_map().GetAddressOf(), 0, 1, gaussian_blur_horizontal_ps.Get());
	gaussian_blur[0][1]->deactivate(dc);

	gaussian_blur[0][2]->clear(dc, FB_FLAG::COLOR, { 0, 0, 0, 1 });
	gaussian_blur[0][2]->activate(dc, FB_FLAG::COLOR);
	FullScreenQuad::blit(dc, gaussian_blur[0][1]->get_color_map().GetAddressOf(), 0, 1, gaussian_blur_vertical_ps.Get());
	gaussian_blur[0][2]->deactivate(dc);

	{
#ifdef USE_IMGUI
		if (display_brightness_imgui)
		{
			ImGui::Begin("brightness");
			// ‰æ‘œ‚Ì•`‰æ
			std::string s = "0";
			if (ImGui::TreeNode(s.c_str()))
			{
				ImGui::Image((void*)gaussian_blur[0][0]->get_color_map().Get(),
					ImVec2(1280.0f * ratio_percentage, 720.0f * ratio_percentage), ImVec2(0, 0), ImVec2(1, 1), { 1,1,1,1 });
				ImGui::Image((void*)gaussian_blur[0][1]->get_color_map().Get(),
					ImVec2(1280.0f * ratio_percentage, 720.0f * ratio_percentage), ImVec2(0, 0), ImVec2(1, 1), { 1,1,1,1 });
				ImGui::Image((void*)gaussian_blur[0][2]->get_color_map().Get(),
					ImVec2(1280.0f * ratio_percentage, 720.0f * ratio_percentage), ImVec2(0, 0), ImVec2(1, 1), { 1,1,1,1 });

				ImGui::TreePop();
			}
			ImGui::End();
		}
#endif // USE_IMGUI
	}

	for (size_t downsampled_index = 1; downsampled_index < downsampled_count; ++downsampled_index)
	{
		// downsampling
		gaussian_blur[downsampled_index][0]->clear(dc, FB_FLAG::COLOR, { 0, 0, 0, 1 });
		gaussian_blur[downsampled_index][0]->activate(dc);
		FullScreenQuad::blit(dc, gaussian_blur[downsampled_index - 1][0]->get_color_map().GetAddressOf(), 0, 1, gaussian_blur_downsampling_ps.Get());
		gaussian_blur[downsampled_index][0]->deactivate(dc);

		// ping-pong gaussian blur
		gaussian_blur[downsampled_index][1]->clear(dc, FB_FLAG::COLOR, { 0, 0, 0, 1 });
		gaussian_blur[downsampled_index][1]->activate(dc);
		FullScreenQuad::blit(dc, gaussian_blur[downsampled_index][0]->get_color_map().GetAddressOf(), 0, 1, gaussian_blur_horizontal_ps.Get());
		gaussian_blur[downsampled_index][1]->deactivate(dc);

		gaussian_blur[downsampled_index][2]->clear(dc, FB_FLAG::COLOR, { 0, 0, 0, 1 });
		gaussian_blur[downsampled_index][2]->activate(dc, FB_FLAG::COLOR);
		FullScreenQuad::blit(dc, gaussian_blur[downsampled_index][1]->get_color_map().GetAddressOf(), 0, 1, gaussian_blur_vertical_ps.Get());
		gaussian_blur[downsampled_index][2]->deactivate(dc);

		{
#ifdef USE_IMGUI
			if (display_brightness_imgui)
			{
				ImGui::Begin("brightness");
				// ‰æ‘œ‚Ì•`‰æ
				std::string s = std::to_string(downsampled_index);
				if (ImGui::TreeNode(s.c_str()))
				{
					ImGui::Image((void*)gaussian_blur[downsampled_index][0]->get_color_map().Get(),
						ImVec2(1280.0f * ratio_percentage, 720.0f * ratio_percentage), ImVec2(0, 0), ImVec2(1, 1), { 1,1,1,1 });
					ImGui::Image((void*)gaussian_blur[downsampled_index][1]->get_color_map().Get(),
						ImVec2(1280.0f * ratio_percentage, 720.0f * ratio_percentage), ImVec2(0, 0), ImVec2(1, 1), { 1,1,1,1 });
					ImGui::Image((void*)gaussian_blur[downsampled_index][2]->get_color_map().Get(),
						ImVec2(1280.0f * ratio_percentage, 720.0f * ratio_percentage), ImVec2(0, 0), ImVec2(1, 1), { 1,1,1,1 });

					ImGui::TreePop();
				}
				ImGui::End();
			}
#endif // USE_IMGUI
		}
	}

	dc->RSSetViewports(viewport_count, cached_viewports);
	dc->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.Get());
}

void Bloom::blit(ID3D11DeviceContext* dc)
{
	UINT viewport_count{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cached_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;
	dc->RSGetViewports(&viewport_count, cached_viewports);
	dc->OMGetRenderTargets(1, cached_render_target_view.ReleaseAndGetAddressOf(), cached_depth_stencil_view.ReleaseAndGetAddressOf());
	ID3D11ShaderResourceView* cached_shader_resource_views[downsampled_count];
	dc->PSGetShaderResources(0, downsampled_count, cached_shader_resource_views);

	std::vector<ID3D11ShaderResourceView*> shader_resource_views;
	for (size_t downsampled_index = 0; downsampled_index < downsampled_count; ++downsampled_index)
	{
		shader_resource_views.push_back(gaussian_blur[downsampled_index][2]->get_color_map().Get());
	}
	FullScreenQuad::blit(dc, shader_resource_views.data(), 0, downsampled_count, gaussian_blur_convolution_ps.Get());

	dc->RSSetViewports(viewport_count, cached_viewports);
	dc->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.Get());
	dc->PSSetShaderResources(0, downsampled_count, cached_shader_resource_views);
	for (ID3D11ShaderResourceView* cached_shader_resource_view : cached_shader_resource_views)
	{
		if (cached_shader_resource_view) cached_shader_resource_view->Release();
	}
}