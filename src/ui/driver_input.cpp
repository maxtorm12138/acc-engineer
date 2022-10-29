#include "driver_input.h"

// spdlog
#include <spdlog/spdlog.h>

// QWT
#include <qwt_plot_opengl_canvas.h>
#include <QwtPlotCurve>
#include <QwtPlotDirectPainter>
#include <QwtCPointerData>

// ui
#include "ui_driver_input.h"

namespace acc_engineer::ui {

driver_input::driver_input(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::DriverInput)
{
    ui_->setupUi(this);

    for (int i = 0; i < driver_input_dots; i++)
    {
        gas_sample_.x_view[i] = i;
        gas_sample_.x_buffer.push_back(i);

        throttle_sample_.x_view[i] = i;
        throttle_sample_.x_buffer.push_back(i);

        speed_sample_.x_view[i] = i;
        speed_sample_.x_buffer.push_back(i);
    }

    init_gas_and_throttle_plot();
    init_speed_plot();
    init_steering_plot();

    ui_->GasAndThrottlePlot->replot();
    ui_->SpeedPlot->replot();
}

driver_input::~driver_input()
{
    delete ui_;
}

void driver_input::handle_new_frame(shared_memory::frame frame)
{
    const float percent_gas = frame.gas * 100;
    const float percent_brake = frame.brake * 100;

    ui_->GasProgressBar->setValue(static_cast<int>(percent_gas));
    ui_->ThrottleProgressBar->setValue(static_cast<int>(percent_brake));
    ui_->GearLabel->setText(format_gear(frame.gear));
    ui_->SpeedLabel->setText(QString("%1").arg(frame.speed_kmh));

    const float frame_x = frame_count_;

    gas_sample_.x_buffer.push_back(frame_x);
    throttle_sample_.x_buffer.push_back(frame_x);
    speed_sample_.x_buffer.push_back(frame_x);

    gas_sample_.y_buffer.push_back(percent_gas);
    throttle_sample_.y_buffer.push_back(percent_brake);
    speed_sample_.y_buffer.push_back(frame.speed_kmh);

    gas_sample_.buffer_to_view();
    throttle_sample_.buffer_to_view();
    speed_sample_.buffer_to_view();

    ui_->GasAndThrottlePlot->setAxisScale(QwtAxis::XBottom, frame_x - 995, frame_x + 5, driver_input_dots);
    ui_->SpeedPlot->setAxisScale(QwtAxis::XBottom, frame_x - 995, frame_x + 5, driver_input_dots);

    ui_->GasAndThrottlePlot->replot();
    ui_->SpeedPlot->replot();

    frame_count_++;
}

void driver_input::init_gas_and_throttle_plot()
{
    gas_curve_ = new QwtPlotCurve(tr("Gas %", "DriverInput.Title.Curve.Gas"));
    throttle_curve_ = new QwtPlotCurve(tr("Throttle %", "DriverInput.Title.Curve.Throttle"));
    gas_and_throttle_painter_ = new QwtPlotDirectPainter(ui_->GasAndThrottlePlot);

    ui_->GasAndThrottlePlot->setCanvas(new QwtPlotOpenGLCanvas);
    ui_->GasAndThrottlePlot->setTitle(tr("Gas And Throttle", "DriverInput.Title.Plot.GasAndThrottle"));
    ui_->GasAndThrottlePlot->setAutoReplot(false);

    ui_->GasAndThrottlePlot->setAxisScale(QwtAxis::YLeft, 0, 100);
    ui_->GasAndThrottlePlot->setAxisScale(QwtAxis::XBottom, 0, driver_input_dots, driver_input_dots);
    ui_->GasAndThrottlePlot->setAxisVisible(QwtAxis::XBottom, false);

    gas_curve_->setPen(Qt::red, 3);
    throttle_curve_->setPen(Qt::blue, 3);

    gas_curve_->attach(ui_->GasAndThrottlePlot);
    throttle_curve_->attach(ui_->GasAndThrottlePlot);

    gas_curve_->setData(new QwtCPointerData<float>(gas_sample_.x_view.data(), gas_sample_.y_view.data(), driver_input_dots));
    throttle_curve_->setData(new QwtCPointerData<float>(throttle_sample_.x_view.data(), throttle_sample_.y_view.data(), driver_input_dots));
}

void driver_input::init_speed_plot()
{
    speed_curve_ = new QwtPlotCurve(tr("Speed(KM/H)", "DriverInput.Title.Curve.Speed"));

    ui_->SpeedPlot->setCanvas(new QwtPlotOpenGLCanvas);
    ui_->SpeedPlot->setTitle(tr("Speed", "DriverInput.Title.Plot.Speed"));
    ui_->SpeedPlot->setAutoReplot(false);

    ui_->SpeedPlot->setAxisScale(QwtAxis::YLeft, 0, 300);
    ui_->SpeedPlot->setAxisScale(QwtAxis::XBottom, 0, driver_input_dots, driver_input_dots);
    ui_->SpeedPlot->setAxisVisible(QwtAxis::XBottom, false);

    speed_painter_ = new QwtPlotDirectPainter(ui_->SpeedPlot);

    speed_curve_->setPen(Qt::yellow, 3);
    speed_curve_->attach(ui_->SpeedPlot);

    speed_curve_->setData(new QwtCPointerData(speed_sample_.x_view.data(), speed_sample_.y_view.data(), driver_input_dots));
}

void driver_input::init_steering_plot()
{
    steering_curve_ = new QwtPlotCurve(tr("Steering", "DriverInput.Title.Curve.Steering"));

    ui_->SteeringPlot->setCanvas(new QwtPlotOpenGLCanvas);
    ui_->SteeringPlot->setTitle(tr("Steering", "DriverInput.Title.Plot.Steering"));
    ui_->SteeringPlot->setAutoReplot(false);

    ui_->SteeringPlot->setAxisVisible(QwtAxis::XBottom, false);

    steering_painter_ = new QwtPlotDirectPainter(ui_->SteeringPlot);
}

QString driver_input::format_gear(int gear)
{
    switch (gear)
    {
    case 0:
        return "R";
    case 1:
        return "N";
    default:
        return QString("%1").arg(gear - 1);
    }
}

void driver_input::sample::buffer_to_view()
{
    std::copy(x_buffer.begin(), x_buffer.end(), x_view.begin());
    std::copy(y_buffer.begin(), y_buffer.end(), y_view.begin());
}
} // namespace acc_engineer::ui
