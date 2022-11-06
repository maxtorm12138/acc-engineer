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

const std::unordered_set<std::string_view> direct_input::arrow_key = {"up", "left", "down", "right"};

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
    if (!keyboard_code.contains(key))
    {
        return false;
    }

    std::vector<INPUT> input_events;
    input_events.reserve(2);

    DWORD keyboard_flags = KEYEVENTF_SCANCODE;

    if (arrow_key.contains(key))
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

    const UINT inserted = SendInput(static_cast<UINT>(input_events.size()), input_events.data(), sizeof(INPUT));
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

    if (arrow_key.contains(key))
    {
        keyboard_flags |= KEYEVENTF_EXTENDEDKEY;
    }

    const INPUT input{.type = INPUT_KEYBOARD, .ki = {.wScan = keyboard_code.at(key), .dwFlags = keyboard_flags}};
    input_events.push_back(input);

    // if numlock is on and an arrow key is being pressed, we need to send an additional scancode
    // https://stackoverflow.com/questions/14026496/sendinput-sends-num8-when-i-want-to-send-vk-up-how-come
    // https://handmade.network/wiki/2823-keyboard_inputs_-_scancodes,_raw_input,_text_input,_key_names
    if (arrow_key.contains(key) && GetKeyState(0x90))
    {
        const INPUT extra_input{.type = INPUT_KEYBOARD, .ki = {.wScan = 0xE0, .dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP}};
        input_events.push_back(extra_input);
    }

    const UINT inserted = SendInput(static_cast<UINT>(input_events.size()), input_events.data(), sizeof(INPUT));
    return inserted == input_events.size();
}

strategy_setter::strategy_setter(net::any_io_executor &executor)
    : strategy_channel_(executor)
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
        using namespace std::chrono_literals;
        while (true)
        {
            auto [strategy, version] = strategy_channel_.async_receive(net::use_future).get();
            if (version <= strategy_version_)
            {
                continue;
            }

            // step 1 set acc to foreground
            if (!do_acc_foreground())
            {
                continue;
            }

            // step 2 show MFD
            {
				SPDLOG_DEBUG("Press P to show MFD");
            	direct_input_.press("p", 1, 10ms);
            	SPDLOG_DEBUG("Press P to show MFD -- DONE");
            }

            // step 3 add fuel
            {
				SPDLOG_DEBUG("Press down 2 times to fuel line");
            	direct_input_.press("down", 2, 10ms);
            	SPDLOG_DEBUG("Press down 2 times to fuel line -- DONE");

				auto mfd_fuel = static_cast<int>(reader_.mfd_fuel_to_add());
            	auto target_fuel = static_cast<int>(std::clamp(strategy.fuel(), 0.f, reader_.fuel_tank_capacity()));
            	SPDLOG_DEBUG("Add fuel mfd_fuel: {} target_fuel: {}", mfd_fuel, target_fuel);
            	direct_input_.press_calculate({"left", "right"}, mfd_fuel, target_fuel, 10ms);
            	SPDLOG_DEBUG("Add fuel mfd_fuel: {} target_fuel: {} -- DONE", mfd_fuel, target_fuel);
            }


            // step 4  check if tyre set is on wet, tyre set will be disable
            //         so going down 5 times will be FR instead of FL
			int next_down_times = 0;
            {
				SPDLOG_DEBUG("Press down 5 times to FR OR FL line");
            	direct_input_.press("down", 5, 10ms);
            	SPDLOG_DEBUG("Press down 5 times to FR OR FL line -- DONE");

            	auto old_rf = reader_.mfd_tyre_pressures().right_front();
            	SPDLOG_DEBUG("Press left 1 times");
            	direct_input_.press("left", 1, 10ms);
            	SPDLOG_DEBUG("Press left 1 times -- DONE");
            	auto new_rf = reader_.mfd_tyre_pressures().right_front();

            	bool is_mfd_wet_tyre = !is_floating_point_close(old_rf, new_rf);

            	SPDLOG_DEBUG("is_mfd_wet_tyre: {} old_rf: {} new_rf: {}", is_mfd_wet_tyre, old_rf, new_rf);
            	SPDLOG_DEBUG("Press right 1 times");
            	direct_input_.press("right", 1, 10ms);
            	SPDLOG_DEBUG("Press right 1 times -- DONE");

            	SPDLOG_DEBUG("Press up 5 times to fuel line");
            	direct_input_.press("up", 5, 10ms);
            	SPDLOG_DEBUG("Press up 5 times to fuel line -- DONE");
				next_down_times = is_mfd_wet_tyre ? 2 : 3;
            }
	
            // step 5 change tyre compound
            {
				SPDLOG_DEBUG("Press down {} times to compound line", next_down_times);
            	direct_input_.press("down", next_down_times, 10ms);
            	SPDLOG_DEBUG("Press down {} times to compound line -- done", next_down_times);

            	switch (strategy.tyre_compound())
            	{
            	case structure::DRY:
            	    SPDLOG_DEBUG("Press left to dry tyre");
            	    direct_input_.press("left", 1, 10ms);
            	    SPDLOG_DEBUG("Press left to dry tyre -- DONE");
					// pressure data might be invalidate (pressing left when on
					// dry compound set pressure as currently used)
            	    break;
            	case structure::WET:
            	    SPDLOG_DEBUG("Press left to wet tyre");
            	    direct_input_.press("right", 1, 10ms);
            	    SPDLOG_DEBUG("Press left to wet tyre -- DONE");
            	    break;
            	default:;
            }

            }
            // step 6 change tyre set
            {
				if (strategy.tyre_set() == structure::DRY)
            	{
            	    SPDLOG_DEBUG("Press up to tyre set line");
            	    direct_input_.press("up", 1, 10ms);
            	    SPDLOG_DEBUG("Press up to tyre set line -- DONE");

            	    auto current_tyre_set = reader_.current_tyre_set();
            	    auto mfd_tyre_set = reader_.mfd_tyre_set();
					SPDLOG_DEBUG("mfd_tyre_set: {} current_tyre_set: {}", mfd_tyre_set, current_tyre_set);
            	    next_down_times = 3;
            	}
            	else
            	{
            	    next_down_times = 2;
            	}
            }

            // step 7 change tyre pressure
            {
				SPDLOG_DEBUG("Press down {} times to left front tyre line", next_down_times);
            	direct_input_.press("down", next_down_times, 10ms);
            	SPDLOG_DEBUG("Press down {} times to left front tyre line -- DONE", next_down_times);

            	auto mfd_tyre_pressure = reader_.mfd_tyre_pressures();
            	auto strategy_tyre_pressure = strategy.tyre_pressures();
            	// clamp to safe value between 20.4 and 35.0
            	strategy_tyre_pressure.set_left_front(clamp_pressure(strategy_tyre_pressure.left_front()));
            	strategy_tyre_pressure.set_right_front(clamp_pressure(strategy_tyre_pressure.right_front()));
            	strategy_tyre_pressure.set_left_rear(clamp_pressure(strategy_tyre_pressure.left_rear()));
            	strategy_tyre_pressure.set_right_rear(clamp_pressure(strategy_tyre_pressure.right_rear()));

            	SPDLOG_DEBUG("mfd_tyre_pressure: [{}] strategy_tyre_pressure: [{}]", mfd_tyre_pressure.ShortDebugString(), strategy_tyre_pressure.ShortDebugString());
				
            	direct_input_.press_calculate({"left", "right"}, mfd_tyre_pressure.left_front(), strategy_tyre_pressure.left_front(), 10ms);
            	direct_input_.press("down", 1, 10ms);
            	direct_input_.press_calculate({"left", "right"}, mfd_tyre_pressure.right_front(), strategy_tyre_pressure.right_front(), 10ms);
            	direct_input_.press("down", 1, 10ms);
            	direct_input_.press_calculate({"left", "right"}, mfd_tyre_pressure.left_rear(), strategy_tyre_pressure.left_rear(), 10ms);
            	direct_input_.press("down", 1, 10ms);
            	direct_input_.press_calculate({"left", "right"}, mfd_tyre_pressure.right_rear(), strategy_tyre_pressure.right_rear(), 10ms);
            }


            strategy_version_ = version;
        }
    }
    catch (sys::system_error &ex)
    {
        SPDLOG_INFO("strategy_setter do_set system_error: {}", ex.what());
    }
}

