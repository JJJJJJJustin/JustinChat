#include "global.h"

QString gateUrlPrefix = "";

std::function<void(QWidget*)> refresh = [](QWidget* w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
};

QString MD5Encrypt(const QString &input)
{
    QByteArray byteArray = input.toUtf8();      // 将输入字符串转换为字节数组
    // 使用 MD5 进行加密
    QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Md5);
    // 返回十六进制的字符串类型加密结果
    return QString(hash.toHex());
}
