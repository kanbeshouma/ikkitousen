#pragma once

#include "UI.h"
#include "number.h"

class Counter : public UI
{
public:
    //--------<constructor/destructor>--------//
    Counter(GraphicsPipeline& graphics, const wchar_t* filename, size_t max_sprites = 1);
    ~Counter() override {}
    //--------< ŠÖ” >--------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    void render(ID3D11DeviceContext* dc) override;
    //--------<getter/setter>--------//
    void set_positoin(const DirectX::XMFLOAT2& pos) { element.position = pos; }
    void set_scale(const DirectX::XMFLOAT2& scale) { element.scale = scale; }
    void set_color(const DirectX::XMFLOAT4& color) { number->set_color(color); element.color = color; }
    void set_offset(const DirectX::XMFLOAT2& o) { offset = o; }
    void set_value(int v) { number->set_value(v); }
private:
    //--------< •Ï” >--------//
    std::unique_ptr<SpriteBatch> sprite{ nullptr };
    Element element;
    DirectX::XMFLOAT2 offset{};
    std::unique_ptr<Number> number{ nullptr };
};