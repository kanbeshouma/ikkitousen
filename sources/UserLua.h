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
    //--------------------<Stateを初期化する関数>--------------------//
    inline lua_State* fCreateLuaState()
    {
        
    }
    //--------------------<Luaファイルを開く>--------------------//
    inline void fLuaOpenFile(lua_State* Lua, const char* FileName)
    {
        
    }

    //--------------------<Luaの関数を呼び出す>--------------------//
    inline void fLuaCallFunc(lua_State* State_,
        int ArgCounts_, // 引数の数
        int RetCounts_) // 戻り値の数
    {
        
    }



}


class LuaWorld final 
{
    //****************************************************************
    // 
    // 関数
    // 
    //****************************************************************
public:
    LuaWorld();
    ~LuaWorld();

     void fLoadFile(const char* FileName_) const;// ファイルをロード
     void fDestroyStack(); // スタックを全削除
     void fCallFunc(int ArgCounts_, // 引数の数
         int RetCounts_) const; // 戻り値の数

   //--------------------<セッター関数>--------------------//
    void fSetNumeric(double Numeric_) const;
    void fSetBoolean(bool Is) const;
    void fSetFunction(const char* FunctionName_) const;

    //--------------------<ゲッター関数>--------------------//
    [[nodiscard]] double fGetGlobalFloat(const char* NumericName_) const;
    [[nodiscard]] double fGetDouble(int StackPoint_) const;
    [[nodiscard]] bool fGetBool(int StackPoint_)const;

    //--------------------<デバッグ関数>--------------------//
    void fDebugShowStack();

private:
    void fCreate();  // 作成（初期化）
    void fClose() const;   // 終了

private:
    lua_State* mLuaState{ nullptr };
};