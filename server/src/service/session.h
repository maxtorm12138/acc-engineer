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

struct udp_session
{
    // indexes
    uint64_t id;
    uint64_t driver_id;
    std::string driver_name;
    net::ip::udp::endpoint endpoint;

    // connection objects
    std::weak_ptr<rpc::channel_stub> stub;
    std::weak_ptr<net::steady_timer> watcher;
};

struct tcp_session
{
    // indexes
    uint64_t id;
    uint64_t driver_id;
    std::string driver_name;

    // connection objects
    std::weak_ptr<rpc::tcp_stub> stub;
    std::weak_ptr<net::steady_timer> watcher;
};

struct driver
{
    std::string ticket;
    uint64_t id;
    std::string name;
};

struct by_stub_id
{};

struct by_endpoint
{};

struct by_driver_id
{};

struct by_driver_name
{};

class udp_session_manager
{
public:
    void add_session(udp_session session)
    {
        staged_udp_sessions_.emplace(session);
    }

    void remove_session(uint64_t stub_id)
    {
        if (staged_udp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            staged_udp_sessions_.get<by_stub_id>().erase(stub_id);
            SPDLOG_DEBUG("udp_session_manager remove_session  stub_id: {} from staged success", stub_id);
        }
        else if (authened_udp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            authened_udp_sessions_.get<by_stub_id>().erase(stub_id);
            SPDLOG_DEBUG("udp_session_manager remove_session  stub_id: {} from authened success", stub_id);
        }
        else
        {
            SPDLOG_CRITICAL("udp_session_manager remove_session stub_id not found in sessions");
            std::abort();
        }
    }

    void reset_timer(uint64_t stub_id, std::chrono::steady_clock::duration duration)
    {
        auto &view0 = staged_udp_sessions_.get<by_stub_id>();
        auto &view1 = authened_udp_sessions_.get<by_stub_id>();

        if (auto it_session = view0.find(stub_id); it_session != view0.end())
        {
            if (auto timer = it_session->watcher.lock(); timer != nullptr)
            {
                SPDLOG_DEBUG("udp_session_manager timer_reset staged {}", stub_id);
                timer->expires_after(duration);
            }
        }
        else if (auto it_session = view1.find(stub_id); it_session != view1.end())
        {
            if (auto timer = it_session->watcher.lock(); timer != nullptr)
            {
                SPDLOG_DEBUG("udp_session_manager timer_reset authened {}", stub_id);
                timer->expires_after(duration);
            }
        }
        else
        {
            SPDLOG_CRITICAL("udp_session_manager reset_timer stub_id not found in sessions");
            std::abort();
        }
    }

    void mark_authened(uint64_t stub_id, uint64_t driver_id, const std::string &driver_name)
    {
        if (staged_udp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            auto node = staged_udp_sessions_.get<by_stub_id>().extract(stub_id);
            node.value().driver_id = driver_id;
            node.value().driver_name = driver_name;
            authened_udp_sessions_.emplace(node.value());
            SPDLOG_DEBUG("udp_session_manager mark_authened stub_id: {} driver_id: {} driver_name: {} success", stub_id, driver_id, driver_name);
        }
        else if (authened_udp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            SPDLOG_DEBUG("udp_session_manager mark_authened stub_id: {} driver_id: {} driver_name: {} already authened", stub_id, driver_id, driver_name);
        }
        else
        {
            SPDLOG_CRITICAL("udp_session_manager mark_authened stub_id not found in sessions");
            std::abort();
        }
    }

    auto &authened()
    {
        return authened_udp_sessions_;
    }

private:
    // clang-format off
    boost::multi_index_container<
        udp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&udp_session::id>>, mi::hashed_unique<mi::tag<by_driver_id>, mi::key<&udp_session::driver_id>>,
            mi::hashed_unique<mi::tag<by_endpoint>, mi::key<&udp_session::endpoint>>, mi::hashed_unique<mi::tag<by_driver_name>, mi::key<&udp_session::driver_name>>
        >
    > authened_udp_sessions_;

