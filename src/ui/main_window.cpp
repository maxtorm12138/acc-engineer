#include "main_window.h"
#include "driver_input.h"
#include "ui_mainwindow.h"


namespace acc_engineer::ui {
main_window::main_window(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
}
} // namespace acc_engineer::ui
