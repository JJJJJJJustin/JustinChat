#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
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

    MainWindow w;
    w.show();

    return a.exec();
}
