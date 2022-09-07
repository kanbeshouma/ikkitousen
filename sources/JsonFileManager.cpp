
#include"JsonFileManager.h"
#include<filesystem>
#include <fstream>


//--------------------<新しいファイルを作成>--------------------//
void JsonManager::CrateNewFile(const char* FileName)
{

    
    std::filesystem::path path = std::string("./resources/Data/") + FileName;
    path.replace_extension(".json");

    // なければ作成
    std::ofstream ofs;
    ofs.open(path.c_str());
    if (ofs)
    {
        ofs.close();
    }

}

void JsonManager::Write(const char* FileName)
{
    // 指定されたファイルを開く

    std::ofstream ofs;
    ofs.open(FileName);
    if (ofs)
    {
       /* nlohmann::json json;
        json["HitPoint"] = 10;
        ofs << json;*/
    }
    else
    {
        throw std::runtime_error("JsonFile Not Found");
    }

}

void JsonManager::Read(const char* FileName)
{
    
}
