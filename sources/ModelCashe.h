#pragma once
#include"skinned_mesh.h"
#include<vector>
#include<memory>
#include "practical_entities.h"

class ModelCashes : public PracticalEntities
{
public:
    static void Load_IntoTitle(ID3D11Device* pDevice_); // タイトル画面の裏でロードする
    static void Load_PreTitle(ID3D11Device* pDevice_);  // タイトル画面の前にロードする
private:
    inline static std::vector<std::shared_ptr<SkinnedMesh>> mpModelCashVec{};
};

inline void ModelCashes::Load_IntoTitle(ID3D11Device* pDevice_)
{
    // ボスのモデルをロード
    {
        auto model = resource_manager->load_model_resource(pDevice_, "./resources/Models/Enemy/boss_animation_sixth.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
    {
        auto model = resource_manager->load_model_resource(pDevice_, "./resources/Models/Enemy/boss_animation_fifth_dive.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
    {
        auto model = resource_manager->load_model_resource(pDevice_, "./resources/Models/Enemy/boss_secondarygun.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
    {
        auto model = resource_manager->load_model_resource(pDevice_, "./resources/Models/Enemy/boss_turret.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
    {
        auto model = resource_manager->load_model_resource(pDevice_, "./resources/Models/Enemy/boss_unit.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
}

inline void ModelCashes::Load_PreTitle(ID3D11Device* pDevice_)
{
    {
        auto model = resource_manager->load_model_resource(pDevice_, ".\\resources\\Models\\Enemy\\enemy_ace_tutorial.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
    {
        auto model = resource_manager->load_model_resource(pDevice_, ".\\resources\\Models\\Enemy\\enemy_tutorial.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
    {
        auto model = resource_manager->load_model_resource(pDevice_, ".\\resources\\Models\\Player\\player_twentyfource.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
    {
        auto model = resource_manager->load_model_resource(pDevice_, ".\\resources\\Models\\stage\\title_stage.fbx");
        mpModelCashVec.emplace_back(std::move(model));
    }
}
