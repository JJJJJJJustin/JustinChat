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
        HttpMgr::GetInstance()->PostHttpReq(QUrl("http://localhost:8080/get_varifycode"), jsonObj, ReqID::ID_GET_VERIFY_CODE, Module::REGISTER_MOD);
    }
    else
    {
        ShowTip(tr("邮箱地址不正确"), false);
    }
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
    m_HandlersMap.insert(ReqID::ID_GET_VERIFY_CODE, [this](QJsonObject jsonObj)
    {
        int error = jsonObj["error"].toInt();
        if(error != int(ErrorCode::SUCCESS))
        {
            ShowTip(tr("参数错误！"), false);
            return;
        }

        QString email = jsonObj["email"].toString();

        // TODO: 发送验证码

        ShowTip(tr("验证码已经发送，注意查收。"), true);
        qDebug() << "email is:" << email << '\n';
    });

}
