#include "effect_manager.h"

void EffectManager::initialize(GraphicsPipeline& graphics)
{
    std::lock_guard<std::mutex> lock(graphics.get_mutex());
    //Effekseer�����_������
    effekseer_renderer = EffekseerRendererDX11::Renderer::Create(graphics.get_device().Get(), graphics.get_dc().Get(), 2048);
    //Effekseer�}�l�[�W���[����
    effekseer_manager = Effekseer::Manager::Create(2048);
    //Effekseer�����_���̊e��ݒ�(���ʂȃJ�X�^�}�C�Y�����Ȃ��ꍇ�͒���I�Ɉȉ��̐ݒ�)
    effekseer_manager->SetSpriteRenderer(effekseer_renderer->CreateSpriteRenderer());
    effekseer_manager->SetRibbonRenderer(effekseer_renderer->CreateRibbonRenderer());
    effekseer_manager->SetRingRenderer(effekseer_renderer->CreateRingRenderer());
    effekseer_manager->SetTrackRenderer(effekseer_renderer->CreateTrackRenderer());
    effekseer_manager->SetModelRenderer(effekseer_renderer->CreateModelRenderer());
    //Effekseern���ł̃��[�_�[�̐ݒ�
    effekseer_manager->SetTextureLoader(effekseer_renderer->CreateTextureLoader());
    effekseer_manager->SetModelLoader(effekseer_renderer->CreateModelLoader());
    effekseer_manager->SetMaterialLoader(effekseer_renderer->CreateMaterialLoader());
    //Effekseer��������W�n�Ōv�Z����
    effekseer_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffectManager::finalize()
{
    if (effekseer_manager != nullptr)
    {
        effekseer_manager->Destroy();
        effekseer_manager = nullptr;
    }
    if (effekseer_renderer != nullptr)
    {
        effekseer_renderer->Destroy();
        effekseer_renderer = nullptr;
    }
}

void EffectManager::update(float elapsed_time)
{
    //�G�t�F�N�g�X�V����
    effekseer_manager->Update(elapsed_time * 60.0f);
}

void EffectManager::render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
    //�r���[&�v���W�F�N�V�����s���Effekseer�����_���ɐݒ�
    effekseer_renderer->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
    effekseer_renderer->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));
    //Effekseer�`��J�n
    effekseer_renderer->BeginRendering();
    //Effekseer�`��J�n
    //�}�l�[�W���[�P�ʂŕ`�悷��̂ŕ`�揇�𐧌䂷��ꍇ�̓}�l�[�W���[�𕡐��쐬���C
    //Draw()�֐������s���鏇���Ő���ł�����
    effekseer_manager->Draw();
    //Effekseer�`��I��
    effekseer_renderer->EndRendering();
}