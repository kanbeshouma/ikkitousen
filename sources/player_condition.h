#pragma once

#include "UI.h"

class PlayerCondition : public UserInterface
{
public:
    //--------<constructor/destructor>--------//
    PlayerCondition(GraphicsPipeline& graphics);
    ~PlayerCondition() override {}
    //--------< �֐� >--------//
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    void render(ID3D11DeviceContext* dc) override;
    //--------<getter/setter>--------//
    void set_is_damage(bool arg) { is_damage = arg; }
private:
    //--------< �ϐ� >--------//
    std::unique_ptr<SpriteBatch> normal_condition{ nullptr };
    std::unique_ptr<SpriteBatch> damage_condition{ nullptr };

    Element condition;

    bool is_damage = false;
    float reset_timer = 0.0f;
};