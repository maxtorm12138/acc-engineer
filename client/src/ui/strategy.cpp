#include "strategy.h"

#include "ui_strategy.h"

namespace acc_engineer::ui {
strategy::strategy(QWidget *parent) : QWidget(parent), ui_(new Ui::Strategy) {
	ui_->setupUi(this);
}


} // namespace acc_engineer::ui