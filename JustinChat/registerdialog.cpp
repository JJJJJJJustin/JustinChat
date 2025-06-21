#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "HttpMgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    ui->pass_edit->setEchoMode(QLineEdit::Password);    // 在 Edit 中输入文字时,内容不可视(通过SetEchoMode函数设置)
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    ui->err_tip->setProperty("state", "normal");        // 为 ui->err_tip 附加一个 qss 文件中自定义的类型
    refresh(ui->err_tip);                               // 然后刷新 err_tip 这个控件的类型
    ui->err_tip->clear();                               // 当没有触发任何错误时，不显示 errTip（默认字样也不显示）

    connect(ui->user_edit,  &QLineEdit::editingFinished, this, [this](){ CheckUserValid(); });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){ CheckEmailValid(); });
    connect(ui->pass_edit,  &QLineEdit::editingFinished, this, [this](){ CheckPassValid(); });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this](){ CheckConfirmValid(); });
    connect(ui->verify_edit,  &QLineEdit::editingFinished, this, [this](){ CheckVerifyValid(); });
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
    CHECK_VALID(CheckUserValid);
    CHECK_VALID(CheckEmailValid);
    CHECK_VALID(CheckPassValid);
    CHECK_VALID(CheckConfirmValid);
    CHECK_VALID(CheckVerifyValid);

    QJsonObject jsonObj;
    jsonObj["user"] = ui->user_edit->text();
    jsonObj["email"] = ui->email_edit->text();
    jsonObj["password"] = MD5Encrypt(ui->pass_edit->text());
    jsonObj["confirm"] = MD5Encrypt(ui->confirm_edit->text());
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
        qDebug() << "user's uid is:" << jsonObj["uid"].toString();
        qDebug() << "email is:" << email << '\n';
        ShowTip(tr("用户注册成功！"), true);
    });

}

void RegisterDialog::AddErrTip(ErrTipType et, QString str)
{
    m_ErrTips.insert(et, str);
    ShowTip(str, false);
}

void RegisterDialog::DeleteErrTip(ErrTipType et)
{
    m_ErrTips.remove(et);

    if(m_ErrTips.empty())
    {
        ui->err_tip->clear();
        return;
    }

    ShowTip(m_ErrTips.first(), false);          // 删除之后需要更新 ErrTip，在这里更新为 m_ErrTips 的首元素
}

bool RegisterDialog::CheckUserValid()
{
    if(ui->user_edit->text() == "")
    {
        AddErrTip(ErrTipType::TIP_USER_ERR, "用户名不能为空!");
        return false;
    }
    DeleteErrTip(ErrTipType::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::CheckEmailValid()
{
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w+)@(\w+)(\.(\w+))+)");
    bool matchResult = regex.match(email).hasMatch();
    if(!matchResult)
    {
        AddErrTip(ErrTipType::TIP_EMAIL_ERR, "请检查邮箱格式!");
        return false;
    }
    DeleteErrTip(ErrTipType::TIP_EMAIL_ERR);
    return true;
}

bool RegisterDialog::CheckPassValid()
{
    auto pass =ui->pass_edit->text();
    if(pass.length() < 6 || pass.length() > 15)
    {
        AddErrTip(ErrTipType::TIP_PWD_ERR, "密码长度应该在 6~15 之间！");
        return false;
    }

    QRegularExpression regex("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool matchResult = regex.match(pass).hasMatch();
    if(!matchResult)
    {
        AddErrTip(ErrTipType::TIP_PWD_ERR, "密码只能包含数字，字母，以及!@#$%^&*");
        return false;
    }

    DeleteErrTip(ErrTipType::TIP_PWD_ERR);
    return true;
}

bool RegisterDialog::CheckConfirmValid()
{
    if(ui->confirm_edit->text() != ui->pass_edit->text())
    {
        AddErrTip(ErrTipType::TIP_CONFIRM_ERR, "前后两次密码不相同!");
        return false;
    }
    DeleteErrTip(ErrTipType::TIP_CONFIRM_ERR);
    return true;
}

bool RegisterDialog::CheckVerifyValid()
{
    auto verifyCode = ui->verify_edit->text();
    if(verifyCode.isEmpty())
    {
        AddErrTip(ErrTipType::TIP_VERIFY_ERR, "验证码不能为空!");
        return false;
    }
    DeleteErrTip(ErrTipType::TIP_VERIFY_ERR);
    return true;
}
