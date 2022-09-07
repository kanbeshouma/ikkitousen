#pragma once
#include <cassert>
#include <filesystem>
#include <fstream>
#include<string>
#include<map>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/set.hpp>


//--------------------<リソース>--------------------//
struct EventFlagResource
{
    int AnimatonNumber;
    float BeginFrame;
    float EndFrame;
    template<class T>
    void serialize(T& archive)
    {
        archive(AnimatonNumber, BeginFrame, EndFrame);
    }

};

//--------------------<システム>--------------------//
class EventFlagSystem
{
private:
    std::map<std::string, EventFlagResource> EventFlagMap;
public:
    //読込
    void Load(const char* FilePath);
    //判定
    bool Check(const char* FlagName, const int AnimationNum, float CurrentAnimationFrame);

};

inline bool EventFlagSystem::Check(const char* FlagName, const int AnimationNum, float CurrentAnimationFrame)
{
    //--------------------<フラグの有無をチェック>--------------------//
    if (!EventFlagMap.count(FlagName)) return false;

    const auto& Resource = EventFlagMap.at(FlagName);
    if (Resource.AnimatonNumber != AnimationNum)  return false;
    if (Resource.BeginFrame > CurrentAnimationFrame) return false;
    if (Resource.EndFrame < CurrentAnimationFrame) return false;
    return true;
}

inline void EventFlagSystem::Load(const char* FilePath)
{
    std::filesystem::path cereal_filename(FilePath);
    cereal_filename.replace_extension("flg");
    if (std::filesystem::exists(cereal_filename.c_str()))
    {
        std::ifstream ifs(cereal_filename.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(EventFlagMap);
    }
    else
    {
        //アサートを出力
        assert("EventFlagFile Can Not Open"); // イベントフラグファイルを開けませんでした
    }
}



