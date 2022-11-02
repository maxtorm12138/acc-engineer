#ifndef ACC_ENGINEER_UI_CONNECT_H
#define ACC_ENGINEER_UI_CONNECT_H

#include <QWidget>

namespace Ui {
class Connect;
}

namespace acc_engineer::logic {
class connection;
}

namespace acc_engineer::ui {

class connect : public QWidget
{
    Q_OBJECT
public:
    explicit connect(logic::connection *logic, QWidget *parent = nullptr);
    
signals:

private slots:
    void on_ConnectButton_clicked();
    void on_ExitButton_clicked();
    
    void handle_connection_failure(uint64_t code, QString message);
private:
    Ui::Connect *ui_;
    logic::connection *logic_;
};

} // namespace acc_engineer::ui

#endif
