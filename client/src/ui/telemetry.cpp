#include "telemetry.h"
#include "ui_telemetry.h"

namespace acc_engineer::ui
{
telemetry::telemetry(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::Telemetry)
{
    ui_->setupUi(this);
    ui_->FrontLeftTyre->set_type(tyre_type::LeftFront);
    ui_->FrontRightTyre->set_type(tyre_type::RightFront);
    ui_->RearLeftTyre->set_type(tyre_type::LeftRear);
    ui_->RearRightTyre->set_type(tyre_type::RightRear);
}

}
