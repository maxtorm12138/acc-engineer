#ifndef ACC_ENGINEER_UI_GAS_CHART_H
#define ACC_ENGINEER_UI_GAS_CHART_H

#include <QChart>

#include "shared_memory_emitter.h"

class QLineSeries;
class QValueAxis;

namespace acc_engineer::ui {
class gas_throttle_chart final : public QChart
{
    Q_OBJECT
public:
    explicit gas_throttle_chart(QGraphicsItem *parent = nullptr, Qt::WindowFlags window_flags = Qt::WindowFlags());

public slots:
    void handle_new_frame(frame frame);

private:
    QLineSeries *gas_spline_series_;
    QLineSeries *throttle_spline_series_;
    QValueAxis *axis_x_;
    QValueAxis *axis_y_;
    qreal pos_x_;
    uint64_t frame_counter_;
};
} // namespace acc_engineer::ui

#endif
