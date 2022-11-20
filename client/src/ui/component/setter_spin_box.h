#ifndef ACC_ENGINEER_UI_COMPONENT_SETTER_SPIN_BOX_H
#define ACC_ENGINEER_UI_COMPONENT_SETTER_SPIN_BOX_H

#include <QSpinBox>

namespace acc_engineer::ui {

class setter_spin_box final : public QSpinBox
{
    Q_OBJECT
public:
    using QSpinBox::QSpinBox;

public slots:
    void offset(QString val);
};

class setter_double_spin_box final : public QDoubleSpinBox
{
    Q_OBJECT
public:
    using QDoubleSpinBox::QDoubleSpinBox;

public slots:
    void offset(QString val);
};

} // namespace acc_engineer::ui

#endif
