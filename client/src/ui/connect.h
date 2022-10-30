#ifndef ACC_ENGINEER_UI_CONNECT_H
#define ACC_ENGINEER_UI_CONNECT_H

#include <QWidget>

namespace Ui {
class Connect;
}

namespace acc_engineer::ui {

class connect : public QWidget
{
    Q_OBJECT
public:
    explicit connect(QWidget *parent = nullptr);

signals:
    void connection(QString address, QString driver, QString password, uint16_t port);

private slots:
    void on_ConnectButton_clicked();
    void on_ExitButton_clicked();

private:
    Ui::Connect *ui_;
};

} // namespace acc_engineer::ui

#endif
