#include "connect.h"

#include "ui_connect.h"
namespace acc_engineer::ui {
connect::connect(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::Connect)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui_->setupUi(this);
}

void connect::on_ConnectButton_clicked()
{
    ui_->ConnectButton->setDisabled(true);
    ui_->ExitButton->setDisabled(true);

    auto address = ui_->AddressEdit->text();
    auto driver = ui_->DriverEdit->text();
    auto password = ui_->PasswordEdit->text();
    auto port = ui_->PortEdit->text().toUInt();

    emit connection(address, driver, password, port);
}

void connect::on_ExitButton_clicked()
{
    close();
}
} // namespace acc_engineer::ui
