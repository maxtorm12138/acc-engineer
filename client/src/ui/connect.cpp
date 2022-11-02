#include "connect.h"

// QT
#include <qmessagebox.h>

// logic
#include "logic/logic.h"

// ui
#include "ui_connect.h"

namespace acc_engineer::ui {
connect::connect(logic::connection *logic, QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::Connect)
    , logic_(logic)
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

    QObject::connect(logic_, &logic::connection::connection_failure, this, &connect::handle_connection_failure, Qt::QueuedConnection);
    logic_->start_connection(address, driver, password, port);
}

void connect::on_ExitButton_clicked()
{
    close();
}
void connect::handle_connection_failure(uint64_t code, QString message)
{
    SPDLOG_ERROR("handle connection failure message: {}", message.toStdString());
    QMessageBox::critical(this, tr("connection failiure"), message);

    ui_->ConnectButton->setDisabled(false);
    ui_->ExitButton->setDisabled(false);
}
} // namespace acc_engineer::ui
