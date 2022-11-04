#include "service.h"

#include <spdlog/spdlog.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>

#include "rpc/error_code.h"
#include "rpc/batch_task.h"

#include "proto/service.pb.h"

namespace acc_engineer {

namespace sys = boost::system;

service::service(config cfg)
    : config_(std::move(cfg))
{}

net::awaitable<void> service::async_run()
{
    using namespace std::placeholders;
    running_ = true;
    methods_.add_request_interceptor(std::bind(&service::timer_reset, this, _1, _2, _3));

    methods_.implement<Echo>(std::bind(&service::echo, this, _1, _2));
    methods_.implement<AuthenticationTCP>(std::bind(&service::authentication_tcp, this, _1, _2));
    methods_.implement<AuthenticationUDP>(std::bind(&service::authentication_udp, this, _1, _2));

    auto executor = co_await net::this_coro::executor;
    runner_ = rpc::batch_task<rpc::wait_only>::create();

    co_await runner_->add(udp_run());
    co_await runner_->add(tcp_run());

    sys::error_code error_code;
    co_await runner_->async_wait(net::experimental::wait_for_one(), error_code);
    SPDLOG_TRACE("run stopped");
}

void service::cancel()
{
    running_ = false;
    runner_->cancel();
}

net::awaitable<void> service::tcp_run()
{
    auto executor = co_await net::this_coro::executor;

    auto runner = rpc::batch_task<rpc::wait_only>::create();

    auto acceptor = [&runner, this]() -> net::awaitable<void> {
        auto executor = co_await net::this_coro::executor;
        net::ip::tcp::acceptor acceptor(executor, net::ip::tcp::endpoint{config_.address(), config_.port()});
        SPDLOG_TRACE("tcp_run listening on {}:{}", config_.address().to_string(), config_.port());

        while (running_)
        {
            sys::error_code error_code;
            auto socket = co_await acceptor.async_accept(rpc::await_error_code(error_code));
            if (error_code)
            {
                if (error_code == net::error::operation_aborted)
                {
                    SPDLOG_INFO("tcp_run acceptor.async_accept system_error: {}", error_code.message());
                }
                else
                {
                    SPDLOG_CRITICAL("tcp_run acceptor.async_accept system_error: {}", error_code.message());
                }
                break;
            }

            co_await runner->add(new_tcp_connection(std::move(socket)));
        }
    };

    co_await runner->add(acceptor());

    co_await runner->async_wait(net::experimental::wait_for_all());

    SPDLOG_TRACE("tcp_run stopped");
}

net::awaitable<void> service::udp_run()
{
    auto executor = co_await net::this_coro::executor;
    auto runner = rpc::batch_task<rpc::wait_only>::create();

    auto acceptor = [&runner, this]() -> net::awaitable<void> {
        auto executor = co_await net::this_coro::executor;

        net::ip::udp::endpoint bind_endpoint{config_.address(), config_.port()};
        auto acceptor = std::make_shared<net::ip::udp::socket>(executor);
        acceptor->open(bind_endpoint.protocol());
        acceptor->bind(bind_endpoint);

        SPDLOG_TRACE("udp run listening on {}:{}", config_.address().to_string(), config_.port());

        std::vector<uint8_t> initial(1500);
        while (running_)
        {
            net::ip::udp::endpoint remote;
            sys::error_code error_code;
            size_t size_read = co_await acceptor->async_receive_from(net::buffer(initial), remote, rpc::await_error_code(error_code));

            if (error_code)
            {
                if (error_code == net::error::operation_aborted)
                {
                    SPDLOG_INFO("udp_run acceptor.async_receive_from system_error: {}", error_code.message());
                }
                else
                {
                    SPDLOG_CRITICAL("udp_run acceptor.async_receive_from system_error: {}", error_code.message());
                }
                break;
            }

            if (auto stub = udp_session_manager_.stub<by_endpoint>(remote); stub != nullptr)
            {
                co_await stub->packet_handler().deliver(std::vector<uint8_t>(initial.begin(), initial.begin() + size_read));
            }
            else
            {
                co_await runner->add(new_udp_connection(acceptor, remote, std::vector(initial.begin(), initial.begin() + size_read)));
            }
        }
    };

    co_await runner->add(acceptor());
    co_await runner->async_wait(net::experimental::wait_for_all());

    SPDLOG_TRACE("udp_run stopped");
}

net::awaitable<void> service::new_tcp_connection(net::ip::tcp::socket socket)
{
    uint64_t stub_id = 0;
    auto remote_endpoint = socket.remote_endpoint();
    using namespace std::chrono_literals;

    SPDLOG_TRACE("new_tcp_connection {} tcp connected {}:{}", stub_id, remote_endpoint.address().to_string(), remote_endpoint.port());
    try
    {
        auto tcp_stub = rpc::tcp_stub::create(std::move(socket), methods_);
        stub_id = tcp_stub->id();

        auto timer = std::make_shared<net::steady_timer>(co_await net::this_coro::executor);

        tcp_session_manager_.add({.id = stub_id, .driver_id = 0, .stub = tcp_stub, .timer = timer});
        BOOST_SCOPE_EXIT_ALL(&)
        {
            tcp_session_manager_.remove<by_stub_id>(stub_id);
        };

        auto tcp_timer = [timer, tcp_stub]() -> net::awaitable<void> {
            sys::error_code error_code;
            timer->expires_after(10s);
            do
            {
                co_await timer->async_wait(rpc::await_error_code(error_code));
            } while (tcp_stub->running() && error_code == net::error::operation_aborted);

            tcp_stub->cancel();
        };

        auto tcp_run = [tcp_stub]() -> net::awaitable<void> { co_await tcp_stub->async_run(); };

        auto runner = rpc::batch_task<rpc::wait_only>::create();

        co_await runner->add(tcp_timer());
        co_await runner->add(tcp_run());

        sys::error_code error_code;
        co_await runner->async_wait(net::experimental::wait_for_one(), error_code);
        if (error_code && error_code != rpc::system_error::operation_canceled)
        {
            throw sys::system_error(error_code);
        }

        if (running_)
        {
            DriverUpdate::Request driver_update_request;
            driver_update_request.set_version(driver_status_version_++);
            for (auto &session : tcp_session_manager_.authened())
            {
                if (session.id != stub_id)
                {
                    auto driver = driver_update_request.add_drivers();
                    driver->set_driver_id(session.driver_id);
                    driver->set_driver_name(session.driver_name);
                }
            }

            co_await post<DriverUpdate>(driver_update_request, [stub_id](auto &session) { return session.id != stub_id; });
        }
    }
    catch (sys::system_error &ex)
    {
        SPDLOG_ERROR("{} tcp run exception: {}", stub_id, ex.what());
    }

    SPDLOG_TRACE("{} tcp disconnected {}:{}", stub_id, remote_endpoint.address().to_string(), remote_endpoint.port());
}

net::awaitable<void> service::new_udp_connection(std::shared_ptr<net::ip::udp::socket> acceptor, net::ip::udp::endpoint remote, std::vector<uint8_t> initial)
{
    uint64_t stub_id = 0;
    using namespace std::chrono_literals;

    SPDLOG_TRACE("new_udp_connection udp connected {}:{}", remote.address().to_string(), remote.port());
    try
    {
        auto executor = co_await net::this_coro::executor;
        rpc::channel_packet_handler packet_handler({executor, 100}, {executor, 100});
        auto channel_stub = rpc::channel_stub::create(std::move(packet_handler), methods_);
        stub_id = channel_stub->id();

        auto timer = std::make_shared<net::steady_timer>(co_await net::this_coro::executor);
        udp_session session{.id = stub_id, .endpoint = remote, .stub = channel_stub, .timer = timer};

        udp_session_manager_.add(session);
        BOOST_SCOPE_EXIT_ALL(&)
        {
            udp_session_manager_.remove<by_stub_id>(stub_id);
        };

        auto udp_timer = [timer, channel_stub]() -> net::awaitable<void> {
            SPDLOG_TRACE("new_udp_connection udp_timer started");
            sys::error_code error_code;
            timer->expires_after(10s);
            do
            {
                co_await timer->async_wait(rpc::await_error_code(error_code));
            } while (channel_stub->running() && error_code == net::error::operation_aborted);

            SPDLOG_TRACE("new_udp_connection udp_timer {} expires", channel_stub->id());
            channel_stub->cancel();
        };

        auto udp_run = [channel_stub]() -> net::awaitable<void> { co_await channel_stub->async_run(); };

        auto udp_send = [channel_stub, acceptor, remote]() -> net::awaitable<void> {
            SPDLOG_TRACE("new_udp_connection udp_send started");

            do
            {
                sys::error_code error_code;
                auto send_buffer = co_await channel_stub->packet_handler().method_send_channel.async_receive(rpc::await_error_code(error_code));
                if (error_code)
                {
                    SPDLOG_ERROR("new_udp_connection udp_send method_send_channel.async_receive system_error: {}", error_code.message());
                    break;
                }

                co_await acceptor->async_send_to(net::buffer(send_buffer), remote, rpc::await_error_code(error_code));
                if (error_code)
                {
                    SPDLOG_ERROR("new_udp_connection udp_send acceptor.async_send_to system_error: {}", error_code.message());
                    break;
                }
            } while (channel_stub->running());
            SPDLOG_TRACE("new_udp_connection udp_send stopped");
        };

        auto error_code = co_await channel_stub->packet_handler().deliver(std::move(initial));
        if (error_code && error_code != rpc::system_error::operation_canceled)
        {
            throw sys::system_error(error_code);
        }

        auto runner = rpc::batch_task<rpc::wait_only>::create();
        co_await runner->add(udp_run());
        co_await runner->add(udp_timer());
        co_await runner->add(udp_send());

        co_await runner->async_wait(net::experimental::wait_for_one(), error_code);
        if (error_code && error_code != rpc::system_error::operation_canceled)
        {
            throw sys::system_error(error_code);
        }
    }
    catch (sys::system_error &ex)
    {
        SPDLOG_ERROR("{} udp run exception: {}", stub_id, ex.what());
    }

    SPDLOG_TRACE("new_udp_connection udp disconnected {}:{}", remote.address().to_string(), remote.port());
}

net::awaitable<sys::error_code> service::timer_reset(uint64_t command_id, const rpc::context &context, google::protobuf::Message &)
{
    using namespace std::chrono_literals;

    if (context.packet_handler_type == rpc::tcp_packet_handler::type)
    {
        auto timer = tcp_session_manager_.timer<by_stub_id>(context.stub_id);
        if (timer != nullptr)
        {
            timer->expires_after(1min);
        }
    }
    else if (context.packet_handler_type == rpc::channel_packet_handler::type)
    {
        auto timer = udp_session_manager_.timer<by_stub_id>(context.stub_id);
        if (timer != nullptr)
        {
            timer->expires_after(30s);
        }
    }

    co_return rpc::system_error::success;
}

net::awaitable<Echo::Response> service::echo(const rpc::context &context, const Echo::Request &request)
{
    Echo::Response response;
    response.set_message(request.message());
    co_return response;
}

net::awaitable<AuthenticationTCP::Response> service::authentication_tcp(const rpc::context &context, const AuthenticationTCP::Request &request)
{
    static std::atomic<uint64_t> driver_id_max{1};

    if (request.password() != config_.password() || request.driver_name().empty())
    {
        AuthenticationTCP::Response response;
        response.set_error_code(1);
        response.set_error_message("authentication failure");
        co_return response;
    }

    if (tcp_session_manager_.stub<by_driver_name>(request.driver_name()) != nullptr)
    {
        AuthenticationTCP::Response response;
        response.set_error_code(2);
        response.set_error_message("driver already authenticated");
        co_return response;
    }

    uint64_t driver_id = driver_id_max++;
    tcp_session_manager_.promote(context.stub_id, driver_id, request.driver_name());

    DriverUpdate::Request driver_update_request;
    driver_update_request.set_version(driver_status_version_++);
    for (auto &session : tcp_session_manager_.authened())
    {
        auto driver = driver_update_request.add_drivers();
        driver->set_driver_id(session.driver_id);
        driver->set_driver_name(session.driver_name);
    }

    co_await post<DriverUpdate>(driver_update_request);

    AuthenticationTCP::Response response;
    response.set_error_code(0);
    response.set_error_message("success");
    response.set_driver_id(driver_id);
    co_return response;
}

net::awaitable<AuthenticationUDP::Response> service::authentication_udp(const rpc::context &context, const AuthenticationUDP::Request &request)
{

    if (request.password() != config_.password() || request.driver_name().empty() || request.driver_id() == 0)
    {
        AuthenticationUDP::Response response;
        response.set_error_code(1);
        response.set_error_message("authentication failure");
        co_return response;
    }

    if (tcp_session_manager_.stub<by_driver_name>(request.driver_name()) == nullptr)
    {
        AuthenticationUDP::Response response;
        response.set_error_code(1);
        response.set_error_message("authentication failure");
        co_return response;
    }

    udp_session_manager_.promote(context.stub_id, request.driver_id(), request.driver_name());

    AuthenticationUDP::Response response;
    response.set_error_code(0);
    response.set_error_message("success");
    response.set_driver_id(request.driver_id());
    co_return response;
}

net::awaitable<DriverStrategyUpdate::Response> service::driver_strategy_update(const rpc::context &context, const DriverStrategyUpdate::Request &request)
{
    auto stub = tcp_session_manager_.stub<by_driver_id>(request.driver().driver_id());
    if (stub == nullptr) {}

    co_return co_await stub->async_call<DriverStrategyUpdate>(request);
}

std::atomic<uint64_t> service::driver_status_version_;
} // namespace acc_engineer
