#include "setter_spin_box.h"

#include <spdlog/spdlog.h>

namespace acc_engineer::ui {
void setter_spin_box::offset(QString val)
{
    const auto offset = val.toInt();
    setValue(value() + offset);
}

void setter_double_spin_box::offset(QString val)
{
    const auto offset = val.toDouble();
    setValue(value() + offset);
}
} // namespace acc_engineer::ui
