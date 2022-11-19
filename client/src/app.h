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

// ui
#include "ui/connect.h"
#include "ui/driver_input.h"
#include "ui/telemetry.h"
#include "ui/strategy.h"

namespace acc_engineer {
namespace net = boost::asio;

class app final : public QApplication
{
    Q_OBJECT

public:
    explicit app(int argc, char *argv[]);

private:
    void net_main();

    void shared_memory_main();

private slots:
    void quit();

private:
    std::atomic<bool> running_;

    net::io_context io_context_;
    std::optional<net::executor_work_guard<net::io_context::executor_type>> guard_;

    rpc::methods methods_;

    std::jthread net_thread_;
    std::jthread shared_memory_thread_;

    ui::connect *connect_;
    ui::driver_input *driver_input_;
    ui::telemetry *telemetry_;
    ui::strategy *strategy_;
};

} // namespace acc_engineer
#endif
