#ifndef GLOBAL_H
#define GLOBAL_H

#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QSettings>
#include <QWidget>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

#include "QStyle"

extern QString gateUrlPrefix;
extern std::function<void(QWidget*)> refresh;
extern QString MD5Encrypt(const QString&);

// 网络请求标识符
enum class ReqID
{
    ID_GET_VERIFY_CODE = 1001,
    ID_REG_USER = 1002

};

// 功能模块
enum class Module
{
    REGISTER_MOD = 0

};

// 错误码（用于鉴别是否发生错误和错误类型）
enum class ErrorCode
{
    SUCCESS = 0,
    ERR_JSON = 1,
    ERR_NETWORK = 2
};

enum ErrTipType
{
    TIP_SUCCESS = 0,
    TIP_USER_ERR = 1,
    TIP_EMAIL_ERR = 2,
    TIP_PWD_ERR = 3,
    TIP_PWD_CONFIRM_ERR = 4,        // “确认密码”栏格式错误
    TIP_CONFIRM_ERR = 5,            // “密码”和“确认密码”不匹配
    TIP_VERIFY_ERR = 6
};
#endif // GLOBAL_H
