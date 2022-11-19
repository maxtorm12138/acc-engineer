#ifndef ACC_ENGINEER_APP_H
#define ACC_ENGINEER_APP_H

// std
#include <thread>

// qt
#include <QObject>
#include <QApplication>

// boost
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

// rpc
#include <rpc/stub.h>
#include <rpc/method.h>
#include "proto/service.pb.h"

// logic
#include "logic/logic.h"

// ui
#include "ui/connect.h"
#include "ui/driver_input.h"
#include "ui/telemetry.h"
#include "ui/strategy.h"

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
    void shared_memory_main();

private slots:
    void quit();
    void handle_connection_success(std::shared_ptr<rpc::tcp_stub> tcp_stub, std::shared_ptr<rpc::udp_stub> udp_stub);
    
private:
    net::io_context io_context_;
    std::optional<net::executor_work_guard<net::io_context::executor_type>> guard_;
    rpc::methods methods_;

    std::shared_ptr<rpc::tcp_stub> tcp_stub_;
    std::shared_ptr<rpc::udp_stub> udp_stub_;


    logic::connection *connection_logic_;

    std::jthread net_thread_;
    std::jthread shared_memory_thread_;

    QApplication *application_;
    ui::connect *connect_;
    ui::driver_input *driver_input_;
    ui::telemetry *telemetry_;
    ui::strategy *strategy_;
};
} // namespace acc_engineer
#endif
