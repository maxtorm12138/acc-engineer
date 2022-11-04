#include <spdlog/spdlog.h>
#include <strategy_setter/strategy_setter.h>

using namespace acc_engineer;

BOOL CALLBACK enum_notepad_windows(HWND hwnd, LPARAM param)
{
    std::string window_text;
    window_text.resize(GetWindowTextLength(hwnd));
    GetWindowText(hwnd, window_text.data(), window_text.size() + 1);

    if (window_text.find("PowerShell") != std::string::npos)
    {
        auto win = reinterpret_cast<HWND *>(param);
        *win = hwnd;
    }

    if (!window_text.empty())
    {
        SPDLOG_INFO("window: {}", window_text);
    }
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