#include "app.h"

// spdlog
#include <spdlog/spdlog.h>

// boost
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>

namespace acc_engineer {
app::app(QApplication *application)
    : io_context_(1)
    , application_(application)
    , connect_(new ui::connect)
    , main_window_(new ui::main_window)
    , driver_input_(new ui::driver_input(main_window_))
    , telemetry_(new ui::telemetry(main_window_))
{}

void app::start()
{
    net_thread_ = std::jthread([this]() { net_main(); });

    connect(connect_, &ui::connect::connection, this, &app::start_connection);
    connect_->show();
}

void app::net_main()
{
    SPDLOG_TRACE("net_main started");
    auto guard = make_work_guard(io_context_);
    io_context_.run();
    SPDLOG_TRACE("net_main ended");
}

void app::start_connection(QString address, QString driver, QString password, uint16_t port)
{
    main_window_->show();
    connect_->close();
}

void app::finish_connection()
{
}
} // namespace acc_engineer