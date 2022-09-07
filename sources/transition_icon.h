#pragma once

#include "icon_base.h"
#include "practical_entities.h"

class TransitionIcon : public IconBase, PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    TransitionIcon(ID3D11Device* device);
    ~TransitionIcon() override {}
    //--------< ŠÖ” >--------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    void render(std::string gui, ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos) override;
private:
    //--------< •Ï” >--------//
    FontElement game;
    FontElement title;
};