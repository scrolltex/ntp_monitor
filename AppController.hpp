#ifndef APPCONTROLLER_HPP
#define APPCONTROLLER_HPP

#include <QObject>
#include <QDebug>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QGuiApplication>
#include <QProcess>

namespace StatusColor {
    const QString green = "#05952d";
    const QString yellow = "#ffd812";
    const QString red = "#b70808";
}

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = 0);
    ~AppController();

private:
    QQmlEngine engine;
    QQmlComponent component;
    QObject* root;

    QObject* time;
    QObject* date;
    QObject* pps;
    QObject* gps;

    QTimer timeUpdate;
    QTimer statusUpdate;

signals:

public slots:
    void UpdateTime();
    void UpdateStatus();
    void RestartNTP();
};

#endif // APPCONTROLLER_HPP
