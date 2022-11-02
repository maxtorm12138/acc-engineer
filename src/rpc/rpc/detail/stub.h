#ifndef ACC_ENGINEER_SERVER_RPC_DETAIL_STUB_H
#define ACC_ENGINEER_SERVER_RPC_DETAIL_STUB_H

#include <span>
#include <bitset>
#include <numeric>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>

#include <boost/scope_exit.hpp>

#include "method.h"
#include "await_error_code.h"
#include "batch_task.h"

#include "proto/rpc.pb.h"

namespace acc_engineer::rpc::detail {
namespace net = boost::asio;
namespace sys = boost::system;

enum class stub_status
{
    idle,
    running,
    stopping,
    stopped
};

enum flags
{
    flag_is_request = 0,
    flag_no_reply = 1,
};

constexpr size_t MAX_IO_CHANNEL_BUFFER_SIZE = 100;

inline uint64_t generate_trace_id()
{
    static std::atomic<uint64_t> stub_id_max_{1};
    return stub_id_max_++;
}

inline uint64_t generate_stub_id()
{
    static std::atomic<uint64_t> trace_id_max_{1};
    return trace_id_max_++;
}

template<typename PacketHandler>
class stub : public std::enable_shared_from_this<stub<PacketHandler>>, public boost::noncopyable
{
public:
    using packet_handler_type = PacketHandler;
    using input_channel_type = net::experimental::channel<void(sys::error_code, std::vector<uint8_t>)>;
    using output_channel_type = net::experimental::channel<void(sys::error_code, std::vector<uint8_t>)>;
    using calling_channel_type = net::experimental::channel<void(sys::error_code, std::vector<uint8_t>)>;

    static std::shared_ptr<stub<PacketHandler>> create(packet_handler_type packet_handler, const methods &methods = methods::empty());

    ~stub();

    net::awaitable<void> async_run();

    void cancel();

    template<is_method_message MethodMessage>
    net::awaitable<response_t<MethodMessage>> async_call(const request_t<MethodMessage> &request);

    uint64_t id() const noexcept;

    packet_handler_type &packet_handler() noexcept;

    bool running() noexcept;

private:
    explicit stub(packet_handler_type packet_handler, const methods &methods = methods::empty());

    net::awaitable<void> input_loop();

    net::awaitable<void> output_loop();

    net::awaitable<void> worker(std::vector<uint8_t> buffer);

    std::tuple<uint64_t, std::bitset<64>, rpc::Cookie, std::span<uint8_t>> unpack(const std::vector<uint8_t> &receive_buffer);

    std::vector<uint8_t> pack(uint64_t command_id, std::bitset<64> flags, rpc::Cookie cookie, std::vector<uint8_t> payload);

