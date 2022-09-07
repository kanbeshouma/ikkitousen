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
    //--------<render関数>--------//
    // 位置、サイズ、角度、描画色
    void render(ID3D11DeviceContext* dc,
        float position_x, float position_y, /* 矩形の左上の座標（スクリーン座標系）*/float size_w, float size_h/* 矩形のサイズ（スクリーン座標系）*/,
        float color_r, float color_g, float color_b, float color_a, float angle/*degree*/);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, float angle);
    // 位置、サイズ、角度、描画色、 切り取り位置、切り取りサイズ
    void render(ID3D11DeviceContext* dc, float position_x, float position_y, float size_w, float size_h,
        float color_r, float color_g, float color_b, float color_a, float angle,
        float texpos_x, float texpos_y, float texsize_w, float texsize_h);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size,
        DirectX::XMFLOAT4 color, float angle, DirectX::XMFLOAT2 texpos, DirectX::XMFLOAT2 texsize);
    // 位置、サイズ
    void render(ID3D11DeviceContext* dc, float position_x, float position_y, float size_w, float size_h);
    void render(ID3D11DeviceContext* dc, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size);
    // フォント画像を使用し任意の文字列を画面に出力する ※フォント画像はアスキーコード順に１６ｘ１６の文字が配置された画像ファイル
    void textout(ID3D11DeviceContext* dc, std::string s,
        float position_x, float position_y, float size_w, float size_h, float color_r, float color_g, float color_b, float color_a);
    void textout(ID3D11DeviceContext* dc, std::string s, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color);

private:
    // render関数のオーバーライドによって重複したコードの共通関数
    void bind_renderer(ID3D11DeviceContext* dc);

private:
    // 頂点フォーマット
    struct vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 texcoord;
    };
};
