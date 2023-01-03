#pragma once
#include"EnemyStructuer.h"
#include <filesystem>
#include <fstream>
#include<map>
#include<string>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include"imgui_include.h"
//****************************************************************
//
// 敵の情報を作成・編集するクラス
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
    void TestLoad();
    void fSave();
    void TestSave();
    std::map<std::string, EnemyParamPack> mEnemyParamMap{};
    const char* mFilePath = "./resources/Data/EnemiesParam.json";
    const char* mTestFilePath = "./resources/Data/Test.json";
    std::vector<WebEnemyParamPack> web_enemy_param{};

};

