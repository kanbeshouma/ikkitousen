#include "framework.h"
#include "shadow_map.h"

#include "shader.h"
#include "texture.h"
#include "user.h"

CONST LONG SHADOWMAP_WIDTH{ 1128 };
CONST LONG SHADOWMAP_HEIGHT{ 1128 };
CONST float SHADOWMAP_DRAWRECT{ 20 };

ShadowMap::ShadowMap(GraphicsPipeline& graphics)
{
	HRESULT hr{ S_OK };
	// ライトから見たシーンの深度描画用のバッファ生成
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_buffer{};
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		texture2d_desc.Width              = SHADOWMAP_WIDTH;
		texture2d_desc.Height             = SHADOWMAP_HEIGHT;
		texture2d_desc.MipLevels          = 1;
		texture2d_desc.ArraySize          = 1;
		texture2d_desc.Format             = DXGI_FORMAT_R32_TYPELESS;
		texture2d_desc.SampleDesc.Count   = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage              = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texture2d_desc.CPUAccessFlags     = 0;
		texture2d_desc.MiscFlags          = 0;
		hr                                = graphics.get_device()->CreateTexture2D(&texture2d_desc, NULL, depth_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		//	深度ステンシルビュー生成
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
			depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
			depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_stencil_view_desc.Texture2D.MipSlice = 0;
			hr = graphics.get_device()->CreateDepthStencilView(depth_buffer.Get(),
				&depth_stencil_view_desc,
				shadowmap_depth_stencil_view.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		//	シェーダーリソースビュー生成
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
			shader_resource_view_desc.Format                    = DXGI_FORMAT_R32_FLOAT;
			shader_resource_view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
			shader_resource_view_desc.Texture2D.MipLevels       = 1;
			hr = graphics.get_device()->CreateShaderResourceView(depth_buffer.Get(),
				&shader_resource_view_desc,
				shadowmap_shader_resource_view.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		// サンプラステートの生成
		{
			D3D11_SAMPLER_DESC sampler_desc{};
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			sampler_desc.MipLODBias = 0;
			sampler_desc.MaxAnisotropy = 16;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			sampler_desc.BorderColor[0] = FLT_MAX;
			sampler_desc.BorderColor[1] = FLT_MAX;
			sampler_desc.BorderColor[2] = FLT_MAX;
			sampler_desc.BorderColor[3] = FLT_MAX;
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
			hr = graphics.get_device()->CreateSamplerState(&sampler_desc, shadowmap_sampler_state.GetAddressOf());
		}
	}
	//----定数バッファ----//
	{
		scene_constants = std::make_unique<Constants<SceneConstants>>(graphics.get_device().Get());
		shadow_constants = std::make_unique<Constants<ShadowMapConstants>>(graphics.get_device().Get());
	}
	//----シェーダー----//
	// シャドウマップ生成用シェーダー
	{
		D3D11_INPUT_ELEMENT_DESC input_element_desc[]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		create_vs_from_cso(graphics.get_device().Get(), "shaders/shadowmap_caster_vs.cso",
			shadowmap_caster_vertex_shader.GetAddressOf(),
			shadowmap_caster_input_layout.GetAddressOf(),
			input_element_desc, ARRAYSIZE(input_element_desc));
	}
}

ShadowMap::~ShadowMap() {}

void ShadowMap::activate_shadowmap(GraphicsPipeline& graphics, const DirectX::XMFLOAT4 light_direction)
{
	// シャドウマップ生成処理
	// シャドウマップ用の深度バッファに設定
	graphics.get_dc()->ClearDepthStencilView(shadowmap_depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	graphics.get_dc()->OMSetRenderTargets(0, nullptr, shadowmap_depth_stencil_view.Get());
	// ビューポートの設定
	{
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(SHADOWMAP_WIDTH);
		viewport.Height = static_cast<float>(SHADOWMAP_HEIGHT);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		graphics.get_dc()->RSSetViewports(1, &viewport);
	}
	// 各ステート設定
	{
		// 描画ステート設定
		graphics.set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	}
	// シェーダー設定
	{
		graphics.get_dc()->IASetInputLayout(shadowmap_caster_input_layout.Get());
		graphics.get_dc()->VSSetShader(shadowmap_caster_vertex_shader.Get(), nullptr, 0);
		graphics.get_dc()->PSSetShader(nullptr, nullptr, 0);
	}
	// ライトの位置から見た視線行列を生成
	{
		DirectX::XMVECTOR light_position = DirectX::XMLoadFloat4(&light_direction);
		light_position = DirectX::XMVectorScale(light_position, -50);
		DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH(light_position,
			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		// シャドウマップに描画したい範囲の射影行列を生成
		static DirectX::XMFLOAT2 near_far = { 0.1f, 200.0f };
#ifdef USE_IMGUI
		imgui_menu_bar("contents", "shadow map", display_shadowmap_imgui);
		if (display_shadowmap_imgui)
		{
			ImGui::Begin("shadow map");
			ImGui::DragFloat2("near_far", &near_far.x, 0.01f, 0.1f, 2000.0f);
			ImGui::End();
		}
#endif // USE_IMGUI
		DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(SHADOWMAP_DRAWRECT, SHADOWMAP_DRAWRECT,
			near_far.x, near_far.y);
		// ライトビュー行列を保存
		DirectX::XMStoreFloat4x4(&light_view_projection, V * P);
		// 定数バッファの更新
		{
			// カメラ関連
			{
				scene_constants->data.view_projection = light_view_projection;
				scene_constants->data.light_direction = light_direction;
				scene_constants->data.light_color     = { 1,1,1,1 };
				scene_constants->data.camera_position = { 0,0,0,0 };
				scene_constants->data.shake_matrix    = camera_shake->get_shake_matrix();
				scene_constants->bind(graphics.get_dc().Get(), 1, CB_FLAG::PS_VS);
			}
			// シャドウマップ
			{
				shadow_constants->data.light_view_projection      = light_view_projection;
				shadow_constants->data.shadow_color               = shadow_color;
				shadow_constants->data.shadow_bias                = shadow_bias;
				shadow_constants->data.number_of_trials           = number_of_trials;
				shadow_constants->data.search_width_magnification = search_width_magnification;
				shadow_constants->bind(graphics.get_dc().Get(), 6, CB_FLAG::PS_VS);
			}
		}
	}
}

void ShadowMap::deactivate_shadowmap(GraphicsPipeline& graphics)
{
	D3D11_VIEWPORT viewport{};
	UINT num_viewports{ 1 };
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	graphics.get_dc()->RSSetViewports(1, &viewport);
}

void ShadowMap::set_shadowmap(GraphicsPipeline& graphics)
{
	graphics.get_dc()->PSSetShaderResources(6, 1, shadowmap_shader_resource_view.GetAddressOf());
	graphics.get_dc()->PSSetSamplers(6, 1, shadowmap_sampler_state.GetAddressOf());
}

void ShadowMap::clear_shadowmap(GraphicsPipeline& graphics)
{
	// 不要になったのでシャドウマップの設定を外しておく。
	// そうしないと次回シャドウマップ生成時に警告が出てしまうため。
	ID3D11ShaderResourceView* clear_shader_resource_view[]{ nullptr };
	graphics.get_dc()->PSSetShaderResources(6, 1, clear_shader_resource_view);
}

void ShadowMap::debug_imgui()
{
#ifdef USE_IMGUI
	if (display_shadowmap_imgui)
	{
		ImGui::Begin("shadow map");
		ImGui::ColorEdit3("shadow_color", &shadow_color.x);
		ImGui::DragFloat("shadow_bias", &shadow_bias, 0.0001f, 0.0f, 1.0f);
		ImGui::DragInt("number_of_trials", &number_of_trials, 1, 0, 50);
		ImGui::DragFloat("search_width_magnification", &search_width_magnification, 0.001f, 0.0f, 1.0f);
		ImGui::Separator();
		ImGui::Text("shadow_map");
		ImGui::Image(shadowmap_shader_resource_view.Get(), { 192, 192 }, { 0, 0 }, { 1, 1 }, { 1, 1, 1, 1 });
		ImGui::End();
	}
#endif // USE_IMGUI
}