#include "strategy_setter.h"

// std
#include <unordered_map>

// spdlog
#include <spdlog/spdlog.h>

// windows
#include <WinUser.h>

namespace acc_engineer::strategy_setter {

const std::unordered_map<std::string_view, WORD> direct_input::keyboard_code{
    {"p", 0x19},
    {"up", MapVirtualKey(0x26, MAPVK_VK_TO_VSC)},
    {"left", MapVirtualKey(0x25, MAPVK_VK_TO_VSC)},
    {"down", MapVirtualKey(0x28, MAPVK_VK_TO_VSC)},
    {"right", MapVirtualKey(0x27, MAPVK_VK_TO_VSC)},
};

bool direct_input::press(std::string_view key, uint64_t presses, std::chrono::steady_clock::duration interval)
{
    uint64_t completed_presses = 0;

    for (uint64_t i = 0; i < presses; i++)
    {
        const bool downed = key_down(key);
        const bool upped = key_up(key);
        if (downed && upped)
        {
            completed_presses++;
        }

        std::this_thread::sleep_for(interval);
    }

    return presses == completed_presses;
}

bool direct_input::key_down(std::string_view key)
{
    const static std::unordered_set<std::string_view> ARROW_KEYS = {"up", "left", "down", "right"};

    if (!keyboard_code.contains(key))
    {
        return false;
    }

    std::vector<INPUT> input_events;
    input_events.reserve(2);

    DWORD keyboard_flags = KEYEVENTF_SCANCODE;

    if (ARROW_KEYS.contains(key))
    {
        keyboard_flags |= KEYEVENTF_EXTENDEDKEY;
        // if numlock is on and an arrow key is being pressed, we need to send an additional scancode
        // https://stackoverflow.com/questions/14026496/sendinput-sends-num8-when-i-want-to-send-vk-up-how-come
        // https://handmade.network/wiki/2823-keyboard_inputs_-_scancodes,_raw_input,_text_input,_key_names
        if (GetKeyState(0x90))
        {
            const INPUT extra_input{.type = INPUT_KEYBOARD, .ki = {.wScan = 0xE0, .dwFlags = KEYEVENTF_SCANCODE}};
            input_events.push_back(extra_input);
        }
    }

    const INPUT input{.type = INPUT_KEYBOARD, .ki = {.wScan = keyboard_code.at(key), .dwFlags = keyboard_flags}};
    input_events.push_back(input);

    const UINT inserted = SendInput(input_events.size(), input_events.data(), sizeof(INPUT));
    return inserted == input_events.size();
}
bool direct_input::key_up(std::string_view key)
{
    const static std::unordered_set<std::string_view> ARROW_KEYS = {"up", "left", "down", "right"};

    if (!keyboard_code.contains(key))
    {
        return false;
    }

    std::vector<INPUT> input_events;
    input_events.reserve(2);

    DWORD keyboard_flags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    if (ARROW_KEYS.contains(key))
    {
        keyboard_flags |= KEYEVENTF_EXTENDEDKEY;
    }

    const INPUT input{.type = INPUT_KEYBOARD, .ki = {.wScan = keyboard_code.at(key), .dwFlags = keyboard_flags}};
    input_events.push_back(input);

    // if numlock is on and an arrow key is being pressed, we need to send an additional scancode
    // https://stackoverflow.com/questions/14026496/sendinput-sends-num8-when-i-want-to-send-vk-up-how-come
    // https://handmade.network/wiki/2823-keyboard_inputs_-_scancodes,_raw_input,_text_input,_key_names
    if (ARROW_KEYS.contains(key) && GetKeyState(0x90))
    {
        const INPUT extra_input{.type = INPUT_KEYBOARD, .ki = {.wScan = 0xE0, .dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP}};
        input_events.push_back(extra_input);
    }

    const UINT inserted = SendInput(input_events.size(), input_events.data(), sizeof(INPUT));
    return inserted == input_events.size();
}

strategy_setter::strategy_setter(net::any_io_executor &executor)
    : strategy_channel_(executor, 1)
{
    strategy_thread_ = std::jthread([this]() { do_set(); });
}

strategy_setter::~strategy_setter()
{
    strategy_channel_.close();
}

net::awaitable<void> strategy_setter::set(uint64_t version, structure::Strategy strategy)
{
    co_await strategy_channel_.async_send({}, std::move(strategy), version, net::use_awaitable);
}

void strategy_setter::do_set()
{
    sys::error_code error_code;

    try
    {
        while (true)
        {
            auto [strategy, version] = strategy_channel_.async_receive(net::use_future).get();
            if (version <= strategy_version_)
            {
                continue;
            }

            strategy_version_ = version;
        }
    }
    catch (sys::system_error &ex)
    {
        SPDLOG_INFO("strategy_setter do_set system_error: {}", ex.what());
    }
}
} // namespace acc_engineer::strategy_setter