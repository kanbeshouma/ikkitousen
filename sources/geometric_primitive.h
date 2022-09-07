#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>

class GeometricPrimitive
{
public:
    struct vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;    // ñ@ê¸
    };
    struct constants
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 material_color;
    };

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

public:
    GeometricPrimitive(ID3D11Device* device);
    GeometricPrimitive(ID3D11Device* device,
        const DirectX::XMFLOAT3& min_position, const DirectX::XMFLOAT3& max_position);
    virtual ~GeometricPrimitive() = default;
    void render(ID3D11DeviceContext* dc,
        const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color);
protected:
    void create_com_buffers(ID3D11Device* device, vertex* vertices, size_t vertex_count,
        uint32_t* indices, size_t index_count);
};

