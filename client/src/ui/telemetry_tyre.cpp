#include "telemetry_tyre.h"
#include "telemetry_tyre.h"

#include "ui_telemetry_tyre.h"

namespace acc_engineer::ui
{
telemetry_tyre::telemetry_tyre(QWidget *parent)
    : QFrame(parent)
    , ui_(new Ui::TelemetryTyre)
{
    ui_->setupUi(this);
}

void telemetry_tyre::set_type(tyre_type type)
{
	switch (type)
	{
	case tyre_type::FrontLeft: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
		break;
	case tyre_type::FrontRight: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
		break;
	case tyre_type::RearLeft: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
		break;
	case tyre_type::RearRight: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
		break;
	default: ;
	}
}
}