    std::vector<uint8_t> pack(uint64_t command_id, std::bitset<64> flags, rpc::Cookie cookie, const google::protobuf::Message &message);

private:
    packet_handler_type packet_handler_;
    input_channel_type input_channel_;
    output_channel_type output_channel_;
    std::unordered_map<uint64_t, std::optional<calling_channel_type>> calling_channel_;
    stub_status status_;
    const methods &methods_;
    const uint64_t id_;
    std::shared_ptr<batch_task<wait_only>> runner_;
};

template<typename PacketHandler>
std::shared_ptr<stub<PacketHandler>> stub<PacketHandler>::create(typename stub<PacketHandler>::packet_handler_type packet_handler, const methods &methods)
{
    return std::shared_ptr<stub<PacketHandler>>(new stub<PacketHandler>(std::move(packet_handler), methods));
}

template<typename PacketHandler>
stub<PacketHandler>::stub(packet_handler_type packet_handler, const methods &methods)
    : packet_handler_(std::move(packet_handler))
    , input_channel_(packet_handler.get_executor(), MAX_IO_CHANNEL_BUFFER_SIZE)
    , output_channel_(packet_handler.get_executor(), MAX_IO_CHANNEL_BUFFER_SIZE)
    , status_(stub_status::idle)
    , methods_(methods)
    , id_(generate_stub_id())
{}

template<typename PacketHandler>
stub<PacketHandler>::~stub()
{
    SPDLOG_DEBUG("~stub {}", id_);
}

template<typename PacketHandler>
net::awaitable<void> stub<PacketHandler>::async_run()
{
    auto executor = co_await net::this_coro::executor;
    runner_ = batch_task<wait_only>::create();

    status_ = stub_status::running;
    BOOST_SCOPE_EXIT_ALL(&)
    {
        status_ = stub_status::stopped;
    };

    co_await runner_->add(input_loop());
    co_await runner_->add(output_loop());

    sys::error_code error_code;
    co_await runner_->async_wait(net::experimental::wait_for_one_error(), error_code);
}

template<typename PacketHandler>
void stub<PacketHandler>::cancel()
{
    if (status_ == stub_status::running)
    {
        SPDLOG_TRACE("stub {} stopping", id_);
        status_ = stub_status::stopping;
        runner_->cancel();
    }
}

template<typename PacketHandler>
template<is_method_message MethodMessage>
net::awaitable<response_t<MethodMessage>> stub<PacketHandler>::async_call(const request_t<MethodMessage> &request)
{
    uint64_t command_id = MethodMessage::descriptor()->options().GetExtension(rpc::cmd_id);

    bool no_reply = MethodMessage::descriptor()->options().GetExtension(rpc::no_reply);
    uint64_t trace_id = generate_trace_id();
    std::bitset<64> flags;
    flags.set(flag_is_request).set(flag_no_reply, no_reply);

    rpc::Cookie cookie;
    cookie.set_trace_id(trace_id);
    cookie.set_error_code(0);

    SPDLOG_TRACE("async_call {} method: {} trace_id: {} request: {}", id_, command_id, trace_id, request.ShortDebugString());

    auto packet = pack(command_id, flags, std::move(cookie), request);

    calling_channel_[trace_id].emplace(co_await net::this_coro::executor);
    BOOST_SCOPE_EXIT_ALL(&)
    {
        calling_channel_.erase(trace_id);
    };

    sys::error_code error_code;
    co_await output_channel_.async_send({}, std::move(packet), await_error_code(error_code));
    SPDLOG_TRACE("async_call {} output_channel: {}", id_, error_code.message());
    if (error_code)
    {
        if (error_code == net::experimental::error::channel_closed)
        {
            throw sys::system_error(system_error::connection_closed);
        }

        if (error_code == net::experimental::error::channel_cancelled)
        {
            throw sys::system_error(system_error::connection_closed);
        }
        throw sys::system_error(system_error::unhandled_system_error);
    }

    if (no_reply)
    {
        co_return response_t<MethodMessage>{};
    }

    auto response_payload = co_await calling_channel_[trace_id]->async_receive(await_error_code(error_code));
    SPDLOG_TRACE("async_call {} calling_channel: {}", id_, error_code.message());
    if (error_code)
    {
        if (error_code == net::experimental::error::channel_closed)
        {
            throw sys::system_error(system_error::connection_closed);
        }

        if (error_code == net::experimental::error::channel_cancelled)
        {
            throw sys::system_error(system_error::connection_closed);
        }

        if (error_code.category() == system_error_category())
        {
            throw sys::system_error(error_code);
        }

        throw sys::system_error(system_error::unhandled_system_error);
    }

    response_t<MethodMessage> response{};
    if (!response.ParseFromArray(response_payload.data(), static_cast<int>(response_payload.size())))
    {
        throw sys::system_error(system_error::proto_parse_fail);
    }

    co_return response;
}

template<typename PacketHandler>
uint64_t stub<PacketHandler>::id() const noexcept
{
    return id_;
}

template<typename PacketHandler>
typename stub<PacketHandler>::packet_handler_type &stub<PacketHandler>::packet_handler() noexcept
{
    return packet_handler_;
}

template<typename PacketHandler>
bool stub<PacketHandler>::running() noexcept
{
    return status_ == stub_status::running;
}

template<typename PacketHandler>
net::awaitable<void> stub<PacketHandler>::input_loop()
{
    std::vector<uint8_t> receive_buffer(PacketHandler::MAX_PACKET_SIZE);
    SPDLOG_TRACE("input_loop {} started", id_);
    while (status_ == stub_status::running)
    {
        sys::error_code error_code;
        error_code = co_await packet_handler_.receive_packet(receive_buffer);
        if (error_code)
        {
            if (error_code == system_error::operation_canceled)
            {
                SPDLOG_INFO("input_loop {} PacketHandler::receive_packet system_error: {}", id_, error_code.message());
            }
            else
            {
                SPDLOG_ERROR("input_loop {} PacketHandler::receive_packet system_error: {}", id_, error_code.message());
            }
            throw sys::system_error(error_code);
        }
        co_await runner_->add(worker(receive_buffer));
    }
}

template<typename PacketHandler>
net::awaitable<void> stub<PacketHandler>::output_loop()
{
    SPDLOG_TRACE("output_loop {} started", id_);
    while (status_ == stub_status::running)
    {
        sys::error_code error_code;
        std::vector<uint8_t> send_buffer = co_await output_channel_.async_receive(await_error_code(error_code));
        if (error_code)
        {
            if (error_code == net::experimental::error::channel_cancelled)
            {
                SPDLOG_TRACE("output_loop {} output_channel_.async_receive system_error: {}", id_, error_code.message());
            }
            else
            {
                SPDLOG_ERROR("output_loop {} output_channel_.async_receive system_error: {}", id_, error_code.message());
            }
            break;
        }

        error_code = co_await packet_handler_.send_packet(std::move(send_buffer));
        if (error_code)
        {
            if (error_code == system_error::operation_canceled)
            {
                SPDLOG_TRACE("output_loop {} PacketHandler::send_packet system_error: {}", id_, error_code.message());
            }
            else
            {
                SPDLOG_ERROR("output_loop {} PacketHandler::send_packet system_error: {}", id_, error_code.message());
            }
            break;
        }
    }
    SPDLOG_TRACE("output_loop {} stopped", id_);
}

template<typename PacketHandler>
net::awaitable<void> stub<PacketHandler>::worker(std::vector<uint8_t> buffer)
{
    auto [command_id, flags, cookie, payload] = unpack(buffer);
    SPDLOG_DEBUG("worker {} unpack command_id: {}, flags: {:#x}, cookie: [{}]", id_, command_id, flags.to_ullong(), cookie.ShortDebugString());

    sys::error_code error_code;

    if (!flags.test(flag_is_request))
    {
        if (!calling_channel_.contains(cookie.trace_id()))
        {
            SPDLOG_INFO("input_loop {} response message {} outdated", id_, cookie.trace_id());
            co_return;
        }
        auto &calling = calling_channel_[cookie.trace_id()];

        co_await calling->async_send(static_cast<system_error>(cookie.error_code()), std::vector(payload.begin(), payload.end()), await_error_code(error_code));
        if (error_code)
        {
            if (error_code == net::experimental::error::channel_cancelled)
            {
                error_code = system_error::operation_canceled;
                SPDLOG_INFO("input_loop {} calling->async_send {} system_error: {}", id_, cookie.trace_id(), error_code.message());
                co_return;
            }

            error_code = system_error::unhandled_system_error;
            SPDLOG_ERROR("input_loop {} calling->async_send {} system_error: {}", id_, cookie.trace_id(), error_code.message());
            throw sys::system_error(error_code);
        }

        co_return;
    }

    if (flags.test(flag_is_request))
    {
        const context context{.stub_id = id_, .packet_handler_type = PacketHandler::type};
        std::vector<uint8_t> response_payload = co_await methods_(command_id, context, payload, error_code);

        if (flags.test(flag_no_reply))
        {
            co_return;
        }

        std::bitset<64> response_flags;
        response_flags.set(flag_is_request, false);

        if (error_code)
        {
            SPDLOG_INFO("worker {} dispatch_request command_id: {} cookie: {} system_error: {}", id_, command_id, cookie.ShortDebugString(), error_code.message());
            cookie.set_error_code(error_code.value());
        }

        std::vector<uint8_t> send_buffer = pack(command_id, response_flags, std::move(cookie), std::move(response_payload));
        co_await output_channel_.async_send({}, std::move(send_buffer), await_error_code(error_code));

        if (error_code)
        {
            if (error_code == net::experimental::error::channel_cancelled)
            {
                error_code = system_error::operation_canceled;
                SPDLOG_INFO(
                    "worker_loop {} output_channel_.async_send command_id: {} cookie: {} system_error: {}", id_, command_id, cookie.ShortDebugString(), error_code.message());
            }
            else
            {
                error_code = system_error::unhandled_system_error;
                SPDLOG_ERROR(
                    "worker_loop {} output_channel_.async_send command_id: {} cookie: {} system_error: {}", id_, command_id, cookie.ShortDebugString(), error_code.message());
            }
            throw sys::system_error(error_code);
        }
    }
}

template<typename PacketHandler>
std::tuple<uint64_t, std::bitset<64>, rpc::Cookie, std::span<uint8_t>> stub<PacketHandler>::unpack(const std::vector<uint8_t> &receive_buffer)
{
    uint64_t command_id;
    uint64_t bit_flags;
    uint64_t cookie_payload_size;
    uint64_t payload_size;

    std::array header = {
        net::buffer(&command_id, sizeof(command_id)),                   /**/
        net::buffer(&bit_flags, sizeof(bit_flags)),                     /**/
        net::buffer(&cookie_payload_size, sizeof(cookie_payload_size)), /**/
        net::buffer(&payload_size, sizeof(payload_size)),               /**/
    };

    auto receive_buffer_view = net::buffer(receive_buffer);

    auto header_size = net::buffer_copy(header, receive_buffer_view);
    receive_buffer_view += header_size;

    rpc::Cookie cookie{};
    if (!cookie.ParseFromArray(receive_buffer_view.data(), static_cast<int>(cookie_payload_size)))
    {
        throw sys::system_error(system_error::proto_parse_fail);
    }

    receive_buffer_view += cookie_payload_size;

    std::span payload(static_cast<uint8_t *>(const_cast<void *>(receive_buffer_view.data())), payload_size);
    receive_buffer_view += payload_size;
    if (receive_buffer_view.size() != 0)
    {
        throw sys::system_error(system_error::data_corrupted);
    }

    return {command_id, std::bitset<64>(bit_flags), std::move(cookie), payload};
}

template<typename PacketHandler>
std::vector<uint8_t> stub<PacketHandler>::pack(uint64_t command_id, std::bitset<64> flags, rpc::Cookie cookie, std::vector<uint8_t> payload)
{
    std::vector<uint8_t> cookie_payload(cookie.ByteSizeLong());
    if (!cookie.SerializeToArray(cookie_payload.data(), static_cast<int>(cookie_payload.size())))
    {
        throw sys::system_error(system_error::proto_serialize_fail);
    }

    uint64_t cookie_payload_size = cookie_payload.size();
    uint64_t payload_size = payload.size();
    uint64_t bit_flags = flags.to_ullong();

    std::array buffer_sequence = {
        net::buffer(&command_id, sizeof(command_id)),                   /**/
        net::buffer(&bit_flags, sizeof(bit_flags)),                     /**/
        net::buffer(&cookie_payload_size, sizeof(cookie_payload_size)), /**/
        net::buffer(&payload_size, sizeof(payload_size)),               /**/
        net::buffer(cookie_payload), net::buffer(payload)               /**/
    };

    size_t total = std::accumulate(buffer_sequence.begin(), buffer_sequence.end(), 0ULL, [](auto &&current, auto &&buffer) { return current + buffer.size(); });

    std::vector<uint8_t> packed(total);
    net::buffer_copy(net::buffer(packed), buffer_sequence, total);
    return packed;
}

template<typename PacketHandler>
std::vector<uint8_t> stub<PacketHandler>::pack(uint64_t command_id, std::bitset<64> flags, rpc::Cookie cookie, const google::protobuf::Message &message)
{
    std::vector<uint8_t> message_payload(message.ByteSizeLong());
    if (!message.SerializeToArray(message_payload.data(), static_cast<int>(message_payload.size())))
    {
        throw sys::system_error(system_error::proto_serialize_fail);
    }

    return pack(command_id, flags, std::move(cookie), std::move(message_payload));
}

} // namespace acc_engineer::rpc::detail

#endif