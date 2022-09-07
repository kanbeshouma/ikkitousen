#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <DirectXMath.h>

enum class FB_FLAG : uint8_t
{
    COLOR = 0x01,
    DEPTH = 0x02,
    COLOR_DEPTH = COLOR | DEPTH,
};

class FrameBuffer
{
public:
    //--------<constructor/destructor>--------//
    FrameBuffer(ID3D11Device* device, uint32_t width, uint32_t height, FB_FLAG flags = FB_FLAG::COLOR_DEPTH);
    virtual ~FrameBuffer() = default;
    //--------< ŠÖ” >--------//
    void clear(ID3D11DeviceContext* dc, FB_FLAG flags = FB_FLAG::COLOR_DEPTH, DirectX::XMFLOAT4 color = { 0, 0, 0, 1 }, float depth = 1);

    void activate(ID3D11DeviceContext* dc, FB_FLAG flags = FB_FLAG::COLOR_DEPTH);
    void deactivate(ID3D11DeviceContext* dc);
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& get_color_map() { return shader_resource_views[0]; }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& get_depth_map() { return shader_resource_views[1]; }
private:
    //--------< •Ï” >--------//
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[2];
    D3D11_VIEWPORT viewport;
    UINT viewport_count{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
    D3D11_VIEWPORT cached_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;
};