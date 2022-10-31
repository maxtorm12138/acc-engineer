#include "telemetry.h"
#include "ui_telemetry.h"

namespace acc_engineer::ui
{
telemetry::telemetry(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::Telemetry)
{
    ui_->setupUi(this);
    ui_->FrontLeftTyre->set_type(tyre_type::FrontLeft);
    ui_->FrontRightTyre->set_type(tyre_type::FrontRight);
    ui_->RearLeftTyre->set_type(tyre_type::RearLeft);
    ui_->RearRightTyre->set_type(tyre_type::RearRight);
}
}
