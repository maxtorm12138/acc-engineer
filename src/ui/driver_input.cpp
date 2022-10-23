#include "driver_input.h"

#include <spdlog/spdlog.h>

#include "ui_driverinput.h"

namespace acc_engineer::ui {
driver_input::driver_input(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::DriverInput)
{
    ui_->setupUi(this);
}

void driver_input::handle_new_frame(frame frame)
{
    SPDLOG_DEBUG("new frame");
    ui_->GasProgressBar->setValue(int(std::floor(frame.gas * 100)));
    ui_->ThrottleProgressBar->setValue(int(std::floor(frame.throttle * 100)));
}
} // namespace acc_engineer::ui
