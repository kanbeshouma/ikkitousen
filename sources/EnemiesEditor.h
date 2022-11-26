#pragma once
#include"EnemyStructuer.h"
#include <filesystem>
#include <fstream>
#include<map>
#include<string>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include"imgui_include.h"
//****************************************************************
//
// �G�̏����쐬�E�ҏW����N���X
//
//****************************************************************

class EnemyEditor final
{
public:
    EnemyEditor();
    ~EnemyEditor();
     const EnemyParamPack fGetParam(EnemyType Type_);
     void fGuiMenu();

private:
    void fLoad();
    void fSave();
    std::map<std::string, EnemyParamPack> mEnemyParamMap{};
    const char* mFilePath = "./resources/Data/EnemiesParam.json";
};

inline EnemyEditor::EnemyEditor()
{
    fLoad();
}

inline EnemyEditor::~EnemyEditor()
{
    fSave();
}

inline const EnemyParamPack EnemyEditor::fGetParam(EnemyType Type_)
{
    const char* enemyName[] = {
        "Archer","Shield","Sword","Spear",
        "Archer_Ace","Shield_Ace","Sword_Ace","Spear_Ace",
        "Boss","TutorialNoMove","BossUnit"
    };

    // �L�[���������Ă���Ύ擾 �Ȃ���΍쐬

    const char* key = enemyName[static_cast<int>(Type_)];
    const auto& pair =
        mEnemyParamMap.find(key);
    if(pair == mEnemyParamMap.end())
    {
        EnemyParamPack param{};
        param.MaxHp = 1;
        mEnemyParamMap.insert(std::make_pair(key, param));
        return param;
    }
    else
    {
        return pair->second;
    }
}

inline void EnemyEditor::fGuiMenu()
{

#if 0
#ifdef USE_IMGUI
    if (ImGui::TreeNode("EnemyEditor"))
    {
        if (ImGui::Button("Load"))
        {
            fLoad();
        }
        ImGui::TreePop();
    }
#endif

#endif // 0

}

inline void EnemyEditor::fLoad()
{
    // Json�t�@�C������l���擾
    std::filesystem::path path = mFilePath;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);
            o_archive(mEnemyParamMap);
        }
    }
}

inline void EnemyEditor::fSave()
{
    // Json�t�@�C������l���擾
    std::filesystem::path path = mFilePath;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ofstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONOutputArchive o_archive(ifs);
            o_archive(mEnemyParamMap);
        }
    }
}
