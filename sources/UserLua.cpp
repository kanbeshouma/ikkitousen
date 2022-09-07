#include"UserLua.h"
#include <stdexcept>
#include<Windows.h>
#include<string>
LuaWorld::LuaWorld()
{
    fCreate();
}

LuaWorld::~LuaWorld()
{
    fClose();
}

void LuaWorld::fCreate()
{
    mLuaState = luaL_newstate();
    luaL_openlibs(mLuaState);
    luaopen_base(mLuaState);
}

void LuaWorld::fClose() const
{
    lua_close(mLuaState);
}

void LuaWorld::fLoadFile(const char* FileName_) const
{
    const auto Result = luaL_dofile(mLuaState, FileName_);  // Luaファイルを開く
        // Luaファイルを開けなかったらエラーメッセージを出力
    assert(!Result);
}

void LuaWorld::fDestroyStack()
{
    // スタック削除処理
    const int LuaStackSize = lua_gettop(mLuaState);
    for (int i = 0; i < LuaStackSize; i++)
    {
        lua_pop(mLuaState, -1);
    }
}

void LuaWorld::fCallFunc(int ArgCounts_, int RetCounts_) const
{
    const auto errorMassage = lua_pcall(mLuaState, ArgCounts_, RetCounts_, 0);
    if (errorMassage != 0)
    {
        // ０以外ならエラーをキャッチ
        assert(lua_tostring(mLuaState, -1));
    }
}

void LuaWorld::fSetNumeric(double Numeric_) const
{
    lua_pushnumber(mLuaState, Numeric_);
}

void LuaWorld::fSetBoolean(bool Is) const
{
    lua_pushboolean(mLuaState, 0);
}

void LuaWorld::fSetFunction(const char* FunctionName_) const
{
    lua_getglobal(mLuaState, FunctionName_);
}

double LuaWorld::fGetGlobalFloat(const char* NumericName_) const
{
    lua_getglobal(mLuaState, NumericName_);
    return  luaL_checknumber(mLuaState, -1);
}

double LuaWorld::fGetDouble(int StackPoint_) const
{
    return luaL_checknumber(mLuaState, StackPoint_);
}

bool LuaWorld::fGetBool(int StackPoint_) const
{
    return lua_toboolean(mLuaState,StackPoint_);
}

void LuaWorld::fDebugShowStack()
{
    // Stackの中身を見る

    OutputDebugStringA("----------------LuaStacks-------------------------\n");

    const int num = lua_gettop(mLuaState);
    OutputDebugStringA(std::to_string(num).c_str());

    if (num == 0) {
        OutputDebugStringA("StacksValue=0");
        return;
    }
    for (int i = num; i >= 1; i--) {
        const int type = lua_type(mLuaState, i);
        switch (type) {
        case LUA_TNIL:
            OutputDebugStringA("Nil");
            break;
        case LUA_TBOOLEAN:
           OutputDebugStringA(lua_toboolean(mLuaState, i) ? "true" : "false");
            break;
        case LUA_TLIGHTUSERDATA:
            OutputDebugStringA("LIGHTUSERDATA");
            break;
        case LUA_TNUMBER:
            OutputDebugStringA(std::to_string(lua_tonumber(mLuaState, i)).c_str());
            break;
        case LUA_TSTRING:
            OutputDebugStringA(lua_tostring(mLuaState, i));
            break;
        case LUA_TTABLE:
            OutputDebugStringA("TABLE\n");
            break;
        case LUA_TFUNCTION:
            OutputDebugStringA("FUNCTION\n");
            break;
        case LUA_TUSERDATA:
            OutputDebugStringA("USERDATA\n");
            break;
        case LUA_TTHREAD:
            OutputDebugStringA("THREAD\n");
            break;
        }
        OutputDebugStringA("\n");
    }
}
