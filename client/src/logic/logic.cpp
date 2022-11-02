#include "logic.h"

// boost
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/connect.hpp>

// rpc
#include <rpc/error_code.h>

// protocol
#include "proto/service.pb.h"

namespace acc_engineer::logic {

connection::connection(net::io_context &io_context, const rpc::methods &methods)
    : io_context_(io_context)
    , methods_(methods)
{}

void connection::start_connection(QString address, QString driver, QString password, uint16_t port)
{
    net::co_spawn(io_context_, do_connection(address, driver, password, port), net::detached);
}

net::awaitable<void> connection::do_connection(QString address, QString driver, QString password, uint16_t port)
{
    auto executor = co_await net::this_coro::executor;
    sys::error_code error_code;

    try
    {
        net::ip::tcp::resolver resolver(executor);
        auto endpoints = co_await resolver.async_resolve(address.toStdString(), std::to_string(port), net::use_awaitable);

        net::ip::tcp::socket tcp_socket(executor);
        auto connected = co_await net::async_connect(tcp_socket, endpoints.begin(), endpoints.end(), net::use_awaitable);

        auto tcp_stub = rpc::tcp_stub::create(std::move(tcp_socket), methods_);

        AuthenticationTCP::Request tcp_authen_request;
        tcp_authen_request.set_driver_name(driver.toStdString());
        tcp_authen_request.set_password(password.toStdString());

        auto tcp_authen_response = co_await tcp_stub->async_call<AuthenticationTCP>(tcp_authen_request);
        if (tcp_authen_response.error_code() != 0)
        {
            throw std::logic_error(fmt::format("{} {}", tcp_authen_response.error_code(), tcp_authen_response.error_message()));
        }
    }
    catch (sys::system_error &ex)
    {
        emit this->connection_failure(ex.code().value(), QString::fromLocal8Bit(ex.what()));
    }
    catch (std::logic_error &ex)
    {}
}
} // namespace acc_engineer::logic
