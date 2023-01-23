#pragma once
#include "Scene.h"
#include "camera.h"
#include "CameraManager.h"
#include"EnemyManager.h"


class SceneParamCheck : public Scene, public PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    SceneParamCheck() {}
    ~SceneParamCheck() override {}
    //--------< 関数 >--------//
    //初期化
    void initialize(GraphicsPipeline& graphics) override;
    //終了化
    void uninitialize() override;
    // エフェクト解放
    void effect_liberation(GraphicsPipeline& graphics) override;
    //更新処理
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    //描画処理
    void render(GraphicsPipeline& graphics, float elapsed_time) override;
    //シャドウマップ登録
    void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) override;
private:
    std::unique_ptr<SkyDome> sky_dome{ nullptr };
    std::unique_ptr<CameraManager> cameraManager;



};