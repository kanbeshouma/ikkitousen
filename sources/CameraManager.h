#pragma once
#include "camera.h"
#include "BaseCamera.h"

//class CameraManager
//{
//public:
//    CameraManager(GraphicsPipeline& graphics, Player* player, std::vector<int> cameraType);
//    ~CameraManager(){};
//
//    void Initialize(GraphicsPipeline& graphics, Player* player);
//    void Update(float elapsedTime, Player* player);
//
//    void CalcViewProjection(GraphicsPipeline& graphics);
//
//    void ChangeCamera(int i);
//    void SetCamera();
//
//    camera* GetCamera(int i) { return cameras[i].get(); }
//
//    camera* GetCamera() { return currentCamera; }
//private:
//
//    std::unique_ptr<camera> cameras[2]{};
//
//    std::vector<int>activeCamera{};
//
//    camera* currentCamera{};
//public:
//    enum CameraTypes
//    {
//        Title = 0,
//        Game,
//    };
//};
class CameraManager
{
public:
    CameraManager(){}
    ~CameraManager(){
        for (Camera* camera : cameras)
        {
            delete camera;
        }
        cameras.clear();
    }

    void Initialize(GraphicsPipeline& graphics);
    void Update(float elapsedTime);

    void CalcViewProjection(GraphicsPipeline& graphics);

    void ChangeCamera(GraphicsPipeline& graphics,int newCamera);
    void SetCamera(int cameraType);
    void RegisterCamera(Camera* camera);

    Camera* GetCurrentCamera() { return currentCamera; }
private:
    std::vector<Camera*> cameras{};
    Camera* currentCamera{};

};