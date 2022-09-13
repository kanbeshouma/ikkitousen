#pragma once
#include<Windows.h>
#include<filesystem>
#include<string>
#include<iostream>
#include <mutex>

enum class TextColor
{
    White,
    Red,
    Green,
    Blue
};
class DebugConsole
{
private:
    DebugConsole() {}
    ~DebugConsole();
public:
    static DebugConsole& Instance()
    {
        static  DebugConsole i;
        return i;
    }
private:
    HANDLE handle;
    SMALL_RECT window_size{ 0,0,100,100 };
    //îrëºêßå‰
     std::mutex mutex;
public:
    void CreateDebugConsole();
    void WriteDebugConsole(std::string text, TextColor color = TextColor::White);
};