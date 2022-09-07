#pragma once

#include <memory>
#include "graphics_pipeline.h"

class Scene
{
public:
    Scene() {}
    virtual ~Scene() {}
    //������
    virtual void initialize(GraphicsPipeline& graphics) = 0;
    //�I����
    virtual void uninitialize() = 0;
    // �G�t�F�N�g���
    virtual void effect_liberation(GraphicsPipeline& graphics) {}
    //�X�V����
    virtual void update(GraphicsPipeline& graphics, float elapsed_time) = 0;
    //�`�揈��
    virtual void render(GraphicsPipeline& graphics, float elapsed_time) = 0;
    //�V���h�E�}�b�v�o�^
    virtual void register_shadowmap(GraphicsPipeline& graphics, float elapsed_time) {}
    //�����������Ă��邩
    bool is_ready()const { return ready; }
    //���������ݒ�
    void set_ready(bool r) { ready = r; }
private:
    bool ready{};
};