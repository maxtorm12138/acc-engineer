#ifndef ACC_ENGINEER_UI_STRATEGY_H
#define ACC_ENGINEER_UI_STRATEGY_H

#include <QWidget>

namespace Ui
{
class Strategy;
}

namespace acc_engineer::ui
{

class strategy final : public QWidget
{
public:
	explicit strategy(QWidget *parent = nullptr);

private:
	Ui::Strategy *ui_;
};

}
#endif
