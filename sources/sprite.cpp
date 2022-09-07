#include <sstream>
#include <WICTextureLoader.h>
#include "misc.h"
#include "texture.h"
#include "shader.h"
#include "sprite.h"

void Sprite::render(ID3D11DeviceContext* dc,
    float position_x, float position_y, /* 矩形の左上の座標（スクリーン座標系）*/float size_w, float size_h/* 矩形のサイズ（スクリーン座標系）*/,
    float color_r, float color_g, float color_b, float color_a, float angle/*degree*/)
{
    // スクリーン（ビューポート）のサイズを取得する
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    dc->RSGetViewports(&num_viewports, &viewport);

    // renderメンバ関数の引数（dx, dy, dw, dh）から矩形の各頂点の位置（スクリーン座標系）を計算する
    //  (x0, y0) *----* (x1, y1)
    //           |   /|
    //           |  / |
    //           | /  |
    //           |/   |
    //  (x2, y2) *----* (x3, y3)

    // left-top
    float x0{ position_x };
    float y0{ position_y };
    // right-top
    float x1{ position_x + size_w };
    float y1{ position_y };
    // left-bottom
    float x2{ position_x };
    float y2{ position_y + size_h };
    // right-bottom
    float x3{ position_x + size_w };
    float y3{ position_y + size_h };


    // 点(x, y)が点(cx, cy)を中心に角(angle)で回転した時の座標を計算する関数オブジェクト（ラムダ式）
    auto rotate = [](float& x, float& y, float cx, float cy, float angle)
    {
        x -= cx;
        y -= cy;

        float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
        float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
        float tx{ x }, ty{ y };
        x = cos * tx + -sin * ty;
        y = sin * tx + cos * ty;

        x += cx;
        y += cy;
    };
    //回転の中心を矩形の中心点にした場合
    float cx = position_x + size_w * 0.5f;
    float cy = position_y + size_h * 0.5f;
    rotate(x0, y0, cx, cy, angle);
    rotate(x1, y1, cx, cy, angle);
    rotate(x2, y2, cx, cy, angle);
    rotate(x3, y3, cx, cy, angle);


    // スクリーン座標系からNDCへの座標変換をおこなう
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    // テクスチャ座標を頂点バッファにセットする   ※今回はテクスチャ全体を表示する
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = dc->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    vertex* vertices{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
    if (vertices != nullptr)
    {
        vertices[0].position = { x0, y0 , 0 };
        vertices[1].position = { x1, y1 , 0 };
        vertices[2].position = { x2, y2 , 0 };
        vertices[3].position = { x3, y3 , 0 };
        vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { color_r, color_g, color_b, color_a };

        vertices[0].texcoord = { 0, 0 }; // 左上
        vertices[1].texcoord = { 1, 0 }; // 右上
        vertices[2].texcoord = { 0, 1 }; // 左下
        vertices[3].texcoord = { 1, 1 }; // 右下
    }
    dc->Unmap(vertex_buffer.Get(), 0);

    bind_renderer(dc);
}

void Sprite::render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size,
    DirectX::XMFLOAT4 color, float angle)
{
    render(dc, position.x, position.y, size.x, size.y, color.x, color.y, color.z, color.w, angle);
}

