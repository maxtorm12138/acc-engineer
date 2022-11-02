#include "app.h"

// spdlog
#include <spdlog/spdlog.h>

// boost
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>

namespace acc_engineer {
app::app(QApplication *application)
    : io_context_(1)
    , connection_logic_(new logic::connection(io_context_, methods_))
    , application_(application)
    , connect_(new ui::connect(connection_logic_))
    , driver_input_(new ui::driver_input)
    , telemetry_(new ui::telemetry)
{}

void app::start()
{
    connect(application_, &QGuiApplication::lastWindowClosed, this, &app::quit);

    net_thread_ = std::jthread([this]() { net_main(); });
    shared_memory_thread_ = std::jthread([this]() { shared_memory_main(); });

    connect(connection_logic_, &logic::connection::connection_success, this, &app::handle_connection_success, Qt::QueuedConnection);
    connect_->show();
}

void app::net_main()
{
    SPDLOG_TRACE("net_main started");
    guard_.emplace(make_work_guard(io_context_));
    io_context_.run();
    SPDLOG_TRACE("net_main ended");
}

void app::shared_memory_main()
{
    SPDLOG_TRACE("shared_memory_main started");
    SPDLOG_TRACE("shared_memory_main ended");
}

void app::quit()
{
    SPDLOG_DEBUG("application quit");
    guard_->reset();
    application_->quit();
}

void app::handle_connection_success(std::shared_ptr<rpc::tcp_stub> tcp_stub, std::shared_ptr<rpc::udp_stub> udp_stub)
{
    tcp_stub_ = tcp_stub;
    udp_stub_ = udp_stub;

    driver_input_->show();

    auto geometry = driver_input_->geometry();
    geometry.setX(geometry.x() + 50);
    geometry.setY(geometry.y() + 50);

    telemetry_->setGeometry(geometry);
    telemetry_->show();

    connect_->close();
}

} // namespace acc_engineer