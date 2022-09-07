#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include "constants.h"

class Debug2D
{
public:
    //--------<constructor/destructor>--------//
    Debug2D(ID3D11Device* device);
    ~Debug2D() {}
    //--------< ä÷êî >--------//
    // ï`âÊé¿çs
    void all_render(ID3D11DeviceContext* context);
    void create_rect(const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& radius, const DirectX::XMFLOAT3& color);
private:
    void create_rect_vertex(ID3D11Device* device);
    //--------< ç\ë¢ëÃ >--------//
    //--rect--//
    struct RectVertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };
    //--------< ïœêî >--------//
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

    float viewport_width = 0;
    float viewport_height = 0;

    //--rect--//
    const size_t rect_max_vertices = 64 * 6;
    std::vector<RectVertex> rect_vertices;
    Microsoft::WRL::ComPtr<ID3D11Buffer> rect_vertex_buffer;
};