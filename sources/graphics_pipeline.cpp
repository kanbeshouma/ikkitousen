#include <vector>
#include <sstream>

#include "graphics_pipeline.h"
#include "shader.h"
#include "texture.h"

#include "framework.h"

void GraphicsPipeline::initialize(HWND hwnd)
{
	// デバイス・デバイスコンテキスト・スワップチェーンの作成
	HRESULT hr{ S_OK };
	UINT create_device_flags{ 0 };
#ifdef ENABLE_DIRECT2D
	create_device_flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // UNIT.99:Required to use Direct2D on DirectX11.
#endif
#ifdef _DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// デバイス、スワップチェーン
	{
#ifndef ENABLE_DIRECT2D
		D3D_FEATURE_LEVEL feature_levels{ D3D_FEATURE_LEVEL_11_0 };
		DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
		swap_chain_desc.BufferCount = 1;   // ダブルバッファリングになる(追加分を書く)
		swap_chain_desc.BufferDesc.Width = SCREEN_WIDTH;
		swap_chain_desc.BufferDesc.Height = SCREEN_HEIGHT;
		swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.OutputWindow = hwnd;
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.SampleDesc.Quality = 0;
		swap_chain_desc.Windowed = !FULLSCREEN;
		swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // フルスクリーン時のマウス座標補完
		// 第一引数の NULL はGPUの種類を指定する(NULLは何が動くか分からない)
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_device_flags,
			&feature_levels, 1, D3D11_SDK_VERSION, &swap_chain_desc,
			swap_chain.GetAddressOf(), device.GetAddressOf(), NULL, device_context.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		// レンダーターゲットビューの作成
		ID3D11Texture2D* back_buffer{};
		hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&back_buffer));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		hr = device->CreateRenderTargetView(back_buffer, NULL, render_target_view.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		back_buffer->Release();
#else
		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory1;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory1.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter1>> enum_adapters1;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter1;
		for (UINT adapter_index = 0; S_OK == dxgi_factory1->EnumAdapters1(adapter_index, dxgi_adapter1.ReleaseAndGetAddressOf()); ++adapter_index) {
			DXGI_ADAPTER_DESC1 adapter_desc_desc;
			dxgi_adapter1->GetDesc1(&adapter_desc_desc);
			OutputDebugStringW(std::wstring(std::to_wstring(adapter_index) + L':' + adapter_desc_desc.Description + L'\n').c_str());
			OutputDebugStringA(std::string("\tVendorId:" + std::to_string(adapter_desc_desc.VendorId) + '\n').c_str());
			OutputDebugStringA(std::string("\tDeviceId:" + std::to_string(adapter_desc_desc.DeviceId) + '\n').c_str());
			OutputDebugStringA(std::string("\tSubSysId:" + std::to_string(adapter_desc_desc.SubSysId) + '\n').c_str());
			OutputDebugStringA(std::string("\tRevision:" + std::to_string(adapter_desc_desc.Revision) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedVideoMemory:" + std::to_string(adapter_desc_desc.DedicatedVideoMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedSystemMemory:" + std::to_string(adapter_desc_desc.DedicatedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tSharedSystemMemory:" + std::to_string(adapter_desc_desc.SharedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.HighPart:" + std::to_string(adapter_desc_desc.AdapterLuid.HighPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.LowPart:" + std::to_string(adapter_desc_desc.AdapterLuid.LowPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tFlags:" + std::to_string(adapter_desc_desc.Flags) + '\n').c_str());
			enum_adapters1.push_back(dxgi_adapter1);
		}
		size_t selected_adapter_index{ 0 };
		dxgi_adapter1 = enum_adapters1.at(selected_adapter_index);

		D3D_FEATURE_LEVEL feature_levels[]
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		D3D_FEATURE_LEVEL feature_level;
		hr = D3D11CreateDevice(dxgi_adapter1.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0, create_device_flags, feature_levels,
			_countof(feature_levels), D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), &feature_level, device_context.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		_ASSERT_EXPR(!(feature_level < D3D_FEATURE_LEVEL_11_0), L"Direct3D Feature Level 11 unsupported.");

		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
		hr = dxgi_adapter1.Get()->GetParent(IID_PPV_ARGS(&dxgi_factory2));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1{};
		swap_chain_desc1.Width = SCREEN_WIDTH;
		swap_chain_desc1.Height = SCREEN_HEIGHT;
		swap_chain_desc1.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swap_chain_desc1.Stereo = FALSE;
		swap_chain_desc1.SampleDesc.Count = 1;
		swap_chain_desc1.SampleDesc.Quality = 0;
		swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc1.BufferCount = 2;
		swap_chain_desc1.Scaling = DXGI_SCALING_STRETCH;
		swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swap_chain_desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		hr = dxgi_factory2->CreateSwapChainForHwnd(device.Get(), hwnd, &swap_chain_desc1, nullptr, nullptr, swap_chain.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> frame_buffer;
		hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(&frame_buffer));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		hr = device->CreateRenderTargetView(frame_buffer.Get(), nullptr, render_target_view.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		Microsoft::WRL::ComPtr<IDXGIDevice2> dxgi_device2;
		hr = device.As(&dxgi_device2);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		Microsoft::WRL::ComPtr<ID2D1Factory1> d2d_factory1;
		D2D1_FACTORY_OPTIONS factory_options{};
#ifdef _DEBUG
		factory_options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory_options, d2d_factory1.GetAddressOf());

		Microsoft::WRL::ComPtr<ID2D1Device> d2d_device;
		hr = d2d_factory1->CreateDevice(dxgi_device2.Get(), d2d_device.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d1_device_context.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = dxgi_device2->SetMaximumFrameLatency(1);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		Microsoft::WRL::ComPtr<IDXGISurface2> dxgi_surface2;
		hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgi_surface2));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2d_bitmap1;
		hr = d2d1_device_context->CreateBitmapFromDxgiSurface(dxgi_surface2.Get(),
			D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), d2d_bitmap1.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		d2d1_device_context->SetTarget(d2d_bitmap1.Get());

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(dwrite_factory.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// フォント
		hr = dwrite_factory->CreateTextFormat(L"GEORGIA", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 48, L"", dwrite_text_format.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		// 文字の表示方法
		hr = dwrite_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		// 文字の色
		hr = d2d1_device_context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OldLace), d2d_solid_color_brush.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#endif
	}
	// 深度ステンシルビュー
	{
		ID3D11Texture2D* depth_stencil_buffer{};
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		texture2d_desc.Width = SCREEN_WIDTH;
		texture2d_desc.Height = SCREEN_HEIGHT;
		texture2d_desc.MipLevels = 1;
		texture2d_desc.ArraySize = 1;
		texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;
		hr = device->CreateTexture2D(&texture2d_desc, NULL, &depth_stencil_buffer);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
		depth_stencil_view_desc.Format = texture2d_desc.Format;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(depth_stencil_buffer, &depth_stencil_view_desc, depth_stencil_view.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		depth_stencil_buffer->Release();
	}
	// ビューポートの設定
	{
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(SCREEN_WIDTH);
		viewport.Height = static_cast<float>(SCREEN_HEIGHT);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		device_context->RSSetViewports(1, &viewport);
	}
	// サンプラーステートオブジェクト
	{
		D3D11_SAMPLER_DESC sampler_desc;
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.MaxAnisotropy = 16;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampler_desc.BorderColor[0] = 0.0;
		sampler_desc.BorderColor[1] = 0.0;
		sampler_desc.BorderColor[2] = 0.0;
		sampler_desc.BorderColor[3] = 0.0;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = device->CreateSamplerState(&sampler_desc, sampler_states[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		hr = device->CreateSamplerState(&sampler_desc, sampler_states[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
		hr = device->CreateSamplerState(&sampler_desc, sampler_states[2].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		// UNIT32 画面の上下左右の端にアーティファクトノイズがでる
        // ※サンプラーステートのD3D11_TEXTURE_ADDRESS_WRAPが原因、
        // D3D11_TEXTURE_ADDRESS_BORDERに変更（BorderColor値に注意せよ）
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampler_desc.BorderColor[0] = 0;
		sampler_desc.BorderColor[1] = 0;
		sampler_desc.BorderColor[2] = 0;
		sampler_desc.BorderColor[3] = 0;
		hr = device->CreateSamplerState(&sampler_desc, sampler_states[3].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	// 深度ステンシルステートオブジェクト
	// ※下記コードは深度テスト：オン、深度ライト：オンの設定である
	// ※深度テスト：オン・オフ、深度ライト：オン・オフの全４通りのオブジェクトを生成する
	{
		auto r_set_DepthEnable_and_DepthWriteMask = [&](int DepthEnable,
			const D3D11_DEPTH_WRITE_MASK& DepthWriteMask, const DEPTH_STENCIL index)
		{
			D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
			depth_stencil_desc.DepthEnable = DepthEnable;     // zテスト（深度テスト）  ON:TRUE OFF:FALSE
			depth_stencil_desc.DepthWriteMask = DepthWriteMask;  // Z-Write (深度ライト)  ON:ALL OFF:ZERO
			depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<int>(index)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		};
		// 深度テスト:ON       深度ライト：ON
		r_set_DepthEnable_and_DepthWriteMask(TRUE, D3D11_DEPTH_WRITE_MASK_ALL, DEPTH_STENCIL::DEON_DWON);
		// 深度テスト:OFF      深度ライト：ON
		r_set_DepthEnable_and_DepthWriteMask(FALSE, D3D11_DEPTH_WRITE_MASK_ALL, DEPTH_STENCIL::DEOFF_DWON);
		//   深度テスト:ON     深度ライト：OFF
		r_set_DepthEnable_and_DepthWriteMask(TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, DEPTH_STENCIL::DEON_DWOFF);
		// 深度テスト:OFF     深度ライト：OFF
		r_set_DepthEnable_and_DepthWriteMask(FALSE, D3D11_DEPTH_WRITE_MASK_ZERO, DEPTH_STENCIL::DEOFF_DWOFF);
	}
	// ブレンディングステートオブジェクト
	{
		auto r_set_blend_mode = [&](int blend_enable, const D3D11_BLEND& src_blend, const D3D11_BLEND& dest_blend,
			const D3D11_BLEND_OP& blend_op, const D3D11_BLEND& src_blend_alpha, const D3D11_BLEND& dest_blend_alpha, const BLEND_STATE& index)
		{
			D3D11_BLEND_DESC blend_desc{};
			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = FALSE;
			blend_desc.RenderTarget[0].BlendEnable = blend_enable;
			blend_desc.RenderTarget[0].SrcBlend = src_blend;
			blend_desc.RenderTarget[0].DestBlend = dest_blend;
			blend_desc.RenderTarget[0].BlendOp = blend_op;
			blend_desc.RenderTarget[0].SrcBlendAlpha = src_blend_alpha;
			blend_desc.RenderTarget[0].DestBlendAlpha = dest_blend_alpha;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<int>(index)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		};
		// なし
		r_set_blend_mode(FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, BLEND_STATE::NO_PROCESS);
		// 通常（アルファブレンド)
		r_set_blend_mode(TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, BLEND_STATE::ALPHA);
		// 加算(透過あり)
		r_set_blend_mode(TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, BLEND_STATE::ADD);
		// 減算
		r_set_blend_mode(TRUE, D3D11_BLEND_ZERO, D3D11_BLEND_INV_SRC_COLOR,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, BLEND_STATE::SUBTRACTION);
		// 乗算
		r_set_blend_mode(TRUE, D3D11_BLEND_ZERO, D3D11_BLEND_SRC_COLOR,
			D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ZERO, BLEND_STATE::MULTIPLY);
	}
	// ラスタライザステートオブジェクト
	{
		D3D11_RASTERIZER_DESC rasterizer_desc{};
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;

		auto r_set_RasterizerState = [&](const D3D11_FILL_MODE& fill_mode, const D3D11_CULL_MODE& cull_mode,
			int antialiased_line_enable, int counterclockwise, const RASTERIZER_STATE& index)
		{
			rasterizer_desc.FillMode = fill_mode;
			rasterizer_desc.CullMode = cull_mode;
			rasterizer_desc.AntialiasedLineEnable = antialiased_line_enable;
			rasterizer_desc.FrontCounterClockwise = counterclockwise;
			hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<int>(index)].GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		};
		// SOLID
		r_set_RasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, FALSE, RASTERIZER_STATE::SOLID);
		// CULL_NONE
		r_set_RasterizerState(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE, FALSE, RASTERIZER_STATE::CULL_NONE);
		// SOLID(反時計回り)
		r_set_RasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, TRUE, RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE);
		// WIREFRAME_CULL_BACK
		r_set_RasterizerState(D3D11_FILL_WIREFRAME, D3D11_CULL_BACK, TRUE, FALSE, RASTERIZER_STATE::WIREFRAME_CULL_BACK);
		// WIREFRAME_CULL_NONE
		r_set_RasterizerState(D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, TRUE, FALSE, RASTERIZER_STATE::WIREFRAME_CULL_NONE);
	}
	// シェーダーオブジェクト
	{
		D3D11_INPUT_ELEMENT_DESC input_element_desc[]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		// VS
		{
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
			// NONE SPECULAR TOON HALF_LAMBERT PBR POINT_LIGHT DEFAULT FIELD
			create_vs_from_cso(device.Get(), "./shaders/skinned_mesh_vs.cso", vertex_shader.ReleaseAndGetAddressOf(),
				input_layout.ReleaseAndGetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::NONE, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::SPECULAR, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::TOON, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::HALF_LAMBERT, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::PBR, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::POINT_LIGHT, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::DEFAULT, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::LaserBeam, vertex_shader));
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::FIELD, vertex_shader));
			// SKYMAP
			create_vs_from_cso(device.Get(), "./shaders/skymap_vs.cso", vertex_shader.ReleaseAndGetAddressOf(), nullptr, nullptr, 0);
			vertex_shaders.insert(std::make_pair(SHADER_TYPES::SKYMAP, vertex_shader));
		}
		// PS
		{
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
			// SPECULAR
			create_ps_from_cso(device.Get(), "./shaders/skinned_mesh_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::SPECULAR, pixel_shader));
			// TOON
			create_ps_from_cso(device.Get(), "./shaders/toon_shader_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::TOON, pixel_shader));
			// HALF_LAMBERT
			create_ps_from_cso(device.Get(), "./shaders/half_lambert_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::HALF_LAMBERT, pixel_shader));
			// PBR
			create_ps_from_cso(device.Get(), "./shaders/PBR_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::PBR, pixel_shader));
			// POINT_LIGHT
			create_ps_from_cso(device.Get(), "./shaders/point_light_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::POINT_LIGHT, pixel_shader));
			// SKYMAP
			create_ps_from_cso(device.Get(), "./shaders/skymap_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::SKYMAP, pixel_shader));
			// DEFAULT
			create_ps_from_cso(device.Get(), "./shaders/default_mesh_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::DEFAULT, pixel_shader));
			// LaserBeam
			create_ps_from_cso(device.Get(), "./shaders/LaserBeamPs.cso",
				pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::LaserBeam, pixel_shader));
			// FIELD
			create_ps_from_cso(device.Get(), "./shaders/field_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
			pixel_shaders.insert(std::make_pair(SHADER_TYPES::FIELD, pixel_shader));
		}
	}
	//----texture maps----//
	{
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		// toon_map
		{
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
			const wchar_t* toon_map_filename = L".\\resources\\TexMaps\\toon_maps\\toonMap.png";
			load_texture_from_file(device.Get(), toon_map_filename, srv.GetAddressOf(), &texture2d_desc);
			texture_maps.insert(std::make_pair(SHADER_TYPES::TOON, srv));
		}
		// skymap
		{
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
			const wchar_t* skymap_filename = L".\\resources\\TexMaps\\skymaps\\envmap_miramar.dds";
			load_texture_from_file(device.Get(), skymap_filename, srv.GetAddressOf(), &texture2d_desc);
			texture_maps.insert(std::make_pair(SHADER_TYPES::SKYMAP, srv));
		}
	}
}

void GraphicsPipeline::set_pipeline_preset(SHADER_TYPES shader_type)
{
	//--シェーダーオブジェクト--//
	device_context->IASetInputLayout(input_layout.Get());
	// VS
	if(vertex_shaders.count(shader_type)) device_context->VSSetShader(vertex_shaders.at(shader_type).Get(), nullptr, 0);
	// PS
	if (texture_maps.count(shader_type)) device_context->PSSetShaderResources(7, 1, texture_maps.at(shader_type).GetAddressOf());
	if (pixel_shaders.count(shader_type)) device_context->PSSetShader(pixel_shaders.at(shader_type).Get(), nullptr, 0);
}

void GraphicsPipeline::set_pipeline_preset(RASTERIZER_STATE rasterizer_state)
{
	// ラスタライザステート
	device_context->RSSetState(rasterizer_states[static_cast<int>(rasterizer_state)].Get());
}

void GraphicsPipeline::set_pipeline_preset(RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state)
{
	// ラスタライザステート
	device_context->RSSetState(rasterizer_states[static_cast<int>(rasterizer_state)].Get());
	// 深度ステンシルステートオブジェクト
	device_context->OMSetDepthStencilState(depth_stencil_states[static_cast<int>(depth_stencil_state)].Get(), 1);
}

void GraphicsPipeline::set_pipeline_preset(BLEND_STATE blend_state, RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state)
{
	// ブレンディングステートオブジェクト
	device_context->OMSetBlendState(blend_states[static_cast<int>(blend_state)].Get(), nullptr, 0xFFFFFFFF);
	// ラスタライザステート
	device_context->RSSetState(rasterizer_states[static_cast<int>(rasterizer_state)].Get());
	// 深度ステンシルステートオブジェクト
	device_context->OMSetDepthStencilState(depth_stencil_states[static_cast<int>(depth_stencil_state)].Get(), 1);
}

void GraphicsPipeline::set_pipeline_preset(BLEND_STATE blend_state,
	RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state, SHADER_TYPES shader_type)
{
	// ブレンディングステートオブジェクト
	device_context->OMSetBlendState(blend_states[static_cast<int>(blend_state)].Get(), nullptr, 0xFFFFFFFF);
	// ラスタライザステート
	device_context->RSSetState(rasterizer_states[static_cast<int>(rasterizer_state)].Get());
	// 深度ステンシルステートオブジェクト
	device_context->OMSetDepthStencilState(depth_stencil_states[static_cast<int>(depth_stencil_state)].Get(), 1);
	//--シェーダーオブジェクト--//
	device_context->IASetInputLayout(input_layout.Get());
	// VS
	if (vertex_shaders.count(shader_type)) device_context->VSSetShader(vertex_shaders.at(shader_type).Get(), nullptr, 0);
	// PS
	if (texture_maps.count(shader_type)) device_context->PSSetShaderResources(7, 1, texture_maps.at(shader_type).GetAddressOf());
	if (pixel_shaders.count(shader_type)) device_context->PSSetShader(pixel_shaders.at(shader_type).Get(), nullptr, 0);
}

void GraphicsPipeline::set_pipeline_preset(RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state, SHADER_TYPES shader_type)
{
	// ラスタライザステート
	device_context->RSSetState(rasterizer_states[static_cast<int>(rasterizer_state)].Get());
	// 深度ステンシルステートオブジェクト
	device_context->OMSetDepthStencilState(depth_stencil_states[static_cast<int>(depth_stencil_state)].Get(), 1);
	//--シェーダーオブジェクト--//
	device_context->IASetInputLayout(input_layout.Get());
	// VS
	if (vertex_shaders.count(shader_type)) device_context->VSSetShader(vertex_shaders.at(shader_type).Get(), nullptr, 0);
	// PS
	if (texture_maps.count(shader_type)) device_context->PSSetShaderResources(7, 1, texture_maps.at(shader_type).GetAddressOf());
	if (pixel_shaders.count(shader_type)) device_context->PSSetShader(pixel_shaders.at(shader_type).Get(), nullptr, 0);
}