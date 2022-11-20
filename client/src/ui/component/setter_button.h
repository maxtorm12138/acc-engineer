#ifndef ACC_ENGINEER_UI_COMPONENT_SETTER_BUTTON_H
#define ACC_ENGINEER_UI_COMPONENT_SETTER_BUTTON_H

#include <QPushButton>

namespace acc_engineer::ui {

class setter_button final : public QPushButton
{
    Q_OBJECT
public:
    explicit setter_button(QWidget *parent = nullptr);
    explicit setter_button(const QString &text, QWidget *parent = nullptr);
    setter_button(const QIcon &icon, const QString &text, QWidget *parent = nullptr);

signals:
    void clicked(QString content);

private slots:
    void on_clicked(bool);
};

} // namespace acc_engineer::ui

#endif
