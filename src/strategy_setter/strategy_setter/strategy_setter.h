#ifndef ACC_ENGINEER_STRATEGY_SETTER_STRATEGY_SETTER_H
#define ACC_ENGINEER_STRATEGY_SETTER_STRATEGY_SETTER_H

// std
#include <tuple>

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

template<typename Key>
std::tuple<Key, int> press_count(std::tuple<Key, Key> keys, float current, float target)
{
    int press = std::rint((target - current) * 10.f);
    if (press < 0)
    {
        return {std::get<0>(keys), -press};
    }
    else
    {
        return {std::get<1>(keys), -press};
    }
}

template<typename Key>
std::tuple<Key, int> press_count(std::tuple<Key, Key> keys, int current, int target)
{
    int press = target - current;
    if (press < 0)
    {
        return {std::get<0>(keys), -press};
    }
    else
    {
        return {std::get<1>(keys), press};
    }
}

class direct_input : public boost::noncopyable
{
public:
    bool press(std::string_view key, uint64_t presses = 1, std::chrono::steady_clock::duration interval = std::chrono::milliseconds(0));

    bool key_down(std::string_view key);

    bool key_up(std::string_view key);

private:
    const static std::unordered_map<std::string_view, WORD> keyboard_code;
};

class strategy_setter : public boost::noncopyable
{
public:
    strategy_setter(net::any_io_executor &executor);

    ~strategy_setter();

    net::awaitable<void> set(uint64_t version, structure::Strategy strategy);

private:
    void do_set();

private:
    net::experimental::concurrent_channel<void(sys::error_code, structure::Strategy, uint64_t version)> strategy_channel_;
    shared_memory::reader reader_;
    std::jthread strategy_thread_;
    std::atomic<uint64_t> strategy_version_;
};

} // namespace acc_engineer::strategy_setter

#endif