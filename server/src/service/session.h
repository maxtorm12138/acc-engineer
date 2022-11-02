#ifndef ACC_ENGINEER_SERVER_SERVICE_SESSIONS_H
#define ACC_ENGINEER_SERVER_SERVICE_SESSIONS_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/key.hpp>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <rpc/stub.h>

namespace boost {
template<>
struct hash<boost::asio::ip::udp::endpoint>
{
    constexpr size_t operator()(const boost::asio::ip::udp::endpoint &ep) const noexcept
    {
        return std::hash<boost::asio::ip::udp::endpoint>{}(ep);
    }
};
} // namespace boost

namespace acc_engineer {

namespace net = boost::asio;
namespace mi = boost::multi_index;

struct by_stub_id
{};

struct by_endpoint
{};

struct by_driver_id
{};

struct by_driver_name
{};

struct udp_session
{
    // indexes
    uint64_t id;
    uint64_t driver_id;
    std::string driver_name;
    net::ip::udp::endpoint endpoint;

    // connection objects
    std::weak_ptr<rpc::channel_stub> stub;
    std::weak_ptr<net::steady_timer> timer;

    // clang-format off
    using authened_map_type = boost::multi_index_container<
        udp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&udp_session::id>>,
            mi::hashed_unique<mi::tag<by_driver_id>, mi::key<&udp_session::driver_id>>,
            mi::hashed_unique<mi::tag<by_endpoint>, mi::key<&udp_session::endpoint>>,
            mi::hashed_unique<mi::tag<by_driver_name>, mi::key<&udp_session::driver_name>>
        >
    >;

    using staged_map_type = boost::multi_index_container<
        udp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&udp_session::id>>,
            mi::hashed_unique<mi::tag<by_endpoint>, mi::key<&udp_session::endpoint>>
        >
    >;

    // clang-format on
    using stub_type = rpc::channel_stub;
    using timer_type = net::steady_timer;
};

struct tcp_session
{
    // indexes
    uint64_t id;
    uint64_t driver_id;
    std::string driver_name;

    // connection objects
    std::weak_ptr<rpc::tcp_stub> stub;
    std::weak_ptr<net::steady_timer> timer;

    // clang-format off
    using authened_map_type = boost::multi_index_container<
        tcp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&tcp_session::id>>,
            mi::hashed_unique<mi::tag<by_driver_id>, mi::key<&tcp_session::driver_id>>,
            mi::hashed_unique<mi::tag<by_driver_name>, mi::key<&tcp_session::driver_name>>
        >
    >;

    using staged_map_type = boost::multi_index_container<
        tcp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&tcp_session::id>>
        >
    >;
    // clang-format on
    using stub_type = rpc::tcp_stub;
    using timer_type = net::steady_timer;
};

struct driver
{
    std::string ticket;
    uint64_t id;
    std::string name;
};

template<typename Session>
class manager
{
public:
    void add(Session session)
    {
        staged_sessions_.emplace(std::move(session));
    }

    template<typename ByTag, typename KeyType>
    void remove(KeyType &key)
    {
        if (!staged_sessions_.template get<ByTag>().extract(key).empty()) {}
        else if (!authened_sessions_.template get<ByTag>().extract(key).empty()) {}
        else {}
    }

    template<typename ByTag, typename KeyType>
    std::shared_ptr<typename Session::timer_type> timer(KeyType &key)
    {
        auto &view_staged = staged_sessions_.template get<ByTag>();
        auto &view_authened = authened_sessions_.template get<ByTag>();

        std::weak_ptr<typename Session::timer_type> weak_timer;
        if (auto it_session = view_staged.find(key); it_session != view_staged.end())
        {
            weak_timer = it_session->timer;
        }
        else if (auto it_session = view_authened.find(key); it_session != view_authened.end())
        {
            weak_timer = it_session->timer;
        }

        return weak_timer.lock();
    };

    template<typename ByTag, typename KeyType>
    std::shared_ptr<typename Session::stub_type> stub(const KeyType &key)
    {
        auto &view_staged = staged_sessions_.template get<ByTag>();
        auto &view_authened = authened_sessions_.template get<ByTag>();

        std::weak_ptr<typename Session::stub_type> weak_stub;
        if (auto it_session = view_staged.find(key); it_session != view_staged.end())
        {
            weak_stub = it_session->stub;
        }
        else if (auto it_session = view_authened.find(key); it_session != view_authened.end())
        {
            weak_stub = it_session->stub;
        }

        return weak_stub.lock();
    }

    template<>
    std::shared_ptr<typename Session::stub_type> stub<by_driver_id, uint64_t>(const uint64_t &key)
    {
        auto &view_authened = authened_sessions_.template get<by_driver_id>();

        std::weak_ptr<typename Session::stub_type> weak_stub;
        if (auto it_session = view_authened.find(key); it_session != view_authened.end())
        {
            weak_stub = it_session->stub;
        }

        return weak_stub.lock();
    }

    template<>
    std::shared_ptr<typename Session::stub_type> stub<by_driver_name, std::string>(const std::string &key)
    {
        auto &view_authened = authened_sessions_.template get<by_driver_name>();

        std::weak_ptr<typename Session::stub_type> weak_stub;
        if (auto it_session = view_authened.find(key); it_session != view_authened.end())
        {
            weak_stub = it_session->stub;
        }

        return weak_stub.lock();
    }

    void promote(uint64_t stub_id, uint64_t driver_id, const std::string &driver_name)
    {
        auto node = staged_sessions_.template get<by_stub_id>().extract(stub_id);
        if (node.empty())
        {
            return;
        }

        auto session = node.value();
        session.driver_id = driver_id;
        session.driver_name = driver_name;

        authened_sessions_.emplace(std::move(session));
    };

    auto &staged()
    {
        return staged_sessions_;
    }

    auto &authened()
    {
        return authened_sessions_;
    }

private:
    typename Session::staged_map_type staged_sessions_;
    typename Session::authened_map_type authened_sessions_;
};

using tcp_session_manager = manager<tcp_session>;
using udp_session_manager = manager<udp_session>;

} // namespace acc_engineer

#endif