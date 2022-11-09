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
	case tyre_type::LeftFront: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
        ui_->TyreNameLabel->setText(tr("Left Front"));
		break;
	case tyre_type::RightFront: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
        ui_->TyreNameLabel->setText(tr("Right Front"));
		break;
	case tyre_type::LeftRear: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
		ui_->TyreNameLabel->setText(tr("LeftRear"));
		break;
	case tyre_type::RightRear: 
		ui_->TyrePressureFrame->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
		ui_->TyreNameLabel->setText(tr("RightRear"));
		break;
	default: ;
	}
}

void telemetry_tyre::set_tyre_pressure(float pressure) const
{
	ui_->TyrePressureLabel->setText(QString("%1").arg(pressure, 2, 'f', 1));
}

void telemetry_tyre::set_tyre_temperature(float temperature) const
{
	ui_->TyreTemperatureLabel->setText(QString("%1").arg(temperature, 2, 'f', 1));
}

void telemetry_tyre::set_brake_temperature(float temperature) const
{
	ui_->BrakeTemperatureLabel->setText(QString("%1").arg(temperature, 2, 'f', 1));
}

void telemetry_tyre::set_pad_compound(int compound) const
{
	ui_->PadCompoundLabel->setText(QString("%1").arg(compound));
}

void telemetry_tyre::set_pad_wear(float wear) const
{
	ui_->PadWearLabel->setText(QString("%1").arg(wear, 2, 'f', 1));
}

void telemetry_tyre::set_disc_wear(float wear) const
{
	ui_->DiscWearLabel->setText(QString("%1").arg(wear, 2, 'f', 1));
}
}
