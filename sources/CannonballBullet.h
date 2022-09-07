#pragma once
#include"BaseBullet.h"
class CannonballBullet final :public BaseBullet
{
public:
    CannonballBullet(GraphicsPipeline& Graphics_,
        DirectX::XMFLOAT3 FirstVec_, float Power_,DirectX::XMFLOAT3 Position_);
    ~CannonballBullet() override;
    void fUpdate(float elapsedTime_) override;

private:
    const float mkGravity{ 0.98f * 20.0f };
    std::unique_ptr<Effect> mpBombEffect{ nullptr };
};