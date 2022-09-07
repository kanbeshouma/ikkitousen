#pragma once

#include "UI.h"

class Number : public UI
{
public:
    //--------<constructor/destructor>--------//
    Number(ID3D11Device* device);
    ~Number() override {}
    //--------< ä÷êî >--------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    void render(ID3D11DeviceContext* dc) override;
    void render(ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos) override;
    //--------<getter/setter>--------//
    void set_offset_pos(const DirectX::XMFLOAT2& off_pos) { offset_pos = off_pos; }
    void set_offset_scale(const DirectX::XMFLOAT2& off_scale) { offset_scale = off_scale; }
    void set_color(const DirectX::XMFLOAT4& color) { element.color = color; }
    void set_value(int v) { value = v; }
private:
    //--------< ïœêî >--------//
    std::unique_ptr<SpriteBatch> number{ nullptr };
    Element element;
    DirectX::XMFLOAT2 offset_pos{};
    DirectX::XMFLOAT2 offset_scale{ 1, 1 };

    struct Details
    {
        int value{}; // êî
        std::vector<int> places; // äeà ÇÃêî
    };

    int value{};
    Details display{};
    int old_value{ -1 };
};