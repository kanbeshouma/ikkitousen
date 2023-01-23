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
    //--------< �֐� >--------//
    //������
    void initialize(GraphicsPipeline& graphics) override;
    //�I����
    void uninitialize() override;
    // �G�t�F�N�g���
    void effect_liberation(GraphicsPipeline& graphics) override;
    //�X�V����
    void update(GraphicsPipeline& graphics, float elapsed_time) override;
    //�`�揈��
    void render(GraphicsPipeline& graphics, float elapsed_time) override;
    //�V���h�E�}�b�v�o�^
    void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) override;
private:
    std::unique_ptr<SkyDome> sky_dome{ nullptr };
    std::unique_ptr<CameraManager> cameraManager;



};