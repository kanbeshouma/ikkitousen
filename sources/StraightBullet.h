#pragma once
#include"BaseBullet.h"
//****************************************************************
// 
// ���i���邾���̒e 
// 
//****************************************************************

class StraightBullet final :public BaseBullet
{
public:
    StraightBullet(GraphicsPipeline& Graphics_,DirectX::XMFLOAT3 InitPoint_,DirectX::XMFLOAT3 mVelocity_);
    ~StraightBullet()override = default;
    void fUpdate(float elapsedTime_) override;
private:
    float mLifeTime{}; // ��������

};


