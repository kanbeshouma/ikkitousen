#pragma once

#include <DirectXMath.h>
#include "sprite_batch.h"
#include "graphics_pipeline.h"

class UI
{
public:
    //--------<constructor/destructor>--------//
    UI() {}
    virtual ~UI() {}
    //--------< ŠÖ” >--------//
    virtual void update(GraphicsPipeline& graphics, float elapsed_time) = 0;
    virtual void render(ID3D11DeviceContext* dc) = 0;
    virtual void render(ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos) {}
protected:
    //--------< \‘¢‘Ì >--------//
    struct Element
    {
        DirectX::XMFLOAT2 position{};
        DirectX::XMFLOAT2 scale{ 1, 1 };
        DirectX::XMFLOAT2 pivot{};
        DirectX::XMFLOAT4 color{ 1,1,1,1 };
        float angle{};
        DirectX::XMFLOAT2 texpos{};
        DirectX::XMFLOAT2 texsize{};
    };
};