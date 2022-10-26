#include "driver_input.h"

// spdlog
#include <spdlog/spdlog.h>

// QWT
#include <QwtPlotCurve>
#include <QwtPlotDirectPainter>

// ui
#include "ui_driver_input.h"

namespace acc_engineer::ui {

driver_input::driver_input(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::DriverInput)
    , gas_curve_(new QwtPlotCurve(tr("Gas %", "DriverInput.Title.Curve.Gas")))
    , throttle_curve_(new QwtPlotCurve(tr("Throttle %", "DriverInput.Title.Curve.Throttle")))
    , speed_curve_(new QwtPlotCurve(tr("Speed(KM/H)", "DriverInput.Title.Curve.Speed")))
    , direct_painter_(new QwtPlotDirectPainter)
    , update_tick_(0)
{
    ui_->setupUi(this);
    init_gas_and_throttle_plot();
    init_speed_plot();
}

driver_input::~driver_input()
{
    delete gas_curve_;
    delete throttle_curve_;
    delete speed_curve_;
    delete direct_painter_;
    delete ui_;
}

void driver_input::handle_new_frame(const frame& frame) const
{
    ui_->GasProgressBar->setValue(frame.percent_gas);
    ui_->ThrottleProgressBar->setValue(frame.percent_throttle);
    ui_->GearLabel->setText(frame.gear_display);
}

void driver_input::init_gas_and_throttle_plot() const
{
    ui_->GasAndThrottlePlot->setTitle(tr("Gas And Throttle", "DriverInput.Title.Plot.GasAndThrottle"));
    ui_->GasAndThrottlePlot->setAutoReplot(false);

    ui_->GasAndThrottlePlot->setAxisScale(QwtAxis::YLeft, 0, 100);
    ui_->GasAndThrottlePlot->setAxisScale(QwtAxis::XBottom, 0, 1000);

    gas_curve_->attach(ui_->GasAndThrottlePlot);
    throttle_curve_->attach(ui_->GasAndThrottlePlot);
}

void driver_input::init_speed_plot() const
{
    ui_->SpeedPlot->setTitle(tr("Speed", "DriverInput.Title.Plot.Speed"));
    ui_->SpeedPlot->setAutoReplot(false);

    ui_->SpeedPlot->setAxisScale(QwtAxis::YLeft, 0, 300);
    ui_->SpeedPlot->setAxisScale(QwtAxis::XBottom, 0, 1000);

    speed_curve_->attach(ui_->SpeedPlot);
}
} // namespace acc_engineer::ui
