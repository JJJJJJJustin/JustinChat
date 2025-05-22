#include "global.h"
#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 读取资源文件，初始化程序的
    QFile qss(":/resources/styles/style.qss");
    if(qss.open(QFile::ReadOnly))
    {
        qDebug("Qss open success.");
        QString str = QLatin1String(qss.readAll());
        a.setStyleSheet(str);
        qss.close();
    }
    else
    {
        qDebug("Open Failed");
    }

    // 读取 config.ini 并设置 gateUrlPrefix
    QString appPath = QCoreApplication::applicationDirPath();
    QString fileName = "config.ini";
    QString configPath = appPath + fileName;

    QSettings settings(configPath, QSettings::IniFormat);
    QString gateHost = settings.value("GateServer/host").toString();
    QString gatePort = settings.value("GateServer/port").toString();
    gateUrlPrefix = "http://" + gateHost + ":" + gatePort;

    MainWindow w;
    w.show();

    return a.exec();
}
