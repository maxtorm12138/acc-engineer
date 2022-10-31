#ifndef ACC_ENGINEER_UI_TELEMETRY_TYRE_H
#define ACC_ENGINEER_UI_TELEMETRY_TYRE_H

#include <QFrame>

namespace Ui
{
class TelemetryTyre;
}

namespace acc_engineer::ui
{
	enum class tyre_type
	{
		FrontLeft = 0,
		FrontRight = 1,
		RearLeft = 2,
		RearRight = 3
	};

	class telemetry_tyre final : public QFrame
	{
		Q_OBJECT
	public:
		explicit telemetry_tyre(QWidget *parent = nullptr);

		void set_type(tyre_type type);
	private:
		Ui::TelemetryTyre *ui_;
	};
}

#endif
