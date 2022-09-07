#include "tunnel.h"

#include "shader.h"
#include "texture.h"

Tunnel::Tunnel(ID3D11Device* device)
{
    // ps
    const char* cso_name_ps{ "shaders/tunnel_ps.cso" };
    create_ps_from_cso(device, cso_name_ps, pixel_shader.GetAddressOf());
    // sprite
    tunnel = std::make_unique<FullScreenQuad>(device);
    // constants
    constants = std::make_unique<Constants<TunnelConstants>>(device);
    // srv
    D3D11_TEXTURE2D_DESC texture2d_desc{};
    load_texture_from_file(device, L".\\resources\\TexMaps\\tunnel\\tunnel.png", tunnel_map.GetAddressOf(), &texture2d_desc);
}

void Tunnel::render(ID3D11DeviceContext* dc, float elapsed_time, float alpha, std::function<void()>draw_func)
{
    constants->data.threshold.z += elapsed_time;
    constants->data.threshold.w = alpha;
    constants->bind(dc, 2);
    tunnel->blit(dc, tunnel_map.GetAddressOf(), 0, 1, pixel_shader.Get());

    draw_func();
}
