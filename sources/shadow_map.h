#pragma once

#include <wrl.h>
#include <d3d11.h>
#include "constants.h"

class ShadowMap : public PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    ShadowMap(GraphicsPipeline& graphics);
    ~ShadowMap();
    //--------< 関数 >--------//
    void activate_shadowmap(GraphicsPipeline& graphics, const DirectX::XMFLOAT4 light_direction);
    void deactivate_shadowmap(GraphicsPipeline& graphics);
    void set_shadowmap(GraphicsPipeline& graphics);
    void clear_shadowmap(GraphicsPipeline& graphics);
    void debug_imgui();
private:
    //--------< 構造体 >--------//
    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 view_projection;         //ビュー・プロジェクション変換行列
        DirectX::XMFLOAT4 light_direction;           //ライトの向き
        DirectX::XMFLOAT4 light_color;           //ライトの向き
        DirectX::XMFLOAT4 camera_position;
        DirectX::XMFLOAT4X4 shake_matrix;
    };
    std::unique_ptr<Constants<SceneConstants>> scene_constants;
    struct ShadowMapConstants
    {
        DirectX::XMFLOAT4X4 light_view_projection;	// ライトの位置から見た射影行列
        DirectX::XMFLOAT3	shadow_color;			// 影色
        float				shadow_bias;			// 深度バイアス

        int number_of_trials;                       // 周りが影かどうかを調べる試行回数
        DirectX::XMINT3 pad1;
        float search_width_magnification;           // 一回当たりの移動幅
        DirectX::XMFLOAT3 pad2;
    };
    std::unique_ptr<Constants<ShadowMapConstants>> shadow_constants;
    //--------< 変数 >--------//
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowmap_depth_stencil_view;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowmap_shader_resource_view;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowmap_sampler_state;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> shadowmap_caster_vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> shadowmap_caster_input_layout;

    DirectX::XMFLOAT4X4 light_view_projection;
    DirectX::XMFLOAT3	shadow_color{ 0.3f, 0.3f, 0.3f };
    float				shadow_bias{ 0.003f };

    int number_of_trials = 10;
    float search_width_magnification = 0.001f;
    bool display_shadowmap_imgui = false;
};