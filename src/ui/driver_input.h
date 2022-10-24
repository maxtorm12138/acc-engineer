#ifndef ACC_ENGINEER_UI_DRIVER_INPUT_H
#define ACC_ENGINEER_UI_DRIVER_INPUT_H
#include <QWidget>
#include <QChart>
#include "shared_memory_emitter.h"

namespace Ui {
class DriverInput;
}

namespace acc_engineer::ui {
class gas_throttle_chart;

class driver_input final : public QWidget
{
    Q_OBJECT
public:
    explicit driver_input(gas_throttle_chart *gas_throttle_chart, QWidget *parent = nullptr);

public slots:
    void handle_new_frame(frame frame);

private:
    Ui::DriverInput *ui_;
};

} // namespace acc_engineer::ui

#endif
