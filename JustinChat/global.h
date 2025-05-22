#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWidget>
#include <QRegularExpression>
#include <functional>
#include <QString>
#include <QSettings>
#include <memory>
#include <mutex>
#include <iostream>

#include "QStyle"

extern QString gateUrlPrefix;
extern std::function<void(QWidget*)> refresh;

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

#endif // GLOBAL_H
