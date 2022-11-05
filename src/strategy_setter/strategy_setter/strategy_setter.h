#ifndef ACC_ENGINEER_STRATEGY_SETTER_STRATEGY_SETTER_H
#define ACC_ENGINEER_STRATEGY_SETTER_STRATEGY_SETTER_H

// std
#include <tuple>

// SPDLOG
#include <spdlog/spdlog.h>

// boost
#include <boost/noncopyable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

// module
#include <shared_memory/reader.h>

// proto

#include "proto/struct.pb.h"
namespace acc_engineer::strategy_setter {

namespace net = boost::asio;
namespace sys = boost::system;

class direct_input : public boost::noncopyable
{
public:
    bool press(std::string_view key, uint64_t presses = 1, std::chrono::steady_clock::duration interval = std::chrono::milliseconds(0));

    static bool key_down(std::string_view key);

    static bool key_up(std::string_view key);

    template<typename T>
    bool press_calculate(std::tuple<std::string_view, std::string_view> keys, T current, T target, std::chrono::steady_clock::duration interval = std::chrono::milliseconds(0),
        std::enable_if_t<std::is_floating_point_v<T>, int *> = nullptr)
    {
        const int presses = std::rint((target - current) * 10.f);
        if (presses < 0)
        {
            return press(std::get<0>(keys), -presses, interval);
        }
        else
        {
            return press(std::get<1>(keys), presses, interval);
        }
    }

    template<typename T>
    requires std::is_integral_v<T>
    bool press_calculate(std::tuple<std::string_view, std::string_view> keys, T current, T target, std::chrono::steady_clock::duration interval = std::chrono::milliseconds(0),
        std::enable_if_t<std::is_integral_v<T>, int *> = nullptr)
    {
        const int presses = target - current;
        if (presses < 0)
        {
            return press(std::get<0>(keys), -presses, interval);
        }
        else
        {
            return press(std::get<1>(keys), presses, interval);
        }
    }

private:
    const static std::unordered_map<std::string_view, WORD> keyboard_code;

    const static std::unordered_set<std::string_view> arrow_key;
};

class strategy_setter : public boost::noncopyable
{
public:
    strategy_setter(net::any_io_executor &executor);

    ~strategy_setter();

    net::awaitable<void> set(uint64_t version, structure::Strategy strategy);

private:
    void do_set();

    bool do_acc_foreground();

    bool is_floating_point_close(float a, float b);

private:
    net::experimental::concurrent_channel<void(sys::error_code, structure::Strategy, uint64_t version)> strategy_channel_;
    shared_memory::reader reader_;
    std::jthread strategy_thread_;
    std::atomic<uint64_t> strategy_version_;
    direct_input direct_input_;
};

} // namespace acc_engineer::strategy_setter

#endif