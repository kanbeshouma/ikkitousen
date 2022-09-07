#include <sstream>
#include <WICTextureLoader.h>
#include "misc.h"
#include "texture.h"
#include "shader.h"
#include "user.h"
#include "sprite_batch.h"

#ifdef USE_IMGUI
#include "../External/imgui/imgui.h"
#include "../External/imgui/imgui_internal.h"
#include "../External/imgui/imgui_impl_dx11.h"
#include "../External/imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

SpriteBatch::SpriteBatch(ID3D11Device* device, const wchar_t* filename, size_t max_sprites)
    : max_vertices(max_sprites * 6)
{
    HRESULT hr{ S_OK };

    std::unique_ptr<vertex[]> vertices{ std::make_unique<vertex[]>(max_vertices) };

    // ���_�o�b�t�@�I�u�W�F�N�g�̐���
    // GPU (�ǂݎ��̂�) �� CPU (�������݂̂�) �ɂ��A�N�Z�X���\�ɂ���
    {
        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = UINT(sizeof(vertex) * max_vertices);
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        buffer_desc.MiscFlags = 0;
        buffer_desc.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA subresource_data{};
        subresource_data.pSysMem = vertices.get();
        subresource_data.SysMemPitch = 0;
        subresource_data.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    // �萔�o�b�t�@�̍쐬
    {
        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(SpriteConstants);
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    // ���̓��C�A�E�g�I�u�W�F�N�g�̐���
    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,  0 },
    };
    // ���_�V�F�[�_�[�I�u�W�F�N�g�̐���
    const char* cso_name{ "shaders/sprite_vs.cso" };
    create_vs_from_cso(device, cso_name, vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
    // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�̐���
    const char* cso_name_ps{ "shaders/sprite_ps.cso" };
    create_ps_from_cso(device, cso_name_ps, pixel_shader.GetAddressOf());
    // �摜�t�@�C���̃��[�h�ƃV�F�[�_�[���\�[�X�r���[�I�u�W�F�N�g(ID3D11ShaderResourceView)�̐���
    load_texture_from_file(device, filename, shader_resource_view.GetAddressOf(), &texture2d_desc);
}

SpriteBatch::~SpriteBatch()
{
    release_all_textures();
}

void SpriteBatch::render(ID3D11DeviceContext* dc, float position_x, float position_y, float scale_x, float scale_y,
    float pivot_x, float pivot_y, float color_r, float color_g, float color_b, float color_a,
    float angle/*degree*/, float glow_horizon, float glow_vertical)
{
    // �X�N���[���i�r���[�|�[�g�j�̃T�C�Y���擾����
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    dc->RSGetViewports(&num_viewports, &viewport);

    // render�����o�֐��̈����idx, dy, dw, dh�j�����`�̊e���_�̈ʒu�i�X�N���[�����W�n�j���v�Z����
    //  (x0, y0) *----* (x1, y1)
    //           |   /|
    //           |  / |
    //           | /  |
    //           |/   |
    //  (x2, y2) *----* (x3, y3)

    // left-top
    float x0{ position_x - (pivot_x * scale_x) };
    float y0{ position_y - (pivot_y * scale_y) };
    // right-top
    float x1{ position_x + ((texture2d_desc.Width - pivot_x) * scale_x) };
    float y1{ position_y - (pivot_y * scale_y) };
    // left-bottom
    float x2{ position_x - (pivot_x * scale_x) };
    float y2{ position_y + ((texture2d_desc.Height - pivot_y) * scale_y) };
    // right-bottom
    float x3{ position_x + ((texture2d_desc.Width - pivot_x) * scale_x) };
    float y3{ position_y + ((texture2d_desc.Height - pivot_y) * scale_y) };


    //��]�̒��S����`�̒��S�_�ɂ����ꍇ
    float cx = position_x;
    float cy = position_y;
    Math::rotate(x0, y0, cx, cy, angle);
    Math::rotate(x1, y1, cx, cy, angle);
    Math::rotate(x2, y2, cx, cy, angle);
    Math::rotate(x3, y3, cx, cy, angle);


    // �X�N���[�����W�n����NDC�ւ̍��W�ϊ��������Ȃ�
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    vertices.push_back({ { x0, y0 , 0 }, { color_r, color_g, color_b, color_a }, { 0, 0 } });
    vertices.push_back({ { x1, y1 , 0 }, { color_r, color_g, color_b, color_a }, { 1, 0 } });
    vertices.push_back({ { x2, y2 , 0 }, { color_r, color_g, color_b, color_a }, { 0, 1 } });
    vertices.push_back({ { x2, y2 , 0 }, { color_r, color_g, color_b, color_a }, { 0, 1 } });
    vertices.push_back({ { x1, y1 , 0 }, { color_r, color_g, color_b, color_a }, { 1, 0 } });
    vertices.push_back({ { x3, y3 , 0 }, { color_r, color_g, color_b, color_a }, { 1, 1 } });


    // �萔�o�b�t�@�̍X�V
    SpriteConstants data{ {glow_horizon, glow_vertical, 0, 0} };
    dc->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
    dc->PSSetConstantBuffers(2, 1, constant_buffer.GetAddressOf());
}

void SpriteBatch::render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 pivot, DirectX::XMFLOAT4 color, float angle, float glow_horizon, float glow_vertical)
{
    render(dc, position.x, position.y, scale.x, scale.y, pivot.x, pivot.y, color.x,
        color.y, color.z, color.w, angle, glow_horizon, glow_vertical);
}

