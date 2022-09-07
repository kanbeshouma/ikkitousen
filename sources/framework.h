#pragma once

#include "graphics_pipeline.h"
#include "high_resolution_timer.h"
#include "misc.h"
#include "practical_entities.h"
#include "scene_manager.h"
#include "debug_flags.h"

#include <sstream>
#include <tchar.h>
#include <Windows.h>
#include <wrl.h>
#ifdef USE_IMGUI
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

// ���s���uALT�v�{�uENTER�v�ł��X�N���[�����[�h�̐؂�ւ����s����
CONST LONG SCREEN_WIDTH{ 1280 };
CONST LONG SCREEN_HEIGHT{ 720 };
CONST BOOL FULLSCREEN{ FALSE };
CONST LPCWSTR APPLICATION_NAME{ L"��@���M" };


class framework : public PracticalEntities
{
public:
	CONST HWND hwnd;
	//--------<constructor/destructor>--------//
	framework(HWND hwnd);
	~framework();
	//--------<operator>--------//
	framework(const framework&) = delete;
	framework& operator=(const framework&) = delete;
	framework(framework&&) noexcept = delete;
	framework& operator=(framework&&) noexcept = delete;
	//--------< �֐� >--------//
	int run();
	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
private:
	bool initialize();
	void update(float elapsed_time/*Elapsed seconds from last frame*/);
	void render(float elapsed_time/*Elapsed seconds from last frame*/);
	bool uninitialize();
private:
	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float elapsed_time{ 0.0f };
	void calculate_frame_stats();
private:
	//--------< �ϐ� >--------//
	/*--------------------------------------------------------------------------------------------//
	//   COM�I�u�W�F�N�g                                                                           //
	//   https://docs.microsoft.com/ja-jp/uwp/cpp-ref-for-winrt/com-ptr                           //
	//	 https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nn-d3d11-id3d11device           //
	//											                                                  //
	//--------------------------------------------------------------------------------------------*/
	// imgui flags
#ifdef USE_IMGUI
	ImGuiWindowFlags Imflags_preset_1 = 0;
#endif
	// graphics_pipeline
	std::unique_ptr<GraphicsPipeline> graphics;
	// scene_manager
	std::unique_ptr<SceneManager> scene_manager;
	// DebugFlags
	std::unique_ptr<DebugFlags> debug_flags;
};