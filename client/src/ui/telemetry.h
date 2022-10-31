#ifndef ACC_ENGINEER_UI_TELEMETRY_H
#define ACC_ENGINEER_UI_TELEMETRY_H
#include <QWidget>

namespace Ui
{
class Telemetry;
}

namespace acc_engineer::ui
{
	class telemetry final : public QWidget
	{
	public:
            explicit telemetry(QWidget *parent = nullptr);
	private:
            Ui::Telemetry *ui_;
	};
}

#endif
