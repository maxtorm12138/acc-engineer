#ifndef ACC_ENGINEER_UI_DRIVER_INPUT_H
#define ACC_ENGINEER_UI_DRIVER_INPUT_H
#include <QWidget>

#include "shared_memory_emitter.h"

namespace Ui {
class DriverInput;
}

namespace acc_engineer::ui {
class driver_input final : public QWidget
{
    Q_OBJECT
public:
    explicit driver_input(QWidget *parent = nullptr);

signals:

public slots:
    void handle_new_frame(frame frame);

private:
    Ui::DriverInput *ui_;
};

} // namespace acc_engineer::ui

#endif
