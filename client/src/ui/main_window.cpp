#include "main_window.h"
#include "driver_input.h"
#include "ui_main_window.h"

namespace acc_engineer::ui {
main_window::main_window(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::MainWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui_->setupUi(this);
}
} // namespace acc_engineer::ui
