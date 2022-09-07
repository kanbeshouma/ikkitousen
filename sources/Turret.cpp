//****************************************************************
// 
// タレットの実装
// 
//****************************************************************


#include"LastBoss.h"

LastBoss::Turret::Turret(GraphicsPipeline& Graphics_)
{
   // モデルの初期化
    mpModel =  resource_manager->load_model_resource(Graphics_.get_device().Get(),
        "./resources/Models/Enemy/boss_turret.fbx");

}

void LastBoss::Turret::fUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mpModel->update_animation(elapsedTime_);
}

void LastBoss::Turret::fRender(GraphicsPipeline& graphics_, 
    const DirectX::XMFLOAT4X4& ParentWorld_,
    const DirectX::XMFLOAT3& Position_)
{

    mPosition =
    {
        ParentWorld_._41,
        ParentWorld_._42,
        ParentWorld_._43

    };
    mpModel->render(graphics_.get_dc().Get(), Turret::mAnimPara,
        ParentWorld_, { 1.0f,1.0f,1.0f,1.0f }, mDissolve);
}

void LastBoss::Turret::fSetDissolve(float Dissolve_)
{
    mDissolve = Dissolve_;
}

DirectX::XMFLOAT3 LastBoss::Turret::fGetPosition() const
{
    return mPosition;
}

//****************************************************************
// 
// 第二砲身
// 
//****************************************************************
LastBoss::SecondGun::SecondGun(GraphicsPipeline& Graphics_)
{
    // モデルの初期化
    mpModel = resource_manager->load_model_resource(Graphics_.get_device().Get(),
        "./resources/Models/Enemy/boss_secondarygun.fbx");
}

void LastBoss::SecondGun::fUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mpModel->update_animation(elapsedTime_);
}

void LastBoss::SecondGun::fRender(GraphicsPipeline& graphics_, const DirectX::XMFLOAT4X4& ParentWorld_,const DirectX::XMFLOAT3& Position_)
{

    mpModel->render(graphics_.get_dc().Get(), SecondGun::mAnimPara,
        ParentWorld_, { 1.0f,1.0f,1.0f,1.0f },mDissolve);
}

void LastBoss::SecondGun::fSetDissolve(float Dissolve_)
{
    mDissolve = Dissolve_;
}