// �ǉ���������ϐ�sx, sy, sw, sh�̓e�N�Z�����W�n�ɂ������`�̍�����W(sx, sy)�ƃT�C�Y(sw, sh)�Ƃ���
// �e�N�Z�����W�n����e�N�X�`�����W�n�ւ̕ϊ��������Ȃ�
void SpriteBatch::render(ID3D11DeviceContext* dc, float position_x, float position_y, float scale_x, float scale_y,
    float pivot_x, float pivot_y, float color_r, float color_g, float color_b, float color_a, float angle,
    float texpos_x, float texpos_y, float texsize_w, float texsize_h, float glow_horizon, float glow_vertical)
{
    // �X�N���[���i�r���[�|�[�g�j�̃T�C�Y���擾����
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    dc->RSGetViewports(&num_viewports, &viewport);

    // render�����o�֐��̈����idx, dy, dw, dh�j�����`�̊e���_�̈ʒu�i�X�N���[�����W�n�j���v�Z����
    //  (x0, y0) *----* (x1, y1)
    //           |   /|
    //           |  / |
    //           | /  |
    //           |/   |
    //  (x2, y2) *----* (x3, y3)

    // left-top
    float x0{ position_x - (pivot_x * scale_x) };
    float y0{ position_y - (pivot_y * scale_y) };
    // right-top
    float x1{ position_x + ((fabsf(texsize_w) - pivot_x) * scale_x) };
    float y1{ position_y - (pivot_y * scale_y) };
    // left-bottom
    float x2{ position_x - (pivot_x * scale_x) };
    float y2{ position_y + ((fabsf(texsize_h) - pivot_y) * scale_y) };
    // right-bottom
    float x3{ position_x + ((fabsf(texsize_w) - pivot_x) * scale_x) };
    float y3{ position_y + ((fabsf(texsize_h) - pivot_y) * scale_y) };

    //��]�̒��S����`�̒��S�_�ɂ����ꍇ
    float cx = position_x;
    float cy = position_y;
    Math::rotate(x0, y0, cx, cy, angle);
    Math::rotate(x1, y1, cx, cy, angle);
    Math::rotate(x2, y2, cx, cy, angle);
    Math::rotate(x3, y3, cx, cy, angle);

    // Convert to NDC space
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    float u0{ texpos_x / texture2d_desc.Width };
    float v0{ texpos_y / texture2d_desc.Height };
    float u1{ (texpos_x + texsize_w) / texture2d_desc.Width };
    float v1{ (texpos_y + texsize_h) / texture2d_desc.Height };

    if (scale_x >= 0)
    {
        vertices.push_back({ { x0, y0 , 0 }, { color_r, color_g, color_b, color_a }, { u0, v0 } });
        vertices.push_back({ { x1, y1 , 0 }, { color_r, color_g, color_b, color_a }, { u1, v0 } });
        vertices.push_back({ { x2, y2 , 0 }, { color_r, color_g, color_b, color_a }, { u0, v1 } });
        vertices.push_back({ { x2, y2 , 0 }, { color_r, color_g, color_b, color_a }, { u0, v1 } });
        vertices.push_back({ { x1, y1 , 0 }, { color_r, color_g, color_b, color_a }, { u1, v0 } });
        vertices.push_back({ { x3, y3 , 0 }, { color_r, color_g, color_b, color_a }, { u1, v1 } });
    }
    else
    {
        vertices.push_back({ { x1, y1 , 0 }, { color_r, color_g, color_b, color_a }, { u1, v0 } });
        vertices.push_back({ { x0, y0 , 0 }, { color_r, color_g, color_b, color_a }, { u0, v0 } });
        vertices.push_back({ { x3, y3 , 0 }, { color_r, color_g, color_b, color_a }, { u1, v1 } });
        vertices.push_back({ { x3, y3 , 0 }, { color_r, color_g, color_b, color_a }, { u1, v1 } });
        vertices.push_back({ { x0, y0 , 0 }, { color_r, color_g, color_b, color_a }, { u0, v0 } });
        vertices.push_back({ { x2, y2 , 0 }, { color_r, color_g, color_b, color_a }, { u0, v1 } });
    }

    // �萔�o�b�t�@�̍X�V
    SpriteConstants data{ {glow_horizon, glow_vertical, 0, 0} };
    dc->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
    dc->PSSetConstantBuffers(2, 1, constant_buffer.GetAddressOf());
}

