#pragma once
#include <cassert>

#include"lua.hpp"
#include"lualib.h"
#include"lauxlib.h"
//****************************************************************
// 
// Lua 
// 
//****************************************************************
namespace ULua
{
    //--------------------<State������������֐�>--------------------//
    inline lua_State* fCreateLuaState()
    {
        
    }
    //--------------------<Lua�t�@�C�����J��>--------------------//
    inline void fLuaOpenFile(lua_State* Lua, const char* FileName)
    {
        
    }

    //--------------------<Lua�̊֐����Ăяo��>--------------------//
    inline void fLuaCallFunc(lua_State* State_,
        int ArgCounts_, // �����̐�
        int RetCounts_) // �߂�l�̐�
    {
        
    }



}


class LuaWorld final 
{
    //****************************************************************
    // 
    // �֐�
    // 
    //****************************************************************
public:
    LuaWorld();
    ~LuaWorld();

     void fLoadFile(const char* FileName_) const;// �t�@�C�������[�h
     void fDestroyStack(); // �X�^�b�N��S�폜
     void fCallFunc(int ArgCounts_, // �����̐�
         int RetCounts_) const; // �߂�l�̐�

   //--------------------<�Z�b�^�[�֐�>--------------------//
    void fSetNumeric(double Numeric_) const;
    void fSetBoolean(bool Is) const;
    void fSetFunction(const char* FunctionName_) const;

    //--------------------<�Q�b�^�[�֐�>--------------------//
    [[nodiscard]] double fGetGlobalFloat(const char* NumericName_) const;
    [[nodiscard]] double fGetDouble(int StackPoint_) const;
    [[nodiscard]] bool fGetBool(int StackPoint_)const;

    //--------------------<�f�o�b�O�֐�>--------------------//
    void fDebugShowStack();

private:
    void fCreate();  // �쐬�i�������j
    void fClose() const;   // �I��

private:
    lua_State* mLuaState{ nullptr };
};