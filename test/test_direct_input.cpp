#include <spdlog/spdlog.h>
#include <strategy_setter/strategy_setter.h>

using namespace acc_engineer;

BOOL CALLBACK enum_notepad_windows(HWND hwnd, LPARAM param)
{
    WCHAR window_name[512];
    auto length = GetWindowTextW(hwnd, window_name, 511);

    if (std::wstring(window_name, length).find(L"PowerShell") != std::string::npos)
    {
        auto win = reinterpret_cast<HWND *>(param);
        *win = hwnd;
    }
    std::wcout << L"Window" << std::wstring(window_name, length) << std::endl;
    return true;
}

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::trace);
    HWND handle;
    if (EnumWindows(&enum_notepad_windows, reinterpret_cast<LPARAM>(&handle)))
    {
        if (SetForegroundWindow(handle))
        {
            strategy_setter::direct_input direct_input;
            direct_input.press("p", 10);
            return 0;
        }
    }

    return -1;
}