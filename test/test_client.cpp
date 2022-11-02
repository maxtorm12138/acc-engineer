#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include "rpc/stub.h"
#include "rpc/method.h"

#include "proto/service.pb.h"

namespace net = boost::asio;
namespace rpc = acc_engineer::rpc;
using namespace acc_engineer;

rpc::methods methods;
std::shared_ptr<rpc::tcp_stub> tcp_stub;
std::shared_ptr<rpc::udp_stub> udp_stub;

net::awaitable<DriverUpdate::Response> driver_update(const rpc::context &context, const DriverUpdate::Request &request)
{
    SPDLOG_DEBUG("DriverUpdate: {}", request.ShortDebugString());
    co_return DriverUpdate::Response{};
}

net::awaitable<void> connect(int argc, char *argv[])
{

    auto executor = co_await net::this_coro::executor;

    net::ip::tcp::resolver resolver(executor);
    auto endpoints = co_await resolver.async_resolve(argv[1], argv[2], net::use_awaitable);

    net::ip::tcp::socket tcp_socket(executor);
    auto connected = co_await net::async_connect(tcp_socket, endpoints.begin(), endpoints.end(), net::use_awaitable);

    tcp_stub = rpc::tcp_stub::create(std::move(tcp_socket), methods);

    net::co_spawn(
        executor, []() { return tcp_stub->async_run(); }, net::detached);

    AuthenticationTCP::Request tcp_authen_request;
    tcp_authen_request.set_driver_name(argv[3]);
    tcp_authen_request.set_password(argv[4]);

    auto tcp_authen_response = co_await tcp_stub->async_call<AuthenticationTCP>(tcp_authen_request);
    if (tcp_authen_response.error_code() != 0)
    {
        throw std::logic_error(fmt::format("{} {}", tcp_authen_response.error_code(), tcp_authen_response.error_message()));
    }

    net::ip::udp::socket udp_socket(executor);
    co_await udp_socket.async_connect(net::ip::udp::endpoint{connected->endpoint().address(), connected->endpoint().port()}, net::use_awaitable);
    udp_stub = rpc::udp_stub::create(std::move(udp_socket), methods);

    net::co_spawn(
        executor, []() { return udp_stub->async_run(); }, net::detached);

    AuthenticationUDP::Request udp_authen_request;
    udp_authen_request.set_driver_name(argv[3]);
    udp_authen_request.set_password(argv[4]);
    udp_authen_request.set_driver_id(tcp_authen_response.driver_id());

    auto udp_authen_response = co_await udp_stub->async_call<AuthenticationUDP>(udp_authen_request);
    if (udp_authen_response.error_code() != 0)
    {
        throw std::logic_error(fmt::format("{} {}", udp_authen_response.error_code(), udp_authen_response.error_message()));
    }
}

net::awaitable<void> keepalive()
{
    using namespace std::chrono_literals;
    auto executor = co_await net::this_coro::executor;
    net::steady_timer timer(executor);

    while (true)
    {
        timer.expires_after(20s);
        co_await timer.async_wait(net::use_awaitable);
        co_await tcp_stub->async_call<Echo>(Echo::Request{});
        co_await udp_stub->async_call<Echo>(Echo::Request{});
    }
}

net::awaitable<void> co_main(int argc, char *argv[])
{
    methods.implement<DriverUpdate>(driver_update);

    co_await connect(argc, argv);
    co_await keepalive();
}

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::trace);
    net::io_context io_context;
    net::co_spawn(io_context, co_main(argc, argv), [](std::exception_ptr exception_ptr) {
        try
        {

            if (exception_ptr)
            {
                std::rethrow_exception(exception_ptr);
            }
        }
        catch (const std::exception &ex)
        {
            SPDLOG_ERROR("co_main exception: {}", ex.what());
        }
    });

    io_context.run();
}
