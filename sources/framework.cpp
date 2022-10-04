#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

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
	// comobjects�̏�����(1�ԏ�ɂȂ�悤��)
	graphics = std::make_unique<GraphicsPipeline>();
	graphics->initialize(hwnd);
	// entities
	entities_initialize(*graphics);
	// �I�v�V�����̃��[�h
	VolumeFile::get_instance().load();
	GameFile::get_instance().load();

	// �V�[���̏�����
	scene_manager = std::make_unique<SceneManager>(*graphics);
	//scene_manager->set_next_scene(new SceneTitle());
	scene_manager->set_next_scene(new SceneLoading(new SceneTitle()));

	// debug_flags
	debug_flags = std::make_unique<DebugFlags>();

	// file��load
	VolumeFile::get_instance().load();
	GameFile::get_instance().load();


	return true;
}

void framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
#ifdef Telecommunications
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
	// device
	mouse->update(hwnd);
	game_pad->update(elapsed_time);
	mouse->operation_activation();
	game_pad->operation_activation();

	// �E�B���h�E���ŏ�������Ă���Ԃ̓Q�[�����~�߂�
	if (!IsIconic(hwnd))
	{
		// hit_stop
		hit_stop->update(*graphics, elapsed_time, *camera_shake);
		// �V�[���̃A�b�v�f�[�g
		if (!hit_stop->get_hit_stop()) scene_manager->update(*graphics, elapsed_time);
		// debug_flags
#ifdef _DEBUG
		debug_flags->update();
#endif // _DEBUG
	}
}


void framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{
	//�ʃX���b�h���Ƀf�o�C�X�R���e�L�X�g���g���Ă����ꍇ
    //�����A�N�Z�X�ł��Ȃ��悤�ɔr�����䂷��
	std::lock_guard<std::mutex> lock(graphics->get_mutex());

	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	graphics->get_dc()->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	graphics->get_dc()->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	graphics->get_dc()->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	HRESULT hr{ S_OK };

	// �V���h�E�}�b�v
	{
		// �V���h�E�}�b�v�̃V�F�[�_���\�[�X�r���[�����Ƃ���
		// �����_�[�^�[�Q�b�g�r���[��ݒ肵�Ă��Ȃ��̂Ńs�N�Z���V�F�[�_�[���Z�b�g����ƌx�����o��(�[�x�}�b�v�����l)
		scene_manager->register_shadowmap(*graphics, elapsed_time);
	}
	// �e�|�C���^�̃o�C���h
	{
		// �T���v���[�X�e�[�g�I�u�W�F�N�g���o�C���h����
		graphics->get_dc()->PSSetSamplers(0, 1, graphics->get_sampler_state(0).GetAddressOf());
		graphics->get_dc()->PSSetSamplers(1, 1, graphics->get_sampler_state(1).GetAddressOf());
		graphics->get_dc()->PSSetSamplers(2, 1, graphics->get_sampler_state(2).GetAddressOf());
		graphics->get_dc()->PSSetSamplers(3, 1, graphics->get_sampler_state(3).GetAddressOf());
		// �����_�[�^�[�Q�b�g�r���[�̃N���A
		FLOAT color[]{ 0.7f, 0.7f, 0.7f, 1.0f };
		graphics->get_dc()->ClearRenderTargetView(graphics->get_render_target_view().Get(), color);
		graphics->get_dc()->ClearDepthStencilView(graphics->get_depth_stencil_view().Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		graphics->get_dc()->OMSetRenderTargets(1, graphics->get_render_target_view().GetAddressOf(), graphics->get_depth_stencil_view().Get());
		// �`��X�e�[�g�ݒ�
		graphics->set_pipeline_preset(BLEND_STATE::ALPHA, RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEON_DWON);
	}

	// �E�B���h�E���ŏ�������Ă���Ԃ̓Q�[����`�悵�Ȃ�
	if (!IsIconic(hwnd))
	{
		// �V�[���̕`��
		scene_manager->render(*graphics, elapsed_time);
	}

	// DierctX2D �f��
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
	//Font::get_instance().text_out(L"ABCDEFG \n abcdefg \n ����������", count, { left, top ,right ,bottom });
	if (is_direct2d)
	{
		count += elapsed_time * string_speed;
		graphics->get_d2d1_device_context()->BeginDraw();
		static const wchar_t message[]{ L"Direct2D��GDI�AGDI+�A�����Direct3D�Ƒ��݉^�p���̂��鍂���������ׂ�2D�O���t�B�b�N�X��񋟂���API�ŁAWindows 7��Windows Server 2008 R2�ȍ~��Windows�Ɏ�������Ă���DirectX�̈ꕔ�ł���BDirect2D 1.0�Ɋւ��Ă�Windows Vista��Windows Server 2008�ł��v���b�g�t�H�[���X�V�v���O����KB971644��K�p���邱�Ƃŗ��p�\�ł���[1]�BWindows Vista�ȍ~�A�n�[�h�E�F�A �A�N�Z�����[�V�������p�~����ADirect3D��ł̃\�t�g�E�F�A�����ƂȂ��Ă��܂���GDI�A�����Windows XP�ȑO����\�t�g�E�F�A�����ł�����GDI+�̌�pAPI�Ƃ��Ĉʒu�Â����Ă���B" };
		//static const wchar_t message[]{ L"ABCDEFG \n abcdefg \n ����������" };
		if (count > _countof(message)) { count = 0; }
		graphics->get_d2d1_device_context()->DrawTextW(message, static_cast<UINT32>(count),
			graphics->get_dwrite_text_format().Get(), D2D1::RectF(left, top, right, bottom),
			graphics->get_d2d_solid_color_brush().Get(), D2D1_DRAW_TEXT_OPTIONS_DISABLE_COLOR_BITMAP_SNAPPING);
		hr = graphics->get_d2d1_device_context()->EndDraw();
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
#endif

#ifdef Telecommunications
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif

	// sync_interval��1���Z�b�g�����60FPS�̌Œ�t���[�����[�g�œ��삷��
	UINT sync_interval{ 0 };


	graphics->get_swap_chain()->Present(sync_interval, 0);
}

bool framework::uninitialize()
{
	// �V�[���̉��
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

#ifdef Telecommunications
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
	// imgui flags�̏�����
	{
		//Imflags_preset_1 |= ImGuiWindowFlags_NoMove;
	}
	ImGuiIO& io = ImGui::GetIO();
	//----�����Őݒ肵�Ă���ConfigFlags�͑S�̂ɓK�������----//
	// imgui �h�b�L���O�̃t���O    // Enable docking(available in imgui `docking` branch at the moment)
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// ���C���E�B���h�E�̊O�ɏo��  // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	io.ConfigViewportsNoDecoration = true; // imgui�E�B���h�E�����C���E�B���h�E�̊O�ɏo�����Ƃ��S��ʁA�ŏ������ł���: false
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

#if 0
			frame_time += tictoc.time_interval();
			if (frame_time > 1.0f / 60.0f)
			{
				calculate_frame_stats();
				update(tictoc.time_interval());
				// �z�C�[�����̃��Z�b�g
				mouse->reset_wheel();
				render(tictoc.time_interval());
				frame_time = 0.0f;
			}
#else

			calculate_frame_stats();
			update(tictoc.time_interval());
			// �z�C�[�����̃��Z�b�g
			mouse->reset_wheel();
			render(tictoc.time_interval());
#endif // 0

		}
	}

#ifdef Telecommunications
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
#ifdef Telecommunications
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
			mouse->set_z_delta(GET_WHEEL_DELTA_WPARAM(wparam)); // ��]��
			mouse->set_n_notch(mouse->get_z_delta() / WHEEL_DELTA); // �m�b�`��
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
		outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
		//outs << APPLICATION_NAME;
#endif // _DEBUG

		SetWindowTextW(hwnd, outs.str().c_str());

		frames = 0;
		elapsed_time += 1.0f;
	}
}