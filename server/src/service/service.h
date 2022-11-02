#ifndef ACC_ENGINEER_SERVER_SERVICE_SERVICE_H
#define ACC_ENGINEER_SERVER_SERVICE_SERVICE_H

// std
#include <unordered_map>

// boost
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/key.hpp>

// module
#include <rpc/stub.h>
#include <rpc/method.h>
#include <rpc/batch_task.h>

#include "config.h"
#include "session.h"

// protocol
#include "proto/service.pb.h"

namespace acc_engineer {
namespace net = boost::asio;
namespace sys = boost::system;
namespace mi = boost::multi_index;

class service
{
public:
    explicit service(config cfg);

    net::awaitable<void> async_run();

    void cancel();

private:
    net::awaitable<sys::error_code> timer_reset(uint64_t command_id, const rpc::context &context, google::protobuf::Message &);

    net::awaitable<Echo::Response> echo(const rpc::context &context, const Echo::Request &request);

    net::awaitable<AuthenticationTCP::Response> authentication_tcp(const rpc::context &context, const AuthenticationTCP::Request &request);

    net::awaitable<AuthenticationUDP::Response> authentication_udp(const rpc::context &context, const AuthenticationUDP::Request &request);

private:
    net::awaitable<void> tcp_run();

    net::awaitable<void> udp_run();

    net::awaitable<void> new_tcp_connection(net::ip::tcp::socket socket);

    net::awaitable<void> new_udp_connection(net::ip::udp::socket &acceptor, net::ip::udp::endpoint remote, std::vector<uint8_t> initial);

    template<typename Message>
    net::awaitable<void> post(const rpc::request_t<Message> &request);

    template<typename Message>
    net::awaitable<void> broadcast(const rpc::request_t<Message> &request);

    config config_;
    bool running_{false};
    rpc::methods methods_;
    std::shared_ptr<rpc::batch_task<rpc::wait_only>> runner_;
    udp_session_manager udp_session_manager_;
    tcp_session_manager tcp_session_manager_;

    // clang-format off
    boost::multi_index_container<
        driver,
        mi::indexed_by<
            mi::hashed_unique< mi::tag<by_driver_id>, mi::key<&driver::id>>,
            mi::hashed_unique< mi::tag<by_driver_name>, mi::key<&driver::name>>
        >
    > drivers_;
    // clang-format on

private:
    static std::atomic<uint64_t> driver_id_max_;
};

template<typename Message>
net::awaitable<void> service::post(const rpc::request_t<Message> &request)
{
    auto executor = co_await net::this_coro::executor;
    auto runner = rpc::batch_task<rpc::response_t<Message>>::create();

    for (auto &session : tcp_session_manager_.authened())
    {
        if (auto stub = session.stub.lock(); stub != nullptr)
        {
            co_await runner->add(stub->async_call<Message>(request));
        }
    }

    auto [order, exceptions, results] = co_await runner->async_wait(net::experimental::wait_for_all());
}

template<typename Message>
net::awaitable<void> service::broadcast(const rpc::request_t<Message> &request)
{
    auto executor = co_await net::this_coro::executor;
    auto runner = rpc::batch_task<rpc::response_t<Message>>::create();
    for (auto &session : udp_session_manager_.authened())
    {
        if (auto stub = session.stub.lock(); stub != nullptr)
        {
            co_await runner->add(stub->async_call<Message>(request));
        }
    }
    auto [order, exceptions, results] = co_await runner->async_wait(net::experimental::wait_for_all());
}

} // namespace acc_engineer

#endif // ACC_ENGINEER_SERVER_SERVICE_H
