#pragma once
#include"BaseBullet.h"
#include"Operators.h"
class DragonBreath final : public BaseBullet
{
public:
    DragonBreath(GraphicsPipeline& graphics ,const DirectX::XMFLOAT3& effect_position,
        float Power,const DirectX::XMFLOAT3& Target_ );

    ~DragonBreath() override;
    void fUpdate(float elapsedTime_) override;

private:
    void fTurnToTarget(float elapsedTime_, float RotSpeed_, const DirectX::XMFLOAT3& Target_);

private:

    DirectX::XMFLOAT3 mTargetPosition{};
    std::unique_ptr<Effect> mpBombEffect{ nullptr };
    const char* BombEffectPath{"./resources/Effect/dragon_breath_expro.efk"};
    float mPower{};
};
