#include "DebugConsole.h"

DebugConsole::~DebugConsole()
{
#ifdef _DEBUG
    FreeConsole();
#endif // DEBUG
}

void DebugConsole::CreateDebugConsole()
{
#ifdef _DEBUG
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONIN$", "r", stdin);
    setvbuf(stdout, nullptr, _IONBF, 0);
    handle = GetStdHandle(STD_OUTPUT_HANDLE);    // DWORD nStdHandle
    if (handle != INVALID_HANDLE_VALUE)
    {
        SetConsoleWindowInfo(
            handle, // HANDLE hConsoleOutput
            TRUE,           // BOOL bAbsolute
            &window_size);// CONST SMALL_RECT *lpConsoleWindow
    }
#endif // DEBUG
}

void DebugConsole::WriteDebugConsole(std::string text, TextColor color)
{
#ifdef _DEBUG
    std::lock_guard<std::mutex> lock(mutex);

    WORD code = FOREGROUND_INTENSITY;
    switch (color)
    {
    case TextColor::White: code = FOREGROUND_INTENSITY; break;
    case TextColor::Red:code = FOREGROUND_RED; break;
    case TextColor::Green: code = FOREGROUND_GREEN; break;
    case TextColor::Blue: code = FOREGROUND_BLUE; break;
    default:
        break;
    }
    SetConsoleTextAttribute(handle, code);
    std::cout << text << std::endl;
    SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY);
#endif // DEBUG
}
