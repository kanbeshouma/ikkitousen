#pragma once
#include <cassert>
#include <filesystem>
#include <fstream>
#include<string>
#include<map>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/set.hpp>


//--------------------<���\�[�X>--------------------//
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

//--------------------<�V�X�e��>--------------------//
class EventFlagSystem
{
private:
    std::map<std::string, EventFlagResource> EventFlagMap;
public:
    //�Ǎ�
    void Load(const char* FilePath);
    //����
    bool Check(const char* FlagName, const int AnimationNum, float CurrentAnimationFrame);

};

inline bool EventFlagSystem::Check(const char* FlagName, const int AnimationNum, float CurrentAnimationFrame)
{
    //--------------------<�t���O�̗L�����`�F�b�N>--------------------//
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
        //�A�T�[�g���o��
        assert("EventFlagFile Can Not Open"); // �C�x���g�t���O�t�@�C�����J���܂���ł���
    }
}



