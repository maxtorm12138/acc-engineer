#include "driver_input.h"

// spdlog
#include <spdlog/spdlog.h>

// QWT
#include <qwt_plot_opengl_canvas.h>
#include <QwtPlotCurve>
#include <QwtCPointerData>

// ui
#include "ui_driver_input.h"

namespace acc_engineer::ui {

driver_input::driver_input(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::DriverInput)
{
    ui_->setupUi(this);

    for (int i = 0; i < driver_input_plot_dot_size; i++)
    {
        throttle_sample_.x_view[i] = i;
        throttle_sample_.x_buffer.push_back(i);

        brake_sample_.x_view[i] = i;
        brake_sample_.x_buffer.push_back(i);

        speed_sample_.x_view[i] = i;
        speed_sample_.x_buffer.push_back(i);

        steering_sample_.x_view[i] = i;
        steering_sample_.x_buffer.push_back(i);
    }

    init_throttle_plot();
    init_brake_plot();
    init_speed_plot();
    init_steering_plot();
}

driver_input::~driver_input()
{
    delete ui_;
}

void driver_input::handle_new_frame(shared_memory::frame frame)
{
    const float percent_gas = frame.gas * 100;
    const float percent_brake = frame.brake * 100;
    const float abs_in_action = frame.abs_in_action * 100;
    const float tc_in_action = frame.tc_in_action * 100;
    const float tick = frame.tick + driver_input_plot_dot_size;
    const float tick_range_min = tick - (driver_input_plot_dot_size - 5);
    const float tick_range_max = tick + 5;

    ui_->ThrottleProgressBar->setFormat("%p%");
    ui_->ThrottleProgressBar->setValue(static_cast<int>(percent_gas));

    ui_->TCInActionProgressBar->setFormat("%p%");
    ui_->TCInActionProgressBar->setValue(static_cast<int>(tc_in_action));

    ui_->BrakeProgressBar->setFormat("%p%");
    ui_->BrakeProgressBar->setValue(static_cast<int>(percent_brake));

    ui_->ABSInActionProgressBar->setFormat("%p%");
    ui_->ABSInActionProgressBar->setValue(static_cast<int>(abs_in_action));

    // ui_->SteeringProgressBar->setFormat("%p%");
    // ui_->SteeringProgressBar->setValue(static_cast<int>(steering_degree));

    ui_->GearLabel->setText(format_gear(frame.gear));
    ui_->SpeedLabel->setText(QString("%1").arg(frame.speed_kmh, 3, 'f', 0));
    ui_->ABSLabel->setText(QString("%1").arg(frame.abs));
    ui_->TCLabel->setText(QString("%1").arg(frame.tc));
    ui_->TC2Label->setText(QString("%1").arg(frame.tc2));
    ui_->EngineMapLabel->setText(QString("%1").arg(frame.engine_map));
    ui_->FuelLabel->setText(QString("%1").arg(frame.fuel, 3, 'f', 2));


    throttle_sample_.x_buffer.push_back(tick);
    brake_sample_.x_buffer.push_back(tick);
    speed_sample_.x_buffer.push_back(tick);
    // steering_sample_.x_buffer.push_back(tick);

    throttle_sample_.y_buffer.push_back(percent_gas);
    brake_sample_.y_buffer.push_back(percent_brake);
    speed_sample_.y_buffer.push_back(frame.speed_kmh);
    // steering_sample_.y_buffer.push_back(steering_degree);

    throttle_sample_.buffer_to_view();
    brake_sample_.buffer_to_view();
    speed_sample_.buffer_to_view();
    steering_sample_.buffer_to_view();

    ui_->ThrottlePlot->setAxisScale(QwtAxis::XBottom, tick_range_min, tick_range_max, driver_input_plot_dot_size);
    ui_->BrakePlot->setAxisScale(QwtAxis::XBottom, tick_range_min, tick_range_max, driver_input_plot_dot_size);
    ui_->SpeedPlot->setAxisScale(QwtAxis::XBottom, tick_range_min, tick_range_max, driver_input_plot_dot_size);
    // ui_->SteeringPlot->setAxisScale(QwtAxis::XBottom,  tick- 995,  tick+ 5, driver_input_dots);

    ui_->ThrottlePlot->replot();
    ui_->BrakePlot->replot();
    ui_->SpeedPlot->replot();
    // ui_->SteeringPlot->replot();

}

void driver_input::init_throttle_plot()
{
    throttle_curve_ = new QwtPlotCurve(tr("Throttle %", "DriverInput.Title.Curve.Throttle"));

    ui_->ThrottlePlot->setCanvas(new QwtPlotOpenGLCanvas);
    ui_->ThrottlePlot->setTitle(tr("Throttle", "DriverInput.Title.Plot.Throttle"));
    ui_->ThrottlePlot->setAutoReplot(false);

    ui_->ThrottlePlot->setAxisScale(QwtAxis::YLeft, 0, 100, 50);
    ui_->ThrottlePlot->setAxisScale(QwtAxis::XBottom, 0, driver_input_plot_dot_size, driver_input_plot_dot_size);
    ui_->ThrottlePlot->setAxisVisible(QwtAxis::XBottom, false);

    throttle_curve_->setPen(Qt::green, 3);
    throttle_curve_->attach(ui_->ThrottlePlot);

    throttle_curve_->setData(new QwtCPointerData(throttle_sample_.x_view.data(), throttle_sample_.y_view.data(), driver_input_plot_dot_size));

    ui_->ThrottlePlot->replot();
}

void driver_input::init_brake_plot()
{
    brake_curve_ = new QwtPlotCurve(tr("Brake %", "DriverInput.Title.Curve.Brake"));

    ui_->BrakePlot->setCanvas(new QwtPlotOpenGLCanvas);
    ui_->BrakePlot->setTitle(tr("Brake", "DriverInput.Title.Plot.Brake"));
    ui_->BrakePlot->setAutoReplot(false);

    ui_->BrakePlot->setAxisScale(QwtAxis::YLeft, 0, 100, 50);
    ui_->BrakePlot->setAxisScale(QwtAxis::XBottom, 0, driver_input_plot_dot_size, driver_input_plot_dot_size);
    ui_->BrakePlot->setAxisVisible(QwtAxis::XBottom, false);

    brake_curve_->setPen(Qt::red, 3);

    brake_curve_->attach(ui_->BrakePlot);

    brake_curve_->setData(new QwtCPointerData(brake_sample_.x_view.data(), brake_sample_.y_view.data(), driver_input_plot_dot_size));

    ui_->BrakePlot->replot();
}

void driver_input::init_speed_plot()
{
    speed_curve_ = new QwtPlotCurve(tr("Speed(KM/H)", "DriverInput.Title.Curve.Speed"));

    ui_->SpeedPlot->setCanvas(new QwtPlotOpenGLCanvas);
    ui_->SpeedPlot->setTitle(tr("Speed", "DriverInput.Title.Plot.Speed"));
    ui_->SpeedPlot->setAutoReplot(false);

    ui_->SpeedPlot->setAxisScale(QwtAxis::YLeft, 0, 300, 100);
    ui_->SpeedPlot->setAxisScale(QwtAxis::XBottom, 0, driver_input_plot_dot_size, driver_input_plot_dot_size);
    ui_->SpeedPlot->setAxisVisible(QwtAxis::XBottom, false);

    speed_curve_->setPen(Qt::yellow, 3);
    speed_curve_->attach(ui_->SpeedPlot);

    speed_curve_->setData(new QwtCPointerData(speed_sample_.x_view.data(), speed_sample_.y_view.data(), driver_input_plot_dot_size));

    ui_->SpeedPlot->replot();
}

void driver_input::init_steering_plot()
{
    steering_curve_ = new QwtPlotCurve(tr("Steering", "DriverInput.Title.Curve.Steering"));

    ui_->SteeringPlot->setCanvas(new QwtPlotOpenGLCanvas);
    ui_->SteeringPlot->setTitle(tr("Steering", "DriverInput.Title.Plot.Steering"));
    ui_->SteeringPlot->setAutoReplot(false);

    ui_->SteeringPlot->setAxisScale(QwtAxis::XBottom, 0, driver_input_plot_dot_size, driver_input_plot_dot_size);
    ui_->SteeringPlot->setAxisVisible(QwtAxis::XBottom, false);

    steering_curve_->setPen(Qt::darkYellow, 3);
    steering_curve_->attach(ui_->SteeringPlot);

    steering_curve_->setData(new QwtCPointerData(steering_sample_.x_view.data(), steering_sample_.y_view.data(), driver_input_plot_dot_size));

    ui_->SteeringPlot->replot();
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
