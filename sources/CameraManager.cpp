#include "CameraManager.h"
//
//CameraManager::CameraManager(GraphicsPipeline& graphics, Player* player, std::vector<int> cameraType)
//{
//    for(auto it : cameraType)
//    {
//        cameras[it] = std::make_unique<camera>(it, graphics, player);
//    }
//    activeCamera = cameraType;
//}
//
//void CameraManager::Initialize(GraphicsPipeline& graphics, Player* player)
//{
//    for (auto& it : activeCamera)
//    {
//        cameras[it]->Initialize(graphics, player);
//    }
//}
//
//void CameraManager::Update(float elapsedTime, Player* player)
//{
//    for (auto& it : activeCamera)
//    {
//        cameras[it]->Update(elapsedTime, player);
//    }
//}
//
//void CameraManager::CalcViewProjection(GraphicsPipeline& graphics)
//{
//    for (auto& it : activeCamera)
//    {
//        cameras[it]->calc_view_projection(graphics);
//    }
//}


void CameraManager::Initialize(GraphicsPipeline& graphics)
{
    currentCamera->Initialize(graphics);
}

void CameraManager::Update(float elapsedTime)
{
    currentCamera->Update(elapsedTime);
}

void CameraManager::CalcViewProjection(GraphicsPipeline& graphics)
{
    currentCamera->calc_view_projection(graphics);
}

void CameraManager::ChangeCamera(GraphicsPipeline& graphics,int newCamera)
{
    SetCamera(newCamera);
    currentCamera->Initialize(graphics);
}

void CameraManager::SetCamera(int cameraType)
{
    currentCamera = cameras.at(cameraType);
}

void CameraManager::RegisterCamera(Camera* camera)
{
    cameras.emplace_back(camera);
}