// 追加する引数変数sx, sy, sw, shはテクセル座標系における矩形の左上座標(sx, sy)とサイズ(sw, sh)とする
// テクセル座標系からテクスチャ座標系への変換をおこなう
void Sprite::render(ID3D11DeviceContext* dc, float position_x, float position_y, float size_w, float size_h,
    float color_r, float color_g, float color_b, float color_a, float angle,
    float texpos_x, float texpos_y, float texsize_w, float texsize_h)
{
    // スクリーン（ビューポート）のサイズを取得する
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    dc->RSGetViewports(&num_viewports, &viewport);

    // renderメンバ関数の引数（dx, dy, dw, dh）から矩形の各頂点の位置（スクリーン座標系）を計算する
    //  (x0, y0) *----* (x1, y1)
    //           |   /|
    //           |  / |
    //           | /  |
    //           |/   |
    //  (x2, y2) *----* (x3, y3)

    // left-top
    float x0{ position_x };
    float y0{ position_y };
    // right-top
    float x1{ position_x + size_w };
    float y1{ position_y };
    // left-bottom
    float x2{ position_x };
    float y2{ position_y + size_h };
    // right-bottom
    float x3{ position_x + size_w };
    float y3{ position_y + size_h };


    // left-top
    float x0_s{ texpos_x };
    float y0_s{ texpos_y };
    // right-top
    float x1_s{ texpos_x + texsize_w };
    float y1_s{ texpos_y };
    // left-bottom
    float x2_s{ texpos_x };
    float y2_s{ texpos_y + texsize_h };
    // right-bottom
    float x3_s{ texpos_x + texsize_w };
    float y3_s{ texpos_y + texsize_h };


    // 点(x, y)が点(cx, cy)を中心に角(angle)で回転した時の座標を計算する関数オブジェクト（ラムダ式）
    auto rotate = [](float& x, float& y, float cx, float cy, float angle)
    {
        x -= cx;
        y -= cy;

        float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
        float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
        float tx{ x }, ty{ y };
        x = cos * tx + -sin * ty;
        y = sin * tx + cos * ty;

        x += cx;
        y += cy;
    };
    //回転の中心を矩形の中心点にした場合
    float cx = position_x + size_w * 0.5f;
    float cy = position_y + size_h * 0.5f;
    rotate(x0, y0, cx, cy, angle);
    rotate(x1, y1, cx, cy, angle);
    rotate(x2, y2, cx, cy, angle);
    rotate(x3, y3, cx, cy, angle);

    // スクリーン座標系からNDCへの座標変換をおこなう
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;
    // テクセル座標系からテクスチャ座標系へ変換をおこなう
    x0_s = x0_s / texture2d_desc.Width - 1.0f;
    y0_s = y0_s / texture2d_desc.Height;
    x1_s = x1_s / texture2d_desc.Width - 1.0f;
    y1_s = y1_s / texture2d_desc.Height;
    x2_s = x2_s / texture2d_desc.Width - 1.0f;
    y2_s = y2_s / texture2d_desc.Height;
    x3_s = x3_s / texture2d_desc.Width - 1.0f;
    y3_s = y3_s / texture2d_desc.Height;

    // テクスチャ座標を頂点バッファにセットする
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = dc->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    vertex* vertices{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
    if (vertices != nullptr)
    {
        vertices[0].position = { x0, y0 , 0 };
        vertices[1].position = { x1, y1 , 0 };
        vertices[2].position = { x2, y2 , 0 };
        vertices[3].position = { x3, y3 , 0 };
        vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { color_r, color_g, color_b, color_a };

        vertices[0].texcoord = { x0_s, y0_s }; // 左上
        vertices[1].texcoord = { x1_s, y1_s }; // 右上
        vertices[2].texcoord = { x2_s, y2_s }; // 左下
        vertices[3].texcoord = { x3_s, y3_s }; // 右下
    }
    dc->Unmap(vertex_buffer.Get(), 0);

    bind_renderer(dc);
}

void Sprite::render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size,
    DirectX::XMFLOAT4 color, float angle, DirectX::XMFLOAT2 texpos, DirectX::XMFLOAT2 texsize)
{
    render(dc, position.x, position.y, size.x, size.y, color.x, color.y, color.z, color.w,
        angle, texpos.x, texpos.y, texsize.x, texsize.y);
}

void Sprite::render(ID3D11DeviceContext* dc, float position_x, float position_y, float size_w, float size_h)
{
    // スクリーン（ビューポート）のサイズを取得する
    D3D11_VIEWPORT viewport{};
    UINT num_viewports{ 1 };
    dc->RSGetViewports(&num_viewports, &viewport);

    // left-top
    float x0{ position_x };
    float y0{ position_y };
    // right-top
    float x1{ position_x + size_w };
    float y1{ position_y };
    // left-bottom
    float x2{ position_x };
    float y2{ position_y + size_h };
    // right-bottom
    float x3{ position_x + size_w };
    float y3{ position_y + size_h };

    // スクリーン座標系からNDCへの座標変換をおこなう
    x0 = 2.0f * x0 / viewport.Width - 1.0f;
    y0 = 1.0f - 2.0f * y0 / viewport.Height;
    x1 = 2.0f * x1 / viewport.Width - 1.0f;
    y1 = 1.0f - 2.0f * y1 / viewport.Height;
    x2 = 2.0f * x2 / viewport.Width - 1.0f;
    y2 = 1.0f - 2.0f * y2 / viewport.Height;
    x3 = 2.0f * x3 / viewport.Width - 1.0f;
    y3 = 1.0f - 2.0f * y3 / viewport.Height;

    // テクセル座標系からテクスチャ座標系へ変換をおこなう
    // left-top
    float x0_s{ 0 };
    float y0_s{ 0 };
    // right-top
    float x1_s{ 1 };
    float y1_s{ 0 };
    // left-bottom
    float x2_s{ 0 };
    float y2_s{ 1 };
    // right-bottom
    float x3_s{ 1 };
    float y3_s{ 1 };

    // テクスチャ座標を頂点バッファにセットする
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = dc->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    vertex* vertices{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
    if (vertices != nullptr)
    {
        vertices[0].position = { x0, y0 , 0 };
        vertices[1].position = { x1, y1 , 0 };
        vertices[2].position = { x2, y2 , 0 };
        vertices[3].position = { x3, y3 , 0 };
        vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { 1.0f, 1.0f, 1.0f, 1.0f };

        vertices[0].texcoord = { x0_s, y0_s }; // 左上
        vertices[1].texcoord = { x1_s, y1_s }; // 右上
        vertices[2].texcoord = { x2_s, y2_s }; // 左下
        vertices[3].texcoord = { x3_s, y3_s }; // 右下
    }
    dc->Unmap(vertex_buffer.Get(), 0);

    bind_renderer(dc);
}

void Sprite::render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size)
{
    render(dc, position.x, position.y, size.x, size.y);
}

