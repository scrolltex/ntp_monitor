#ifndef APPCONTROLLER_HPP
#define APPCONTROLLER_HPP

#include <QObject>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QTimer>
#include <QGuiApplication>
#include <QProcess>
#include <QDebug>

#ifdef DEBUG
#include <QFile>
#endif

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
    QObject* pps;
    QObject* gps;

    QTimer statusUpdate;

signals:

public slots:
    void UpdateStatus();
    void RestartNTP();
};

#endif // APPCONTROLLER_HPP
