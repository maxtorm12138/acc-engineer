#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>
#include <strategy_setter/strategy_setter.h>
#include "proto/struct.pb.h"
#include <boost/asio.hpp>
using namespace acc_engineer;
namespace net = boost::asio;
namespace sys = boost::system;

net::awaitable<void> co_main()
{
    using namespace std::chrono_literals;
    auto executor = co_await net::this_coro::executor;

    strategy_setter::strategy_setter setter(executor);

    structure::Strategy strategy;
    strategy.set_fuel(20.);
    co_await setter.set(1, strategy);

    net::steady_timer timer(executor);

    timer.expires_after(10s);

    co_await timer.async_wait(net::use_awaitable);
}

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::trace);

    net::io_context context;

    net::co_spawn(context, co_main(), net::detached);

    context.run();
}