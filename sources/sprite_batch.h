#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include <wrl.h>

class SpriteBatch
{
private:
    // ���_�t�H�[�}�b�g
    struct vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 texcoord;
    };
    // �萔�o�b�t�@
    struct SpriteConstants
    {
        DirectX::XMFLOAT4 threshold;
    };

    // �ϐ�
    const size_t max_vertices;
    std::vector<vertex> vertices;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
    D3D11_TEXTURE2D_DESC texture2d_desc;

public:
    //--�Q�b�^�[--//
    const ID3D11ShaderResourceView* get_shader_resource_view() const { return shader_resource_view.Get(); }
    const D3D11_TEXTURE2D_DESC& get_texture2d_desc() const { return texture2d_desc; }

    //--�R���X�g���N�^/�֐�--//
    SpriteBatch(ID3D11Device* device, const wchar_t* filename, size_t max_sprites);
    ~SpriteBatch();

    //--------<render�֐�>--------//
    // �ʒu�A�T�C�Y�A�p�x�A�`��F
    void render(ID3D11DeviceContext* dc, float position_x, float position_y, float scale_x, float scale_y,
        float pivot_x, float pivot_y, float color_r, float color_g, float color_b, float color_a,
        float angle/*degree*/, float glow_horizon = 0.0f, float glow_vertical = 0.0f);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position,
        DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 pivot, DirectX::XMFLOAT4 color,
        float angle, float glow_horizon = 0.0f, float glow_vertical = 0.0f);
    // �ʒu�A�T�C�Y�A�p�x�A�`��F�A �؂���ʒu�A�؂���T�C�Y
    void render(ID3D11DeviceContext* dc, float position_x, float position_y, float scale_x, float scale_y,
        float pivot_x, float pivot_y, float color_r, float color_g, float color_b, float color_a, float angle,
        float texpos_x, float texpos_y, float texsize_w, float texsize_h, float glow_horizon = 0.0f, float glow_vertical = 0.0f);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 scale,
        DirectX::XMFLOAT2 pivot, DirectX::XMFLOAT4 color, float angle,
        DirectX::XMFLOAT2 texpos, DirectX::XMFLOAT2 texsize, float glow_horizon = 0.0f, float glow_vertical = 0.0f);
    // �ʒu�A�T�C�Y
    // ��ʏ�̕`��ʒu�ƃT�C�Y�̎w��݂̂Ńe�N�X�`���S�̂�`�悷��
    void render(ID3D11DeviceContext* dc, float position_x, float position_y, float scale_x, float scale_y, float glow_horizon = 0.0f, float glow_vertical = 0.0f);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 scale, float glow_horizon = 0.0f, float glow_vertical = 0.0f);

    void begin(ID3D11DeviceContext* dc);
    void end(ID3D11DeviceContext* dc);
};