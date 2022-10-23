#include "driver_input.h"

#include "ui_driverinput.h"

namespace acc_engineer::ui
{
	driver_input::driver_input(QWidget* parent)
		: QWidget(parent)
		  , ui_(new Ui::DriverInput)
	{
		ui_->setupUi(this);
	}
}
