#pragma once

//****************************************************************
// 
//  Json�t�@�C���Ƃ��Ă̏����o�����T�|�[�g
// 
//****************************************************************

class JsonManager
{
public:
    JsonManager() = default;
    ~JsonManager() = default;


   static void CrateNewFile(const char* FileName);
   static void Write(const char* FileName);
   static void Read(const char* FileName);
};


