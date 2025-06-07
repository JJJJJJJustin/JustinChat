#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "HttpMgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    // 在 Edit 中输入文字时,内容不可视(通过SetEchoMode函数设置)
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    ui->err_tip->setProperty("state", "normal");        // 为 ui->err_tip 附加一个 qss 文件中自定义的类型
    refresh(ui->err_tip);                               // 然后刷新 err_tip 这个控件的类型

    connect(HttpMgr::GetInstance().get(), &HttpMgr::SigRegModeFinish, this, &RegisterDialog::SlotRegModeFinish);

    InitHandlersMap();
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
        QJsonObject jsonObj;
        jsonObj["email"] = emailStr;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gateUrlPrefix + "/get_verifycode"), jsonObj, ReqID::ID_GET_VERIFY_CODE, Module::REGISTER_MOD);
    }
    else
    {
        ShowTip(tr("邮箱地址不正确"), false);
    }
}

void RegisterDialog::on_confirm_button_clicked()
{
    if(ui->user_edit->text() == "")
    {
        ShowTip(tr("用户名不能为空"), false);
        return;
    }

    if(ui->email_edit->text() == "")
    {
        ShowTip(tr("邮箱不能为空"), false);
        return;
    }

    if(ui->pass_edit->text() == "")
    {
        ShowTip(tr("密码不能为空"), false);
        return;
    }

    if(ui->confirm_edit->text() == "")
    {
        ShowTip(tr("确认密码不能为空"), false);
        return;
    }

    if(ui->confirm_edit->text() != ui->pass_edit->text())
    {
        ShowTip(tr("两次密码不相同，请确认后重新尝试"), false);
        return;
    }

    if(ui->verify_edit->text() == "")
    {
        ShowTip(tr("验证码不能为空"), false);
        return;
    }

    QJsonObject jsonObj;
    jsonObj["user"] = ui->user_edit->text();
    jsonObj["email"] = ui->email_edit->text();
    jsonObj["password"] = ui->pass_edit->text();
    jsonObj["confirm"] = ui->confirm_edit->text();
    jsonObj["verifycode"] = ui->verify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gateUrlPrefix + "/user_register"), jsonObj, ReqID::ID_REG_USER, Module::REGISTER_MOD);
}

void RegisterDialog::SlotRegModeFinish(QString res, ReqID reqID, ErrorCode errCode)
{
    if(errCode != ErrorCode::SUCCESS)
    {
        ShowTip(tr("网络请求错误！"), false);
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());       // res 的类型从 QString --> QByteArray --> QJsonDocument
    if(jsonDoc.isNull())
    {
        ShowTip(tr("json 解析错误！"), false);
        return;
    }
    else if(!jsonDoc.isObject())
    {
        ShowTip(tr("json 解析错误！"), false);
        return;
    }

    // 处理数据
    m_HandlersMap[reqID](jsonDoc.object());

    return;
}



void RegisterDialog::ShowTip(QString str, bool state)
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

void RegisterDialog::InitHandlersMap()
{
    // HttpPOST 请求 -> 一系列信号函数与槽函数 ->  SlotRegModeFinish -> 读取服务器验证码服务的处理结果，并在前端 QT 上作出响应
    m_HandlersMap.insert(ReqID::ID_GET_VERIFY_CODE, [this](QJsonObject jsonObj)
    {
        int error = jsonObj["error"].toInt();
        if(error != int(ErrorCode::SUCCESS))
        {
            ShowTip(tr("参数错误！"), false);
            return;
        }

        // 如果 json 中没有 error 信息，则证明验证码发送成功
        QString email = jsonObj["email"].toString();
        qDebug() << "email is:" << email << '\n';
        ShowTip(tr("验证码已经发送，注意查收。"), true);
    });

    // HttpPOST 请求 -> 一系列信号函数与槽函数 ->  SlotRegModeFinish -> 读取服务器注册服务的处理结果，并在前端 QT 上作出响应
    m_HandlersMap.insert(ReqID::ID_REG_USER, [this](QJsonObject jsonObj)
    {
        int error = jsonObj["error"].toInt();
        if(error != int(ErrorCode::SUCCESS))
        {
            ShowTip(tr("参数错误！"), false);
            return;
        }

        QString email = jsonObj["email"].toString();
        qDebug() << "email is:" << email << '\n';
        ShowTip(tr("用户注册成功！"), true);
    });


}
