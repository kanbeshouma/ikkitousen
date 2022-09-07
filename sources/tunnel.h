#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <functional>

#include "constants.h"
#include "fullscreen_quad.h"

class Tunnel
{
public:
    //--------<constructor/destructor>--------//
    Tunnel(ID3D11Device* device);
    ~Tunnel() = default;
    //--------< ŠÖ” >--------//
    void render(ID3D11DeviceContext* dc, float elapsed_time, float alpha, std::function<void()>draw_func);
private:
    //--------< •Ï” >--------//
    struct TunnelConstants
    {
        DirectX::XMFLOAT4 threshold{ 0.5f,0.5f,0,0 }; // xy:tunnel pos  z:timer  w:alpha
    };
    std::unique_ptr<Constants<TunnelConstants>> constants;
    std::unique_ptr<FullScreenQuad> tunnel;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tunnel_map;
};