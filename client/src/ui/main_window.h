#ifndef ACC_ENGINEER_UI_MAIN_WINDOW_H
#define ACC_ENGINEER_UI_MAIN_WINDOW_H
#include <QWidget>

namespace Ui {
class MainWindow;
}

namespace acc_engineer::ui {
class main_window final : public QWidget
{
    Q_OBJECT
public:
    explicit main_window(QWidget *parent = nullptr);

private:
    Ui::MainWindow *ui_;
};

} // namespace acc_engineer::ui

#endif