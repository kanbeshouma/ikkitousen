#include "DebugConsole.h"

DebugConsole::~DebugConsole()
{
#ifdef DEBUG_CONSOLE
    FreeConsole();
#endif // DEBUG_CONSOLE
}

void DebugConsole::CreateDebugConsole()
{
#ifdef DEBUG_CONSOLE
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
#endif // DEBUG_CONSOLE
}

void DebugConsole::WriteDebugConsole(std::string text, TextColor color)
{
#ifdef DEBUG_CONSOLE
    std::lock_guard<std::mutex> lock(mutex);

    WORD code = FOREGROUND_INTENSITY;
    switch (color)
    {
    case TextColor::White: code = FOREGROUND_INTENSITY; break;
    case TextColor::Red:code = FOREGROUND_RED; break;
    case TextColor::Green: code = FOREGROUND_GREEN; break;
    case TextColor::Blue: code = FOREGROUND_BLUE; break;
    case TextColor::SkyBlue: code = FOREGROUND_BLUE | FOREGROUND_GREEN; break;
    case TextColor::Pink: code = FOREGROUND_RED | FOREGROUND_BLUE; break;
    case TextColor::Yellow: code = FOREGROUND_GREEN | FOREGROUND_RED; break;
    default:
        break;
    }
    SetConsoleTextAttribute(handle, code);
    std::cout << text << std::endl;
    SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY);
#endif // DEBUG_CONSOLE
}
