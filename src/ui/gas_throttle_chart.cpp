#include "gas_throttle_chart.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <spdlog/spdlog.h>

namespace acc_engineer::ui {
gas_throttle_chart::gas_throttle_chart(QGraphicsItem *parent, Qt::WindowFlags window_flags)
    : QChart(parent, window_flags)
    , gas_spline_series_(nullptr)
    , throttle_spline_series_(nullptr)
    , axis_x_(new QValueAxis)
    , axis_y_(new QValueAxis)
    , pos_x_(0)
    , frame_counter_(0)
{
    gas_spline_series_ = new QLineSeries(this);
    throttle_spline_series_ = new QLineSeries(this);

    gas_spline_series_->setPen(QPen(QBrush(QColor(Qt::blue)), 2));
    gas_spline_series_->setUseOpenGL(true);

    throttle_spline_series_->setPen(QPen(QBrush(QColor(Qt::red)), 2));
    throttle_spline_series_->setUseOpenGL(true);

    addSeries(gas_spline_series_);
    addSeries(throttle_spline_series_);

    addAxis(axis_x_, Qt::AlignBottom);
    addAxis(axis_y_, Qt::AlignLeft);

    gas_spline_series_->attachAxis(axis_x_);
    gas_spline_series_->attachAxis(axis_y_);

    throttle_spline_series_->attachAxis(axis_x_);
    throttle_spline_series_->attachAxis(axis_y_);

    axis_x_->setTickCount(400);
    axis_x_->setRange(0, 400);
    axis_y_->setRange(0, 101);
}

void gas_throttle_chart::handle_new_frame(frame frame)
{
    gas_spline_series_->append(pos_x_, frame.percent_gas);
    throttle_spline_series_->append(pos_x_, frame.percent_throttle + 1.0);

    pos_x_++;
    frame_counter_++;

    if (frame_counter_ == 400)
    {
        scroll((plotArea().width() / 400) * 5, 0);
        frame_counter_ = 395;
        gas_spline_series_->removePoints(0, 5);
        throttle_spline_series_->removePoints(0, 5);
    }
}
} // namespace acc_engineer::ui