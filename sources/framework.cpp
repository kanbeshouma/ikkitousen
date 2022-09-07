#include "framework.h"
#include "scene_title.h"
#include "scene_Game.h"
#include "volume_icon.h"
#include "game_icon.h"
#include "scene_loading.h"

framework::framework(HWND hwnd)
	: hwnd(hwnd) {}

bool framework::initialize()
{
	// comobjectsの初期化(1番上になるように)
	graphics = std::make_unique<GraphicsPipeline>();
	graphics->initialize(hwnd);
	// entities
	entities_initialize(*graphics);
	// オプションのロード
	VolumeFile::get_instance().load();
	GameFile::get_instance().load();

	// シーンの初期化
	scene_manager = std::make_unique<SceneManager>(*graphics);
	//scene_manager->set_next_scene(new SceneTitle());
	scene_manager->set_next_scene(new SceneLoading(new SceneTitle()));

	// debug_flags
	debug_flags = std::make_unique<DebugFlags>();

	// fileのload
	VolumeFile::get_instance().load();
	GameFile::get_instance().load();


	return true;
}

void framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
	// device
	mouse->update(hwnd);
	game_pad->update(elapsed_time);
	mouse->operation_activation();
	game_pad->operation_activation();

	// ウィンドウが最小化されている間はゲームを止める
	if (!IsIconic(hwnd))
	{
		// hit_stop
		hit_stop->update(*graphics, elapsed_time, *camera_shake);
		// シーンのアップデート
		if (!hit_stop->get_hit_stop()) scene_manager->update(*graphics, elapsed_time);
		// debug_flags
#ifdef _DEBUG
		debug_flags->update();
#endif // _DEBUG
	}
}


void framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{
	//別スレッド中にデバイスコンテキストが使われていた場合
    //同時アクセスできないように排他制御する
	std::lock_guard<std::mutex> lock(graphics->get_mutex());

	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	graphics->get_dc()->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	graphics->get_dc()->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	graphics->get_dc()->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	HRESULT hr{ S_OK };

	// シャドウマップ
	{
		// シャドウマップのシェーダリソースビューを作るときは
		// レンダーターゲットビューを設定していないのでピクセルシェーダーをセットすると警告が出る(深度マップも同様)
		scene_manager->register_shadowmap(*graphics, elapsed_time);
	}
	// 各ポインタのバインド
	{
		// サンプラーステートオブジェクトをバインドする
		graphics->get_dc()->PSSetSamplers(0, 1, graphics->get_sampler_state(0).GetAddressOf());
		graphics->get_dc()->PSSetSamplers(1, 1, graphics->get_sampler_state(1).GetAddressOf());
		graphics->get_dc()->PSSetSamplers(2, 1, graphics->get_sampler_state(2).GetAddressOf());
		graphics->get_dc()->PSSetSamplers(3, 1, graphics->get_sampler_state(3).GetAddressOf());
		// sync_intervalに1をセットすると60FPSの固定フレームレートで動作する
		// レンダーターゲットビューのクリア
		FLOAT color[]{ 0.7f, 0.7f, 0.7f, 1.0f };
		graphics->get_dc()->ClearRenderTargetView(graphics->get_render_target_view().Get(), color);
		graphics->get_dc()->ClearDepthStencilView(graphics->get_depth_stencil_view().Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		graphics->get_dc()->OMSetRenderTargets(1, graphics->get_render_target_view().GetAddressOf(), graphics->get_depth_stencil_view().Get());
		// 描画ステート設定
		graphics->set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	}

	// ウィンドウが最小化されている間はゲームを描画しない
	if (!IsIconic(hwnd))
	{
		// シーンの描画
		scene_manager->render(*graphics, elapsed_time);
	}

	// DierctX2D デモ
#ifdef ENABLE_DIRECT2D
	static bool is_direct2d = false;
	static float string_speed = 10;
	static float left = 50;
	static float top = 50;
	static float right = SCREEN_WIDTH;
	static float bottom = SCREEN_HEIGHT;
	static float count{ 0 };
	static bool display_direct2d_imgui = false;
#ifdef USE_IMGUI
	imgui_menu_bar("contents", "Direct2D", display_direct2d_imgui);
	if (display_direct2d_imgui)
	{
		ImGui::Begin("Direct2D");
		ImGui::Checkbox("is_direct2d", &is_direct2d);
		if (is_direct2d)
		{
			ImGui::DragFloat("string_speed", &string_speed, 1.0f, 1.0f, 50.0f);
			ImGui::DragFloat("left", &left, 1.0f, 1.0f, SCREEN_WIDTH);
			ImGui::DragFloat("top", &top, 1.0f, 1.0f, SCREEN_HEIGHT);
			ImGui::DragFloat("right", &right, 1.0f, 1.0f, SCREEN_WIDTH);
			ImGui::DragFloat("bottom", &bottom, 1.0f, -SCREEN_HEIGHT, SCREEN_HEIGHT);
			ImGui::Text("count:%f", count);
		}
		ImGui::End();
	}
#endif
	//Font::get_instance().text_out(L"ABCDEFG \n abcdefg \n あいうえお", count, { left, top ,right ,bottom });
	if (is_direct2d)
	{
		count += elapsed_time * string_speed;
		graphics->get_d2d1_device_context()->BeginDraw();
		static const wchar_t message[]{ L"Direct2DはGDI、GDI+、およびDirect3Dと相互運用性のある高速かつ高精細な2Dグラフィックスを提供するAPIで、Windows 7とWindows Server 2008 R2以降のWindowsに実装されているDirectXの一部である。Direct2D 1.0に関してはWindows VistaとWindows Server 2008でもプラットフォーム更新プログラムKB971644を適用することで利用可能である[1]。Windows Vista以降、ハードウェア アクセラレーションが廃止され、Direct3D上でのソフトウェア実装となってしまったGDI、およびWindows XP以前からソフトウェア実装であったGDI+の後継APIとして位置づけられている。" };
		//static const wchar_t message[]{ L"ABCDEFG \n abcdefg \n あいうえお" };
		if (count > _countof(message)) { count = 0; }
		graphics->get_d2d1_device_context()->DrawTextW(message, static_cast<UINT32>(count),
			graphics->get_dwrite_text_format().Get(), D2D1::RectF(left, top, right, bottom),
			graphics->get_d2d_solid_color_brush().Get(), D2D1_DRAW_TEXT_OPTIONS_DISABLE_COLOR_BITMAP_SNAPPING);
		hr = graphics->get_d2d1_device_context()->EndDraw();
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
#endif

#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif

	UINT sync_interval{ 0 };
	graphics->get_swap_chain()->Present(sync_interval, 0);
}

bool framework::uninitialize()
{
	// シーンの解放
	if (scene_manager != nullptr) { scene_manager->clear(*graphics); }
	// entities
	entities_uninitialize(*graphics);
	return true;
}

framework::~framework() {}

int framework::run()
{
	MSG msg{};

	if (!initialize())
	{
		uninitialize();
		return 0;
	}

#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
	// imgui flagsの初期化
	{
		//Imflags_preset_1 |= ImGuiWindowFlags_NoMove;
	}
	ImGuiIO& io = ImGui::GetIO();
	//----ここで設定しているConfigFlagsは全体に適応される----//
	// imgui ドッキングのフラグ    // Enable docking(available in imgui `docking` branch at the moment)
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// メインウィンドウの外に出す  // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	io.ConfigViewportsNoDecoration = true; // imguiウィンドウをメインウィンドウの外に出したとき全画面、最小化ができる: false
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(graphics->get_device().Get(), graphics->get_dc().Get());
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsCyber();
#endif

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tictoc.tick();
			calculate_frame_stats();
			update(tictoc.time_interval());
			// ホイール情報のリセット
			mouse->reset_wheel();
			render(tictoc.time_interval());
		}
	}

#ifdef USE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

#if 1
	BOOL fullscreen = 0;
	graphics->get_swap_chain()->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		graphics->get_swap_chain()->SetFullscreenState(FALSE, 0);
	}
