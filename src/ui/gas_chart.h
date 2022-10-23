#ifndef ACC_ENGINEER_UI_GAS_CHART_H
#define ACC_ENGINEER_UI_GAS_CHART_H

#include <QChart>

#include "shared_memory_emitter.h"

class QSplineSeries;
class QValueAxis;

namespace acc_engineer::ui {
class gas_chart final : public QChart
{
    Q_OBJECT
public:
    explicit gas_chart(QGraphicsItem *parent = nullptr, Qt::WindowFlags window_flags= Qt::WindowFlags());

public slots:
    void handle_new_frame(frame frame);

private:
    QSplineSeries *spline_series_;
	QValueAxis *axis_x_;
    QValueAxis *axis_y_;
	qreal x_{0};
};
} // namespace acc_engineer::ui

#endif
