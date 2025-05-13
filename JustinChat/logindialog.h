#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

// 该 LoginDialog 属于 UI 命名空间下，继承于 Ui_LoginDialog
namespace Ui {
    class LoginDialog;
}

// 该 LoginDialog 属于全局范围下，继承于 QDialog
class LoginDialog : public QDialog              // LoginDialog 继承于 QDialog，所以有一些默认的成员函数可以使用（比如 show()）
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    Ui::LoginDialog *ui;                        // 注意：此处使用的类型是 UI::LoginDialog，而非 ::LoginDialog

signals:
    void SwitchRegister();

};

#endif // LOGINDIALOG_H
