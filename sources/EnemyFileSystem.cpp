//#include "EnemyFileSystem.h"
//#include<cereal/archives/json.hpp>
//#include<cereal/archives/binary.hpp>
//#include <cereal/types/vector.hpp>
//#include <string>
//#include<filesystem>
//#include<fstream>
//
//
////****************************************************************
//// 
//// 敵のファイル管理システム 
//// 
////****************************************************************
//
//void EnemyFileSystem::fLoadFromJson(std::vector<EnemySource>& SourceVec_, const char* FileName_)
//{
//    // Jsonファイルから値を取得
//    std::filesystem::path path = FileName_;
//    path.replace_extension(".json");
//    if (std::filesystem::exists(path.c_str()))
//    {
//        std::ifstream ifs;
//        ifs.open(path);
//        if (ifs)
//        {
//            cereal::JSONInputArchive o_archive(ifs);
//            o_archive(SourceVec_);
//        }
//    }
//    else
//    {
//        _ASSERT_EXPR(0, "Json Not Found");
//    }
//}
//
//void EnemyFileSystem::fSaveToJson(std::vector<EnemySource>& SourceVec_, const char* FileName_)
//{
//    // Jsonにかきだし
//    std::filesystem::path path = (std::string("./resources/Data/")+FileName_);
//    path.replace_extension(".json");
//    std::ofstream ifs(path);
//    if (ifs)
//    {
//        cereal::JSONOutputArchive o_archive(ifs);
//        o_archive(SourceVec_);
//    }
//}
//
//void EnemyFileSystem::fLoadFromJson(std::map<std::string, EnemyData>& SourceMap_, const char* FileName_)
//{
//    // Jsonファイルから値を取得
//    std::filesystem::path path = FileName_;
//    path.replace_extension(".json");
//    if (std::filesystem::exists(path.c_str()))
//    {
//        std::ifstream ifs;
//        ifs.open(path);
//        if (ifs)
//        {
//            cereal::JSONInputArchive o_archive(ifs);
//            o_archive(SourceMap_);
//        }
//    }
//    else
//    {
//        _ASSERT_EXPR(0, "Json Not Found");
//    }
//}
//
//void EnemyFileSystem::fSaveToJson(std::map<std::string, EnemyData>& SourceMap_, const char* FileName_)
//{
//    // Jsonにかきだし
//    std::filesystem::path path = (std::string("./resources/Data/") + FileName_);
//    path.replace_extension(".json");
//    std::ofstream ifs(path);
//    if (ifs)
//    {
//        cereal::JSONOutputArchive o_archive(ifs);
//        o_archive(SourceMap_);
//    }
//}
