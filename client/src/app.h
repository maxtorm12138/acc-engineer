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

// strategy setter
#include <strategy_setter/strategy_setter.h>

namespace acc_engineer {
namespace net = boost::asio;

class app final : public QApplication
{
    Q_OBJECT

public:
    explicit app(int argc, char *argv[]);

    ~app() override;

private:
    void net_main();

    void telemetry_collect_main();

private:
    std::atomic<bool> running_;

    net::io_context io_context_;
    net::executor_work_guard<net::io_context::executor_type> guard_;

    strategy_setter::strategy_setter strategy_setter_;

    rpc::methods methods_;

    std::jthread net_thread_;
    std::jthread telemetry_collect_thread_;

    ui::connect *connect_;
    ui::driver_input *driver_input_;
    ui::telemetry *telemetry_;
    ui::strategy *strategy_;
};

} // namespace acc_engineer
#endif
