#ifndef ACC_ENGINEER_APP_H
#define ACC_ENGINEER_APP_H

// std
#include <thread>

// qt
#include <QObject>
#include <QApplication>

// boost
#include <boost/asio/io_context.hpp>

// rpc
#include <rpc/stub.h>

// module
#include "ui/connect.h"
#include "ui/main_window.h"
#include "ui/driver_input.h"

namespace acc_engineer {
namespace net = boost::asio;

class app final : public QObject
{
    Q_OBJECT
public:
    app(QApplication *application);

    void start();

private:
    void net_main();

private slots:
    void start_connection(QString address, QString driver, QString password, uint16_t port);
    void finish_connection();

private:
    net::io_context io_context_;

    std::jthread net_thread_;
    std::jthread shared_memory_thread_;

    std::shared_ptr<rpc::tcp_stub> tcp_stub_;
    std::shared_ptr<rpc::udp_stub> udp_stub_;

    QApplication *application_;
    ui::connect *connect_;
    ui::main_window *main_window_;
    ui::driver_input *driver_input_;
};
} // namespace acc_engineer
#endif
