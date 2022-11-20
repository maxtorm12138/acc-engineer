#include "setter_button.h"

namespace acc_engineer::ui {

setter_button::setter_button(QWidget *parent)
    : QPushButton(parent)
{
    connect(this, &QPushButton::clicked, this, &setter_button::on_clicked);
}

setter_button::setter_button(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    connect(this, &QPushButton::clicked, this, &setter_button::on_clicked);
}

setter_button::setter_button(const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton(icon, text, parent)
{
    connect(this, &QPushButton::clicked, this, &setter_button::on_clicked);
}

void setter_button::on_clicked(bool)
{
    emit clicked(text());
}

} // namespace acc_engineer::ui
