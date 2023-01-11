#include"EnemiesEditor.h"
#include"Correspondence.h"
 EnemyEditor::EnemyEditor()
{
    //TestSave();
    //TestLoad();
     //<Httpリクエストに成功した時>//
     if (CorrespondenceManager::Instance().HttpRequest())
     {
         //<成功した場合はデータをMAPに登録する>//
         for (auto& data : CorrespondenceManager::Instance().GetWebEnemyParamPack())
         {
             EnemyParamPack param;
             param.MaxHp = data.MaxHp;
             param.AttackPower = data.AttackPower;
             param.InvincibleTime = data.InvincibleTime;
             param.BodyCapsuleRad = data.BodyCapsuleRad;
             param.AttackCapsuleRad = data.AttackCapsuleRad;
             param.StunTime = data.StunTime;

             mEnemyParamMap.insert(std::make_pair(data.enemy_name, param));
         }
     }
     //<失敗した時>//
     else
     {
         fLoad();
     }
}

 EnemyEditor::~EnemyEditor()
{
    fSave();
}

 const EnemyParamPack EnemyEditor::fGetParam(EnemyType Type_)
{
    const char* enemyName[] = {
        "Archer","Shield","Sword","Spear",
        "Archer_Ace","Shield_Ace","Sword_Ace","Spear_Ace",
        "Boss","TutorialNoMove","BossUnit"
    };

    // キーを検索してあれば取得 なければ作成

    const char* key = enemyName[static_cast<int>(Type_)];
    const auto& pair =
        mEnemyParamMap.find(key);
    if (pair == mEnemyParamMap.end())
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

 void EnemyEditor::fGuiMenu()
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

 void EnemyEditor::fLoad()
{

    // Jsonファイルから値を取得
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

 void EnemyEditor::TestLoad()
{

    // Jsonファイルから値を取得
    std::filesystem::path path = mTestFilePath;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);
            o_archive(web_enemy_param);
        }
    }
    web_enemy_param;
}

 void EnemyEditor::fSave()
{
    // Jsonファイルから値を取得
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

 void EnemyEditor::TestSave()
{
    {
        WebEnemy::WebEnemyParamPack data;
        data.enemy_name = "Archer";
        web_enemy_param.emplace_back(data);
    }
    {
        WebEnemy::WebEnemyParamPack data;
        data.enemy_name = "Archer_Ace";
        web_enemy_param.emplace_back(data);
    }




    // Jsonファイルから値を取得
    std::filesystem::path path = mTestFilePath;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ofstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONOutputArchive o_archive(ifs);
            o_archive(web_enemy_param);
        }
    }
}
