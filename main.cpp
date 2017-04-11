#include <QGuiApplication>
#include <iostream>
#include <QFile>
#include <QDateTime>

#include "AppController.hpp"

QString MsgTypeToStr(QtMsgType type)
{
    switch(type)
    {
        case QtMsgType::QtDebugMsg: return "Debug";
        case QtMsgType::QtInfoMsg: return "Info";
        case QtMsgType::QtWarningMsg: return "Warning";
        case QtMsgType::QtCriticalMsg: return "Critical";
        case QtMsgType::QtFatalMsg: return "Fatal";
        default: return "Unknown";
    }
}

void CustomMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    auto now = QDateTime::currentDateTime().toString("dd.MM.yy HH:mm:ss.zzz");
    QString str = QString("[%1] [%2] %3").arg(now, MsgTypeToStr(type), msg);

#ifdef DEBUG
    std::cout << qUtf8Printable(str) << std::endl;
#endif

#ifdef Q_OS_LINUX
    QFile file("/var/log/ntp_monitor.log");
#else
    QFile file("ntp_monitor.log");
#endif

    if(!file.open(QIODevice::Append | QIODevice::Text))
    {
        std::cerr << "Can`t open log file!" << std::endl;
        return;
    }

    QTextStream out(&file);
    out << str << endl;
    out.flush();
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qInstallMessageHandler(CustomMessageHandler);

    AppController appc;
    Q_UNUSED(appc);

    return app.exec();
}
