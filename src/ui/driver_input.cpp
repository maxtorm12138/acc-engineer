#include "driver_input.h"

#include <spdlog/spdlog.h>

#include "ui_driverinput.h"

#include "gas_chart.h"

namespace acc_engineer::ui {
driver_input::driver_input(gas_chart *gas_chart, QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::DriverInput)
{
    ui_->setupUi(this);
    ui_->GasChart->setChart(gas_chart);
}

void driver_input::handle_new_frame(frame frame)
{
    SPDLOG_DEBUG("new frame");
    ui_->GasProgressBar->setValue(frame.percent_gas);
    ui_->ThrottleProgressBar->setValue(frame.percent_throttle);
    ui_->SpeedLabel->setText(QString("%1 km/h").arg(frame.speed));
    if (frame.gear > 1)
    {
        ui_->GearLabel->setText(QString("%1").arg(frame.gear - 1));
    }
    else if (frame.gear == 1)
    {
        ui_->GearLabel->setText(QString("N"));
    }
    else if (frame.gear == 0)
    {
        ui_->GearLabel->setText(QString("R"));
    }
}
} // namespace acc_engineer::ui
