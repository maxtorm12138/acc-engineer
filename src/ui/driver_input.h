#ifndef ACC_ENGINEER_UI_DRIVER_INPUT_H
#define ACC_ENGINEER_UI_DRIVER_INPUT_H
#include <QWidget>

namespace Ui
{
	class DriverInput;
}

namespace acc_engineer::ui
{
	class driver_input final : public QWidget
	{
		Q_OBJECT
	public:
		explicit driver_input(QWidget* parent = nullptr);
	private:
		Ui::DriverInput* ui_;
	};
}

#endif
