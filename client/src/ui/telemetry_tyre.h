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
		LeftFront = 0,
		RightFront = 1,
		LeftRear = 2,
		RightRear = 3
	};

	class telemetry_tyre final : public QFrame
	{
		Q_OBJECT
	public:
		explicit telemetry_tyre(QWidget *parent = nullptr);

		void set_type(tyre_type type);

		void set_tyre_pressure(float pressure) const;

		void set_tyre_temperature(float temperature) const;

		void set_brake_temperature(float temperature) const;

		void set_pad_compound(int compound) const;

		void set_pad_wear(float wear) const;

		void set_disc_wear(float wear) const;

	private:
		Ui::TelemetryTyre *ui_;
	};
}

#endif
