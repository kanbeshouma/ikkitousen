#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "SceneParamCheck.h"

void SceneParamCheck::initialize(GraphicsPipeline& graphics)
{
    // モデルのロード
    sky_dome = std::make_unique<SkyDome>(graphics);
    sky_dome->set_purple_threshold(0);
    sky_dome->set_red_threshold(0);

    // カメラ
    cameraManager = std::make_unique<CameraManager>();
    //cameraManager->RegisterCamera();
    //cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));


}

void SceneParamCheck::uninitialize()
{
}

void SceneParamCheck::effect_liberation(GraphicsPipeline& graphics)
{
}

void SceneParamCheck::update(GraphicsPipeline& graphics, float elapsed_time)
{
    // camera
    cameraManager->Update(elapsed_time);

}

void SceneParamCheck::render(GraphicsPipeline& graphics, float elapsed_time)
{
    cameraManager->CalcViewProjection(graphics);

    sky_dome->Render(graphics, elapsed_time);


}

void SceneParamCheck::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
}
