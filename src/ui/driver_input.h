#ifndef ACC_ENGINEER_UI_DRIVER_INPUT_H
#define ACC_ENGINEER_UI_DRIVER_INPUT_H
#include <QWidget>
#include "shared_memory_emitter.h"

namespace Ui
{
class DriverInput;
}

class QwtPlotCurve;
class QwtPlotDirectPainter;

namespace acc_engineer::ui {
class gas_throttle_chart;

class driver_input final : public QWidget
{
    Q_OBJECT
public:
    explicit driver_input(QWidget *parent = nullptr);

    ~driver_input() override;

public slots:
    void handle_new_frame(const frame& frame) const;

private:
    void init_gas_and_throttle_plot() const;
    void init_speed_plot() const;

private:
    Ui::DriverInput *ui_;
    QwtPlotCurve *gas_curve_;
    QwtPlotCurve *throttle_curve_;
    QwtPlotCurve *speed_curve_;
    QwtPlotDirectPainter *direct_painter_;
    uint64_t update_tick_;
};

} // namespace acc_engineer::ui

#endif
