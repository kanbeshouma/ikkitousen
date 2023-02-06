#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "SceneParamCheck.h"
#include"BulletManager.h"
#include"imgui_include.h"
#include"Correspondence.h"

void SceneParamCheck::initialize(GraphicsPipeline& graphics)
{
    // モデルのロード
    sky_dome = std::make_unique<SkyDome>(graphics);
    sky_dome->set_purple_threshold(0);
    sky_dome->set_red_threshold(0);

    // カメラ
    cameraManager = std::make_unique<CameraManager>();
    cameraManager->RegisterCamera(new DebugCamera(graphics));
    //<デバッグシーンではカメラはデバッグカメラしかないから0番目を直で設定>//
    cameraManager->ChangeCamera(graphics, 0);
    //cameraManager->RegisterCamera();
    //cameraManager->ChangeCamera(graphics, static_cast<int>(CameraTypes::Game));

    enemy_manager = std::make_unique<EnemyManager>();
    BulletManager& mBulletManager = BulletManager::Instance();
    mBulletManager.fInitialize();

    enemy_manager->fInitialize(graphics, mBulletManager.fGetAddFunction());
}

void SceneParamCheck::uninitialize()
{
}

void SceneParamCheck::effect_liberation(GraphicsPipeline& graphics)
{
}

void SceneParamCheck::update(GraphicsPipeline& graphics, float elapsed_time)
{
#ifdef USE_IMGUI
    ImGui::Begin("LoadEnemyData");
    ImGui::InputTextAbove("IP Adress", SocketCommunicationManager::Instance().host_ip, sizeof(SocketCommunicationManager::Instance().host_ip), ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::Button("Copy HostIP -> HTTP IP"))
    {
        std::memcpy(SocketCommunicationManager::Instance().http_ip, SocketCommunicationManager::Instance().host_ip, sizeof(SocketCommunicationManager::Instance().http_ip));
    }
    ImGui::InputTextAbove("HTTP IP Adress", SocketCommunicationManager::Instance().http_ip, sizeof(SocketCommunicationManager::Instance().http_ip), ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::Button("Copy HTTP IP -> HostIP"))
    {
        std::memcpy(SocketCommunicationManager::Instance().host_ip, SocketCommunicationManager::Instance().http_ip, sizeof(SocketCommunicationManager::Instance().http_ip));
    }
    if (ImGui::Button("Http"))
    {
        if (CorrespondenceManager::Instance().HttpRequest())
        {
            enemy_manager->DebugLoadEnemyParam();
        }
    }
    ImGui::End();
#endif
    // camera
    cameraManager->Update(elapsed_time);

    BulletManager& mBulletManager = BulletManager::Instance();
    enemy_manager->fDebugUpdate(graphics, elapsed_time, mBulletManager.fGetAddFunction());

}

void SceneParamCheck::render(GraphicsPipeline& graphics, float elapsed_time)
{
    cameraManager->CalcViewProjection(graphics);

    sky_dome->Render(graphics, elapsed_time);
    enemy_manager->fRender(graphics);
}

void SceneParamCheck::register_shadowmap(GraphicsPipeline& graphics, float elapsed_time)
{
}
