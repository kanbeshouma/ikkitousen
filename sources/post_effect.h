#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "framebuffer.h"
#include "fullscreen_quad.h"
#include "constants.h"

class PostEffect
{
public:
    //--------<constructor/destructor>--------//
    PostEffect(ID3D11Device* device);
    ~PostEffect();
    //--------< 関数 >--------//
    void begin(ID3D11DeviceContext* dc);
    void end(ID3D11DeviceContext* dc);
    void apply_an_effect(ID3D11DeviceContext* dc, float elapsed_time);
	void blit(ID3D11DeviceContext* dc);
	void scene_preview();

	static void clear_post_effect();
	static void dash_post_effect(ID3D11DeviceContext* dc, const DirectX::XMFLOAT3& pos);
	static void stage_choice_post_effect(ID3D11DeviceContext* dc, float divisions);
	static void lockon_post_effect(float scope, float alpha);
	static void title_post_effect(float power);
	static void boss_awakening_effect(const DirectX::XMFLOAT2& screen_pos, float power, float wipe_threshold = 0);
	static void wipe_effect(float threshold);
	static void color_filter(float hueShift, float saturation, float brightness);
	static void chronostasis_effect(float scope, float saturation);

	//--------<getter/setter>--------//
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& get_color_map() { return framebuffers[static_cast<int>(FRAME_BUFER_SLOT::OFF_SCREEN)]->get_color_map(); }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& get_depth_map() { return framebuffers[static_cast<int>(FRAME_BUFER_SLOT::OFF_SCREEN)]->get_depth_map(); }

private:
    //--------< 定数 >--------//
    enum class POST_EFFECT_TYPE {
		NONE,                       // なし
		BLUR,						// ブラー
		RGB_SHIFT,					// RGBシフト
		WHITE_NOISE,				// ホワイトノイズ
		LOW_RESOLUTION,				// モザイク(低解像度)
		SCAN_LINE,					// 走査線
		GAME_BOY,					// ゲームボーイ風
		BARREL_SHAPED,				// 樽状湾曲
		GLITCH,                     // グリッチ
		VIGNETTING,                 // ビネット
		DASH_BLUR,                  // ダッシュブラー
		LOCKON,                     // ロックオン
		LOCKON_CENTRAL,             // ロックオン真ん中
		ZOOM_RGB_SHIFT,             // RGB_SHIFT(ズーム)
		COLOR_FILTER,				//カラーフィルター
		WIPE,

		PST_EFC_COUNT               //この定数が最後に
	};
    enum class FRAME_BUFER_SLOT { OFF_SCREEN, LUMINANCE };
    //--------< 構造体 >--------//
	struct PostEffectConstants
	{
		// luminance
		DirectX::XMFLOAT4 luminance_min_max{ 0.6f, 0.8f, 0, 0 };
		DirectX::XMFLOAT4 luminance_dot_rgb{ 0.299f, 0.587f, 0.114f, 0 };
		// blur
		float blur_gaussian_sigma{ 1.0f };
		DirectX::XMFLOAT3 pad1;
		float blur_bloom_intensity{ 1.0f };
		DirectX::XMFLOAT3 pad2;
		// white_noise
		float white_noise_time{ 10.0f };
		DirectX::XMFLOAT3 pad3;
		// low_resolution
		float low_resolution{ 1.2f };
		float low_resolution_number_of_divisions{ 55.0f };
		DirectX::XMFLOAT2 pad4;
		// scan_line
		float scan_line_time{};
		DirectX::XMFLOAT3 pad5;
		// game_boy
		DirectX::XMFLOAT4 border_color1{ 0.612f, 0.725f, 0.086f, 0 };
		DirectX::XMFLOAT4 border_color2{ 0.549f, 0.667f, 0.078f, 0 };
		DirectX::XMFLOAT4 border_color3{ 0.188f, 0.392f, 0.188f, 0 };
		DirectX::XMFLOAT4 border_color4{ 0.063f, 0.247f, 0.063f, 0 };
		// barrel_shaped
		float barrel_shaped_distortion{ 0.2f };
		DirectX::XMFLOAT3 pad6;
		// glitch
		DirectX::XMFLOAT2 glitch_pase{ 0.5f, -0.5f };
		float glitch_step_value{ 400.0f };
		float glitch_time{};
		// vignette
		DirectX::XMFLOAT2 vignetting_pos{ 0.5f, 0.5f };
		float vignetting_scope{ 0.2f };
		float pad7;
		// dash_blur
		DirectX::XMFLOAT4 reference_position{ 0.5f, 0.5f, 0, 0 };
		float zoom_power{ 0.74f };
		int focus_detail{ 50 };
		DirectX::XMFLOAT2 pad8;
		// lockon
		float lockon_scope{ 0.2f };
		float lockon_thickness{0.005f};
		float lockon_alpha{0.5f};
		float pad9;
		DirectX::XMFLOAT3 lockon_color{ 1,1,1 };
		float pad10;
		// slashing attack
		float slashing_power = 0.0;
		DirectX::XMFLOAT3 pad11;
		// zoom rgb shift
		DirectX::XMFLOAT2 rgb_shift_target_point = { 0.5f,0.5f };
		float rgb_shift_zoom_power = 0;
		float pad12;
		// wipe
		float wipe_threshold = 0.0f;
		DirectX::XMFLOAT3 pad13;
		// color filter
		// 色相調整
		float hueShift = 1.0f;
		// 彩度調整
		float saturation = 1.0f;
		// 明度調整
		float brightness = 1.0f;
		float pad14;
	};
	std::unique_ptr<Constants<PostEffectConstants>> effect_constants;
    //--------< 関数 >--------//


    //--------< 変数 >--------//
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shaders[16];
	// framebufferの実体
	static const int FRAMEBUFFERS_COUNT = 16;
	std::unique_ptr<FrameBuffer> framebuffers[FRAMEBUFFERS_COUNT];
	// FullScreenQuadの実体
	std::unique_ptr<FullScreenQuad> bit_block_transfer;

	int last_pst_efc_index;
	bool display_effect_imgui = false;

	static PostEffectConstants effect_constants_for_preservation;
	static int post_effect_count;
	static int effect_type[FRAMEBUFFERS_COUNT - 2];
};