#endif

	return uninitialize() ? static_cast<int>(msg.wParam) : 0;
}

LRESULT framework::handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			//PostMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
	case WM_ENTERSIZEMOVE:
		tictoc.stop();
		break;
	case WM_EXITSIZEMOVE:
		tictoc.start();
		break;
	case WM_MOUSEWHEEL:
		if (!mouse->get_is_wheel())
		{
			mouse->set_is_wheel(true);
			mouse->set_z_delta(GET_WHEEL_DELTA_WPARAM(wparam)); // 回転量
			mouse->set_n_notch(mouse->get_z_delta() / WHEEL_DELTA); // ノッチ数
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}


	return 0;
}

void framework::calculate_frame_stats()
{
	if (++frames, (tictoc.time_stamp() - elapsed_time) >= 1.0f)
	{
		float fps = static_cast<float>(frames);
		std::wostringstream outs;
		outs.precision(6);

#ifdef _DEBUG
		LPCWSTR game_mode{ L"/Debug" };
		LPCWSTR wire_flame{ L"/Wire Frame Off" };
		LPCWSTR flat_debug{ L"/Debug 2D Off" };

		if (debug_flags->get_perspective_switching()) game_mode = L"/Debug";
		else game_mode = L"/Game";
		if (debug_flags->get_wireframe_switching()) wire_flame = L"/Wire Frame On";
		else wire_flame = L"/Wire Frame Off";
		if (debug_flags->get_debug_2D_switching()) flat_debug = L"/Debug 2D On";
		else flat_debug = L"/Debug 2D Off";

		outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)" << game_mode << wire_flame << flat_debug;
#else
		//outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
		outs << APPLICATION_NAME;
#endif // _DEBUG

		SetWindowTextW(hwnd, outs.str().c_str());

		frames = 0;
		elapsed_time += 1.0f;
	}
}