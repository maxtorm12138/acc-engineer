#ifndef ACC_ENGINEER_LOGIC_H
#define ACC_ENGINEER_LOGIC_H

// qt
#include <QObject>

// boost
#include <boost/asio/io_context.hpp>

// rpc
#include <rpc/stub.h>
#include <rpc/method.h>

namespace acc_engineer::logic {

namespace net = boost::asio;
namespace sys = boost::system;

class connection : public QObject
{
    Q_OBJECT
public:
    explicit connection(net::io_context &io_context, const rpc::methods &methods);

signals:
    void connection_success(std::shared_ptr<rpc::tcp_stub> tcp_stub, std::shared_ptr<rpc::udp_stub> udp_stub);
    void connection_failure(uint64_t code, QString message);

public slots:
    void start_connection(QString address, QString driver, QString password, uint16_t port);

private:
    net::awaitable<void> do_connection(QString address, QString driver, QString password, uint16_t port);

private:
    net::io_context &io_context_;
    const rpc::methods &methods_;
};

} // namespace acc_engineer::logic

#endif
