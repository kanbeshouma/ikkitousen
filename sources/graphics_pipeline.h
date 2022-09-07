#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <map>
#include <mutex>
#include "misc.h"

#ifdef ENABLE_DIRECT2D
#include <d3d11_1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dwrite.lib")
#endif

enum class DEPTH_STENCIL
{
	DEON_DWON, DEOFF_DWON, DEON_DWOFF, DEOFF_DWOFF, DEPTH_STENCIL_COUNT,
};
enum class BLEND_STATE
{
	NO_PROCESS,
	ALPHA,
	ADD,
	SUBTRACTION,
	MULTIPLY,

	BLEND_STATE_COUNT,
};
enum class RASTERIZER_STATE
{
	SOLID,                   // SOLID, 面カリングなし, 時計回りが表
	CULL_NONE,               // SOLID, 面カリングあり
	SOLID_COUNTERCLOCKWISE,  // SOLID, 面カリングなし, 反時計回りが表
	WIREFRAME_CULL_BACK,     // WIREFRAME, 後ろ向きの三角形を描画しない
	WIREFRAME_CULL_NONE,     // WIREFRAME, 常にすべての三角形を描画する

	RASTERIZER_STATE_COUNT,
};
enum class SHADER_TYPES
{
	NONE = -1,
	SPECULAR,
	TOON,
	HALF_LAMBERT,
	PBR,
	POINT_LIGHT,
	SKYMAP,
	DEFAULT,
	LaserBeam,
	FIELD,
};

class GraphicsPipeline
{
public:
    //--------<constructor/destructor>--------//
    GraphicsPipeline() {}
    ~GraphicsPipeline() {}
    //--------< 関数 >--------//
    void initialize(HWND hwnd);
    //--------<getter/setter>--------//
	Microsoft::WRL::ComPtr<ID3D11Device> get_device() { return device; }
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> get_dc() { return device_context; }
#ifndef ENABLE_DIRECT2D
	Microsoft::WRL::ComPtr<IDXGISwapChain> get_swap_chain() { return swap_chain; }
#else
	Microsoft::WRL::ComPtr<IDXGISwapChain1> get_swap_chain() { return swap_chain; }
	Microsoft::WRL::ComPtr<IDWriteFactory> get_dwrite_factory() { return dwrite_factory; }
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> get_d2d1_device_context() { return d2d1_device_context; }
	Microsoft::WRL::ComPtr<IDWriteTextFormat> get_dwrite_text_format() { return dwrite_text_format; }
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> get_d2d_solid_color_brush() { return d2d_solid_color_brush; }
#endif
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> get_render_target_view() { return render_target_view; }
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> get_depth_stencil_view() { return depth_stencil_view; }
	Microsoft::WRL::ComPtr<ID3D11SamplerState> get_sampler_state(int index) { return sampler_states[index]; }
	std::mutex& get_mutex() { return mutex; }
	//--------<描画プリセットのセッター>--------//
	// シェーダー
	void set_pipeline_preset(SHADER_TYPES shader_type);
	// ラスタライザーステート
	void set_pipeline_preset(RASTERIZER_STATE rasterizer_state);
	// ラスタライザーステート、デプスステンシルステート
	void set_pipeline_preset(RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state);
	// ブレンドステート、ラスタライザーステート、デプスステンシルステート
	void set_pipeline_preset(BLEND_STATE blend_state, RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state);
	// ラスタライザーステート、デプスステンシルステート、シェーダー
	void set_pipeline_preset(RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state, SHADER_TYPES shader_type);
	// ブレンドステート、ラスタライザーステート、デプスステンシルステート、シェーダー
	void set_pipeline_preset(BLEND_STATE blend_state, RASTERIZER_STATE rasterizer_state, DEPTH_STENCIL depth_stencil_state, SHADER_TYPES shader_type);
private:
    //--------< 構造体 >--------//

    //--------< 変数 >--------//
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
#ifndef ENABLE_DIRECT2D
	Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
#else
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
#endif
#ifdef ENABLE_DIRECT2D
	Microsoft::WRL::ComPtr<IDWriteFactory> dwrite_factory;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1_device_context;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> dwrite_text_format;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2d_solid_color_brush;
#endif
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_states[4];
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states[static_cast<int>(BLEND_STATE::BLEND_STATE_COUNT)];
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[static_cast<int>(RASTERIZER_STATE::RASTERIZER_STATE_COUNT)];
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states[static_cast<int>(DEPTH_STENCIL::DEPTH_STENCIL_COUNT)];
	std::mutex mutex;
	//--shaders--//
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	std::map<SHADER_TYPES, Microsoft::WRL::ComPtr<ID3D11VertexShader>> vertex_shaders;
	std::map<SHADER_TYPES, Microsoft::WRL::ComPtr<ID3D11PixelShader>> pixel_shaders;
	//--maps--//
	std::map<SHADER_TYPES, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> texture_maps;
};