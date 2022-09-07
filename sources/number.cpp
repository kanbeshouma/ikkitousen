#include "number.h"
#include "Operators.h"

Number::Number(ID3D11Device* device)
{
    number = std::make_unique<SpriteBatch>(device, L".\\resources\\Sprites\\ui\\number.png", 11);

    element.scale   = { 1.0f, 1.0f };
    element.texsize = { static_cast<float>(number->get_texture2d_desc().Width) / 10.0f, static_cast<float>(number->get_texture2d_desc().Height) };
    element.pivot   = element.texsize * DirectX::XMFLOAT2(0.5f, 0.5f);
}

void Number::update(GraphicsPipeline& graphics, float elapsed_time)
{
    display.value = value;

    if (old_value != display.value)
    {
        if (!display.places.empty()) { display.places.clear(); }
        int number_of_splits = display.value;
        for (int i = 0; i < 10; ++i) // ‚±‚ÌƒNƒ‰ƒX‚Å•\‚¹‚éÅ‘å”‚Í10‚Ì10æ(100‰­)
        {
            display.places.emplace_back<int>(number_of_splits % 10);
            number_of_splits = number_of_splits / 10;
            if (number_of_splits == 0) break;
        }
        if (display.value < 0) { display.places.clear(); }
    }

    old_value = display.value;
}

void Number::render(ID3D11DeviceContext* dc)
{
    //--number--//
    number->begin(dc);
    int step = 0;
    for (auto& item : display.places)
    {
        ++step;
        number->render(dc, { offset_pos.x + (display.places.size() - step) * element.texsize.x * offset_scale.x, offset_pos.y },
            offset_scale * element.scale, element.pivot, element.color, element.angle,
            { element.texpos.x + element.texsize.x * item, element.texpos.y }, element.texsize);
    }
    number->end(dc);
}

void Number::render(ID3D11DeviceContext* dc, const DirectX::XMFLOAT2& add_pos)
{
    //--number--//
    number->begin(dc);
    int step = 0;
    for (auto& item : display.places)
    {
        ++step;
        number->render(dc, { (offset_pos.x + (display.places.size() - step) * element.texsize.x * offset_scale.x) + add_pos.x, offset_pos.y + add_pos.y },
            offset_scale * element.scale, element.pivot, element.color, element.angle,
            { element.texpos.x + element.texsize.x * item, element.texpos.y }, element.texsize);
    }
    number->end(dc);
}