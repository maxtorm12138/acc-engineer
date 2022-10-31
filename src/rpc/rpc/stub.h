#ifndef ACC_ENGINEER_SERVER_RPC_STUB_H
#define ACC_ENGINEER_SERVER_RPC_STUB_H

#include <rpc/detail/stub.h>
#include <rpc/detail/packet_handler.h>

namespace acc_engineer::rpc {

using detail::channel_packet_handler;
using detail::tcp_packet_handler;
using detail::udp_packet_handler;

using tcp_stub = detail::stub<tcp_packet_handler>;
using udp_stub = detail::stub<udp_packet_handler>;
using channel_stub = detail::stub<channel_packet_handler>;

} // namespace acc_engineer::rpc

#endif // ACC_ENGINEER_SERVER_RPC_STUB_H
