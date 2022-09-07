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
    //--------< �֐� >--------//
    void activate_shadowmap(GraphicsPipeline& graphics, const DirectX::XMFLOAT4 light_direction);
    void deactivate_shadowmap(GraphicsPipeline& graphics);
    void set_shadowmap(GraphicsPipeline& graphics);
    void clear_shadowmap(GraphicsPipeline& graphics);
    void debug_imgui();
private:
    //--------< �\���� >--------//
    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 view_projection;         //�r���[�E�v���W�F�N�V�����ϊ��s��
        DirectX::XMFLOAT4 light_direction;           //���C�g�̌���
        DirectX::XMFLOAT4 light_color;           //���C�g�̌���
        DirectX::XMFLOAT4 camera_position;
        DirectX::XMFLOAT4X4 shake_matrix;
    };
    std::unique_ptr<Constants<SceneConstants>> scene_constants;
    struct ShadowMapConstants
    {
        DirectX::XMFLOAT4X4 light_view_projection;	// ���C�g�̈ʒu���猩���ˉe�s��
        DirectX::XMFLOAT3	shadow_color;			// �e�F
        float				shadow_bias;			// �[�x�o�C�A�X

        int number_of_trials;                       // ���肪�e���ǂ����𒲂ׂ鎎�s��
        DirectX::XMINT3 pad1;
        float search_width_magnification;           // ��񓖂���̈ړ���
        DirectX::XMFLOAT3 pad2;
    };
    std::unique_ptr<Constants<ShadowMapConstants>> shadow_constants;
    //--------< �ϐ� >--------//
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