    boost::multi_index_container<
        udp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&udp_session::id>>,
            mi::hashed_unique<mi::tag<by_endpoint>, mi::key<&udp_session::endpoint>>
        >
    > staged_udp_sessions_;
    // clang-format on
};

class tcp_session_manager
{
public:
    void add_session(tcp_session session)
    {
        staged_tcp_sessions_.emplace(session);
    }

    void remove_session(uint64_t stub_id)
    {
        if (staged_tcp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            staged_tcp_sessions_.get<by_stub_id>().erase(stub_id);
            SPDLOG_DEBUG("udp_session_manager remove_session  stub_id: {} from staged success", stub_id);
        }
        else if (authened_tcp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            authened_tcp_sessions_.get<by_stub_id>().erase(stub_id);
            SPDLOG_DEBUG("udp_session_manager remove_session  stub_id: {} from authened success", stub_id);
        }
        else
        {
            SPDLOG_CRITICAL("udp_session_manager remove_session stub_id not found in sessions");
            std::abort();
        }
    }

    void reset_timer(uint64_t stub_id, std::chrono::steady_clock::duration duration)
    {
        auto &view0 = staged_tcp_sessions_.get<by_stub_id>();
        auto &view1 = authened_tcp_sessions_.get<by_stub_id>();

        if (auto it_session = view0.find(stub_id); it_session != view0.end())
        {
            if (auto timer = it_session->watcher.lock(); timer != nullptr)
            {
                SPDLOG_DEBUG("udp_session_manager timer_reset staged {}", stub_id);
                timer->expires_after(duration);
            }
        }
        else if (auto it_session = view1.find(stub_id); it_session != view1.end())
        {
            if (auto timer = it_session->watcher.lock(); timer != nullptr)
            {
                SPDLOG_DEBUG("udp_session_manager timer_reset authened {}", stub_id);
                timer->expires_after(duration);
            }
        }
        else
        {
            SPDLOG_CRITICAL("udp_session_manager reset_timer stub_id not found in sessions");
            std::abort();
        }
    }

    void mark_authened(uint64_t stub_id, uint64_t driver_id, const std::string &driver_name)
    {
        if (staged_tcp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            auto node = staged_tcp_sessions_.get<by_stub_id>().extract(stub_id);
            node.value().driver_id = driver_id;
            node.value().driver_name = driver_name;
            authened_tcp_sessions_.emplace(node.value());
            SPDLOG_DEBUG("udp_session_manager mark_authened stub_id: {} driver_id: {} driver_name: {} success", stub_id, driver_id, driver_name);
        }
        else if (authened_tcp_sessions_.get<by_stub_id>().contains(stub_id))
        {
            SPDLOG_DEBUG("udp_session_manager mark_authened stub_id: {} driver_id: {} driver_name: {} already authened", stub_id, driver_id, driver_name);
        }
        else
        {
            SPDLOG_CRITICAL("udp_session_manager mark_authened stub_id not found in sessions");
            std::abort();
        }
    }

    auto &authened()
    {
        return authened_tcp_sessions_;
    }

private:
    // clang-format off
    boost::multi_index_container<
        tcp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&tcp_session::id>>,
            mi::hashed_unique<mi::tag<by_driver_id>, mi::key<&tcp_session::driver_id>>,
            mi::hashed_unique<mi::tag<by_driver_name>, mi::key<&tcp_session::driver_name>>
        >
    > authened_tcp_sessions_;

    boost::multi_index_container<
        tcp_session,
        mi::indexed_by<
            mi::hashed_unique<mi::tag<by_stub_id>, mi::key<&tcp_session::id>>
        >
    > staged_tcp_sessions_;
    // clang-format on
};

} // namespace acc_engineer

#endif