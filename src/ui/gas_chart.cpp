#include "gas_chart.h"
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

namespace acc_engineer::ui {
gas_chart::gas_chart(QGraphicsItem *parent, Qt::WindowFlags window_flags)
    : QChart(parent, window_flags),
    spline_series_(nullptr),
	axis_x_(new QValueAxis),
    axis_y_(new QValueAxis)
{
    spline_series_ = new QSplineSeries(this);
    spline_series_->setPen(QPen(Qt::yellow));

    addSeries(spline_series_);

    addAxis(axis_x_, Qt::AlignBottom);
    addAxis(axis_y_, Qt::AlignLeft);

    spline_series_->attachAxis(axis_x_);
    spline_series_->attachAxis(axis_y_);

    axis_x_->setTickCount(600);
    axis_x_->setRange(0, 10);
    axis_y_->setRange(0, 100);
}

void gas_chart::handle_new_frame(frame frame)
{
    qreal y = frame.percent_gas;

    spline_series_->append(x_++, y);
}
} // namespace acc_engineer::ui