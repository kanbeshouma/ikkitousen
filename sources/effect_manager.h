#pragma once

#include "graphics_pipeline.h"
#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>

class EffectManager
{
public:
    EffectManager() {}
    ~EffectManager() {}
public:
    //‰Šú‰»ˆ—
    void initialize(GraphicsPipeline& graphics);
    //I—¹ˆ—
    void finalize();
    //XVˆ—
    void update(float elapsed_time);
    //•`‰æˆ—
    void render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);
    //--------<getter/setter>--------//
    Effekseer::Manager* get_effekseer_manager() { return effekseer_manager; }
private:
    //--------< •Ï” >--------//
    Effekseer::Manager* effekseer_manager{ nullptr };
    EffekseerRenderer::Renderer* effekseer_renderer{ nullptr };
};