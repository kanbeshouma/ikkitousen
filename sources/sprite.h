#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>

class Sprite
{
private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
    D3D11_TEXTURE2D_DESC texture2d_desc;

public:
    Sprite(ID3D11Device* device, const wchar_t* filename);
    ~Sprite();
    //--------<render�֐�>--------//
    // �ʒu�A�T�C�Y�A�p�x�A�`��F
    void render(ID3D11DeviceContext* dc,
        float position_x, float position_y, /* ��`�̍���̍��W�i�X�N���[�����W�n�j*/float size_w, float size_h/* ��`�̃T�C�Y�i�X�N���[�����W�n�j*/,
        float color_r, float color_g, float color_b, float color_a, float angle/*degree*/);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, float angle);
    // �ʒu�A�T�C�Y�A�p�x�A�`��F�A �؂���ʒu�A�؂���T�C�Y
    void render(ID3D11DeviceContext* dc, float position_x, float position_y, float size_w, float size_h,
        float color_r, float color_g, float color_b, float color_a, float angle,
        float texpos_x, float texpos_y, float texsize_w, float texsize_h);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size,
        DirectX::XMFLOAT4 color, float angle, DirectX::XMFLOAT2 texpos, DirectX::XMFLOAT2 texsize);
    // �ʒu�A�T�C�Y
    void render(ID3D11DeviceContext* dc, float position_x, float position_y, float size_w, float size_h);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size);
    // �t�H���g�摜���g�p���C�ӂ̕��������ʂɏo�͂��� ���t�H���g�摜�̓A�X�L�[�R�[�h���ɂP�U���P�U�̕������z�u���ꂽ�摜�t�@�C��
    void textout(ID3D11DeviceContext* dc, std::string s,
        float position_x, float position_y, float size_w, float size_h, float color_r, float color_g, float color_b, float color_a);
    void textout(ID3D11DeviceContext* dc, std::string s, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color);

private:
    // render�֐��̃I�[�o�[���C�h�ɂ���ďd�������R�[�h�̋��ʊ֐�
    void bind_renderer(ID3D11DeviceContext* dc);

private:
    // ���_�t�H�[�}�b�g
    struct vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 texcoord;
    };
};
