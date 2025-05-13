#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    // 在 Edit 中输入文字时,内容不可视(通过SetEchoMode函数设置)
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    ui->err_tip->setProperty("state", "normal");        // 为 ui->err_tip 附加一个 qss 文件中自定义的类型
    refresh(ui->err_tip);                               // 然后刷新 err_tip 这个控件的类型
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto emailStr = ui->email_edit->text();

    QRegularExpression regex(R"((\w+)(\.|_)?(\w+)@(\w+)(\.(\w+))+)");
    bool matchResult = regex.match(emailStr).hasMatch();
    if(matchResult)
    {
        // Send HTTP code to user
    }
    else
    {
        showTip(tr("邮箱地址不正确"), false);
    }
}

void RegisterDialog::showTip(QString str, bool state)
{
    if(state == true)
    {
        ui->err_tip->setProperty("state", "normal");
    }
    else
    {
        ui->err_tip->setProperty("state", "error");
    }

    ui->err_tip->setText(str);

    refresh(ui->err_tip);
}