bool strategy_setter::do_acc_foreground()
{
    using namespace std::chrono_literals;

    auto window_enumerator = [](HWND hwnd, LPARAM param) -> BOOL {
        std::string window_text;
        window_text.resize(GetWindowTextLength(hwnd));

        GetWindowText(hwnd, window_text.data(), static_cast<int>(window_text.size() + 1));

        if (window_text.find("AC2") != std::string::npos)
        {
            SPDLOG_DEBUG("found ac2 window");
            (*reinterpret_cast<HWND *>(param)) = hwnd;
        }

        return TRUE;
    };

    HWND ac2_win = nullptr;
    if (!EnumWindows(window_enumerator, reinterpret_cast<LPARAM>(&ac2_win)))
    {
        SPDLOG_ERROR("EnumWindows fail");
        return false;
    }

    if (ac2_win == nullptr)
    {
        SPDLOG_ERROR("EnumWindows fail");
        return false;
    }

    if (GetForegroundWindow() == ac2_win)
    {
        SPDLOG_INFO("AC2 already foreground");
        return true;
    }

    if (!SetForegroundWindow(ac2_win))
    {
        SPDLOG_ERROR("SetForegroundWindow fail error: {}", std::system_category().message(GetLastError()));
        return false;
    }

    std::this_thread::sleep_for(200ms);
    if (GetForegroundWindow() != ac2_win)
    {
        SPDLOG_INFO("do AC2 foreground fail");
        return false;
    }

    return true;
}

bool strategy_setter::is_floating_point_close(float a, float b)
{
    return std::abs(a - b) < 1e-5;
}

float strategy_setter::clamp_pressure(float original)
{
    return std::clamp(original, 20.4f, 35.0f);
}
} // namespace acc_engineer::strategy_setter