void Sprite::textout(ID3D11DeviceContext* dc, std::string s,
    float position_x, float position_y, float size_w, float size_h, float color_r, float color_g, float color_b, float color_a)
{
    float sw = static_cast<float>(texture2d_desc.Width / 16);
    float sh = static_cast<float>(texture2d_desc.Height / 16);
    float carriage = 0;
    for (const char c : s)
    {
        render(dc, position_x + carriage, position_y, size_w, size_h, color_r, color_g, color_b, color_a, 0,
            sw * (c & 0x0F), sh * (c >> 4), sw, sh);
        carriage += size_w;
    }
}

void Sprite::textout(ID3D11DeviceContext* dc, std::string s,
    DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color)
{
    textout(dc, s, position.x, position.y, size.x, size.y, color.x, color.y, color.z, color.w);
}

void Sprite::bind_renderer(ID3D11DeviceContext* dc)
{
    // シェーダー リソースのバインド
    dc->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());

    // 頂点バッファーのバインド
    UINT stride{ sizeof(vertex) };
    UINT offset{ 0 };
    dc->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);

    // プリミティブタイプおよびデータの順序に関する情報のバインド
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // 入力レイアウトオブジェクトのバインド
    dc->IASetInputLayout(input_layout.Get());

    // シェーダーのバインド
    dc->VSSetShader(vertex_shader.Get(), nullptr, 0);
    dc->PSSetShader(pixel_shader.Get(), nullptr, 0);

    // プリミティブの描画
    dc->Draw(4, 0);
}

Sprite::Sprite(ID3D11Device* device, const wchar_t* filename)
{
    // 頂点情報のセット
#if 1
    vertex vertices[]
    {
        /* 中心を {0, 0} として画面端を-1～１で座礁を決定する */
      { { -1.0f, +1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0, 0 } },   // 左上
      { { +1.0f, +1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1, 0 } },   // 右上
      { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0, 1 } },   // 左下
      { { +1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1, 1 } },   // 右下
    };
#endif // 1

    HRESULT hr{ S_OK };

    // 頂点バッファオブジェクトの生成
    // GPU (読み取りのみ) と CPU (書き込みのみ) によるアクセスを可能にする
    {
        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(vertices);
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        buffer_desc.MiscFlags = 0;
        buffer_desc.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA subresource_data{};
        subresource_data.pSysMem = vertices;
        subresource_data.SysMemPitch = 0;
        subresource_data.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    // 入力レイアウトオブジェクトの生成
    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,  0 },
    };

    // 頂点シェーダーオブジェクトの生成
    const char* cso_name{ "shaders/sprite_vs.cso" };
    create_vs_from_cso(device, cso_name, vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
    // ピクセルシェーダーオブジェクトの生成
    const char* cso_name_ps{ "shaders/sprite_ps.cso" };
    create_ps_from_cso(device, cso_name_ps, pixel_shader.GetAddressOf());
    // 画像ファイルのロードとシェーダーリソースビューオブジェクト(ID3D11ShaderResourceView)の生成
    load_texture_from_file(device, filename, shader_resource_view.GetAddressOf(), &texture2d_desc);
}

Sprite::~Sprite()
{
    release_all_textures();
}