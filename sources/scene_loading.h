#pragma once

#include "Scene.h"
#include "graphics_pipeline.h"
#include "SpriteBatch.h"
#include "practical_entities.h"

class SpriteBatch;

class SceneLoading : public Scene, PracticalEntities
{
public:
    //--------<constructor/destructor>--------//
    SceneLoading(Scene* nextscene) : next_scene(nextscene) {}
    ~SceneLoading() override {}
    //--------< �֐� >--------//
    //������
    void initialize(GraphicsPipeline& graphics) override;
    //�I����
    void uninitialize() override;
    //�X�V����
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    //�`�揈��
    void render(GraphicsPipeline& graphics, float elapsed_time) override;
private:
    //--------< �֐� >--------//
    static void loading_thread(GraphicsPipeline* graphics, SceneLoading* scene);
private:
    //--------< �ϐ� >--------//
    Scene* next_scene{};
    std::unique_ptr<SpriteBatch> load_back;
    std::unique_ptr<SpriteBatch> load_text;
    std::unique_ptr<SpriteBatch> load_icon;

    float loadTextWidth{};
    DirectX::XMFLOAT2 loadIconPosition{};
};