#include <memory>

#include "debug_2D.h"
#include "shader.h"
#include "misc.h"
#include "user.h"
#include "debug_flags.h"
#include "operators.h"

Debug2D::Debug2D(ID3D11Device* device)
{
    HRESULT hr{ S_OK };
    // vs
    {
        // 入力レイアウト
        D3D11_INPUT_ELEMENT_DESC input_element_desc[]
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
              D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
              D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        create_vs_from_cso(device, "shaders/debug_2D_vs.cso", vertex_shader.GetAddressOf(),
            input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
    }
    // ps
    {
        create_ps_from_cso(device, "shaders/debug_2D_ps.cso", pixel_shader.GetAddressOf());
    }
    create_rect_vertex(device);
}

void Debug2D::all_render(ID3D11DeviceContext* context)
{
#ifdef _DEBUG
    // スクリーン（ビューポート）のサイズを取得する
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    context->RSGetViewports(&num_viewports, &viewport);
    viewport_width = viewport.Width;
    viewport_height = viewport.Height;
    // シェーダー設定
    context->VSSetShader(vertex_shader.Get(), nullptr, 0);
    context->PSSetShader(pixel_shader.Get(), nullptr, 0);
    context->IASetInputLayout(input_layout.Get());
    // rect描画
    {
        HRESULT hr{ S_OK };
        D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
        hr = context->Map(rect_vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        size_t vertex_count = rect_vertices.size();
        _ASSERT_EXPR(rect_max_vertices >= vertex_count, "Buffer overflow");
        RectVertex* data{ reinterpret_cast<RectVertex*>(mapped_subresource.pData) };
        if (data != nullptr)
        {
            const RectVertex* p = rect_vertices.data();
            memcpy_s(data, rect_max_vertices * sizeof(RectVertex), p, vertex_count * sizeof(RectVertex));
        }
        context->Unmap(rect_vertex_buffer.Get(), 0);

        UINT stride{ sizeof(RectVertex) };
        UINT offset{ 0 };
        context->IASetVertexBuffers(0, 1, rect_vertex_buffer.GetAddressOf(), &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        if (DebugFlags::get_debug_2D_switching()) context->Draw(static_cast<UINT>(vertex_count), 0);
        rect_vertices.clear();
    }
#endif // _DEBUG
}

void Debug2D::create_rect(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& radius, const DirectX::XMFLOAT3& color)
{
#ifdef _DEBUG
    DirectX::XMFLOAT2 left_top     = { position - radius };
    DirectX::XMFLOAT2 right_top    = { position.x + radius.x,  position.y - radius.y };
    DirectX::XMFLOAT2 left_bottom  = { position.x - radius.x,  position.y + radius.y };
    DirectX::XMFLOAT2 right_bottom = { position + radius };
    left_top.x     = 2.0f * left_top.x / viewport_width - 1.0f;
    left_top.y     = 1.0f - 2.0f * left_top.y / viewport_height;
    right_top.x    = 2.0f * right_top.x / viewport_width - 1.0f;
    right_top.y    = 1.0f - 2.0f * right_top.y / viewport_height;
    left_bottom.x  = 2.0f * left_bottom.x / viewport_width - 1.0f;
    left_bottom.y  = 1.0f - 2.0f * left_bottom.y / viewport_height;
    right_bottom.x = 2.0f * right_bottom.x / viewport_width - 1.0f;
    right_bottom.y = 1.0f - 2.0f * right_bottom.y / viewport_height;

    rect_vertices.push_back({ { left_top.x,     left_top.y,     0 }, { color.x, color.y, color.z, 0.5f } }); // 左上
    rect_vertices.push_back({ { right_top.x,    right_top.y,    0 }, { color.x, color.y, color.z, 0.5f } }); // 右上
    rect_vertices.push_back({ { left_bottom.x,  left_bottom.y,  0 }, { color.x, color.y, color.z, 0.5f } }); // 左下
    rect_vertices.push_back({ { left_bottom.x,  left_bottom.y,  0 }, { color.x, color.y, color.z, 0.5f } }); // 左下
    rect_vertices.push_back({ { right_top.x,    right_top.y,    0 }, { color.x, color.y, color.z, 0.5f } }); // 右上
    rect_vertices.push_back({ { right_bottom.x, right_bottom.y, 0 }, { color.x, color.y, color.z, 0.5f } }); // 右下
#endif // _DEBUG
}

void Debug2D::create_rect_vertex(ID3D11Device* device)
{
    std::unique_ptr<RectVertex[]> vertices{ std::make_unique<RectVertex[]>(rect_max_vertices) };
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = UINT(sizeof(RectVertex) * rect_max_vertices);
    desc.Usage = D3D11_USAGE_DYNAMIC; // D3D11_USAGE_DYNAMIC
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = vertices.get();
    subresourceData.SysMemPitch = 0;
    subresourceData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&desc, &subresourceData, rect_vertex_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}