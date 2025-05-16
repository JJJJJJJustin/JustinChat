#include "httpmgr.h"
#include <QNetWorkReply.h>

HttpMgr::~HttpMgr()
{

}

HttpMgr::HttpMgr()
{
    // 连接两个函数 SigHttpFinish & SlotHttpFinish
    connect(this, &HttpMgr::SigHttpFinish, this, &HttpMgr::SlotHttpFinish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject jsonObj, ReqID reqID, Module mod)
{
    // 将 JsonOjbect 类型数据转换为 QByteArray 类型
    QByteArray data = QJsonDocument(jsonObj).toJson();

    // 设置请求头（分别设置了 1请求体的数据类型 2请求体的字节数）
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    //发送请求并获取回执、处理响应(发送请求时，使用 POST 类型请求)
    QNetworkReply* reply = m_Manager.post(request, data);

    auto self = shared_from_this();

    // 此时一旦 Http 信号发送完成，则 reply 会释放出 QNetworkReply::finished
    // 我们可以以此为标识，在 lambda 表达式中进行一些操作，并为槽函数发送 SigHttpFinish 信号
    QObject::connect(reply, &QNetworkReply::finished, [reply, reqID, mod, self]()
    {
        // 处理错误情况，并通过 SigHttpFinish 发送 error code
        if(reply->error() != QNetworkReply::NoError)
        {
            qDebug() << reply->errorString();

            emit self->SigHttpFinish("", reqID, mod, ErrorCode::ERR_NETWORK);
            reply->deleteLater();                       // 为程序的安全性，不立即删除，而是使用 deleteLater() 在下一次调用时删除

            return;
        }

        // 无错误则读取请求
        QString res = reply->readAll();

        // 发送 error code 为 SUCCESS 的信号
        emit self->SigHttpFinish(res, reqID, mod, ErrorCode::SUCCESS);
        reply->deleteLater();

        return;
    });

}


void HttpMgr::SlotHttpFinish(QString res, ReqID reqID, Module mod, ErrorCode errcode)
{
    if(mod == Module::REGISTER_MOD)
        emit SigRegModeFinish(res, reqID, errcode);
}
