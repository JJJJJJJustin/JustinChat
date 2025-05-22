#ifndef HTTPMGR_H
#define HTTPMGR_H

#include "global.h"
#include "singleton.h"
#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QJsonDocument>


class HttpMgr : public QObject, public Singleton<HttpMgr>, public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT                                // 启用 QT 的信号和槽机制

public:
    ~HttpMgr();

    void PostHttpReq(QUrl url, QJsonObject jsonObj, ReqID reqID, Module mod);
private:
    friend class Singleton<HttpMgr>;
    HttpMgr();

public slots:
    void SlotHttpFinish(QString res, ReqID reqID, Module mod, ErrorCode errcode);
signals:
    void SigHttpFinish(QString res, ReqID reqID, Module mod, ErrorCode errcode);
    void SigRegModeFinish(QString res, ReqID reqID, ErrorCode errCode);

private:
    QNetworkAccessManager m_Manager;
};

#endif // HTTPMGR_H