void SpriteBatch::render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 scale,
    DirectX::XMFLOAT2 pivot, DirectX::XMFLOAT4 color, float angle,
    DirectX::XMFLOAT2 texpos, DirectX::XMFLOAT2 texsize, float glow_horizon, float glow_vertical)
{
    render(dc, position.x, position.y, scale.x, scale.y, pivot.x, pivot.y,
        color.x, color.y, color.z, color.w, angle, texpos.x, texpos.y, texsize.x, texsize.y, glow_horizon, glow_vertical);
}

void SpriteBatch::render(ID3D11DeviceContext* dc, float position_x, float position_y, float scale_x, float scale_y, float glow_horizon, float glow_vertical)
{
    // �X�N���[���i�r���[�|�[�g�j�̃T�C�Y���擾����
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    dc->RSGetViewports(&num_viewports, &viewport);

    // left-top
    float x0{ position_x };
    float y0{ position_y };
    // right-top
    float x1{ position_x + (texture2d_desc.Width * scale_x) };
    float y1{ position_y };
    // left-bottom
    float x2{ position_x };
    float y2{ position_y + (texture2d_desc.Height * scale_y) };
    // right-bottom
    float x3{ position_x + (texture2d_desc.Width * scale_x) };
    float y3{ position_y + (texture2d_desc.Height * scale_y) };

    // Convert to NDC space
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    float u0{ 0 };
    float v0{ 0 };
    float u1{ 1 };
    float v1{ 1 };

    vertices.push_back({ { x0, y0 , 0 }, { 1.0f, 1.0f, 1.0f, 1.0f }, { u0, v0 } });
    vertices.push_back({ { x1, y1 , 0 }, { 1.0f, 1.0f, 1.0f, 1.0f }, { u1, v0 } });
    vertices.push_back({ { x2, y2 , 0 }, { 1.0f, 1.0f, 1.0f, 1.0f }, { u0, v1 } });
    vertices.push_back({ { x2, y2 , 0 }, { 1.0f, 1.0f, 1.0f, 1.0f }, { u0, v1 } });
    vertices.push_back({ { x1, y1 , 0 }, { 1.0f, 1.0f, 1.0f, 1.0f }, { u1, v0 } });
    vertices.push_back({ { x3, y3 , 0 }, { 1.0f, 1.0f, 1.0f, 1.0f }, { u1, v1 } });

    // �萔�o�b�t�@�̍X�V
    SpriteConstants data{ {glow_horizon, glow_vertical, 0, 0} };
    dc->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
    dc->PSSetConstantBuffers(2, 1, constant_buffer.GetAddressOf());

}

void SpriteBatch::render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 scale, float glow_horizon, float glow_vertical)
{
    render(dc, position.x, position.y, scale.x, scale.y, glow_horizon, glow_vertical);
}

void SpriteBatch::begin(ID3D11DeviceContext* dc)
{
    vertices.clear();
    dc->VSSetShader(vertex_shader.Get(), nullptr, 0);
    dc->PSSetShader(pixel_shader.Get(), nullptr, 0);
    dc->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
}

void SpriteBatch::end(ID3D11DeviceContext* dc)
{
    // �e�N�X�`�����W�𒸓_�o�b�t�@�ɃZ�b�g����
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = dc->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    size_t vertex_count = vertices.size();
    _ASSERT_EXPR(max_vertices >= vertex_count, "Buffer overflow");
    vertex* data{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
    if (data != nullptr)
    {
        const vertex* p = vertices.data();
        memcpy_s(data, max_vertices * sizeof(vertex), p, vertex_count * sizeof(vertex));
    }
    dc->Unmap(vertex_buffer.Get(), 0);

    // �V�F�[�_�[ ���\�[�X�̃o�C���h
    UINT stride{ sizeof(vertex) };
    UINT offset{ 0 };
    dc->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
    // �v���~�e�B�u�^�C�v����уf�[�^�̏����Ɋւ�����̃o�C���h
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // ���̓��C�A�E�g�I�u�W�F�N�g�̃o�C���h
    dc->IASetInputLayout(input_layout.Get());
    // �v���~�e�B�u�̕`��
    dc->Draw(static_cast<UINT>(vertex_count), 0);
}