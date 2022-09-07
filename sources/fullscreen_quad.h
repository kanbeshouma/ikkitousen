#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class FullScreenQuad
{
public:
    //--------<constructor/destructor>--------//
    FullScreenQuad(ID3D11Device* device);
    virtual ~FullScreenQuad() = default;
private:
    //--------< •Ï” >--------//
    Microsoft::WRL::ComPtr<ID3D11VertexShader> embedded_vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> embedded_pixel_shader;
public:
    //--------< ŠÖ” >--------//
    void blit(ID3D11DeviceContext* dc, ID3D11ShaderResourceView** shader_resource_view,
        uint32_t start_slot, uint32_t num_views, ID3D11PixelShader* replaced_pixel_shader = nullptr);
};