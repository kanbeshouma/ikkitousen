#pragma once

#include <memory>
#include "graphics_pipeline.h"

class Scene
{
public:
    Scene() {}
    virtual ~Scene() {}
    //初期化
    virtual void initialize(GraphicsPipeline& graphics) = 0;
    //終了化
    virtual void uninitialize() = 0;
    // エフェクト解放
    virtual void effect_liberation(GraphicsPipeline& graphics) {}
    //更新処理
    virtual void update(GraphicsPipeline& graphics, float elapsed_time) = 0;
    //描画処理
    virtual void render(GraphicsPipeline& graphics, float elapsed_time) = 0;
    //シャドウマップ登録
    virtual void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) {}
    //準備完了しているか
    bool is_ready()const { return ready; }
    //準備完了設定
    void set_ready(bool r) { ready = r; }
private:
    bool ready{};
};