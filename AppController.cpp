#include "AppController.hpp"

AppController::AppController(QObject *parent) : QObject(parent)
{
    // Init qml engine
    component = new QQmlComponent(&engine, QUrl(QStringLiteral("qrc:/main.qml")));
    if(!component->isReady())
    {
        qCritical() << component->errorString();
        exit(-1);
    }

    root = component->create();

    // Find links to qml objects
    time = root->findChild<QObject*>("time");
    date = root->findChild<QObject*>("date");
    pps = root->findChild<QObject*>("pps");
    gps = root->findChild<QObject*>("gps");

    connect(&engine, SIGNAL(quit()), qApp, SLOT(quit()));
    connect(root, SIGNAL(restart_ntp()), this, SLOT(RestartNTP()));

    // Init timers
    timeUpdate = new QTimer(this);
    connect(timeUpdate, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timeUpdate->start(500);

    statusUpdate = new QTimer(this);
    connect(statusUpdate, SIGNAL(timeout()), this, SLOT(UpdateStatus()));
    statusUpdate->start(2000);

    // First update call
    UpdateTime();
    UpdateStatus();
}

AppController::~AppController()
{
    // Delete timers
    delete timeUpdate;
    delete statusUpdate;

    // Delete QML objects
    delete root;
    delete component;
}

void AppController::UpdateTime()
{
    time->setProperty("text", QTime::currentTime().toString());
    date->setProperty("text", QDate::currentDate().toString("dd.MM.yyyy"));
}

void AppController::UpdateStatus()
{
#ifdef Q_OS_LINUX
    QProcess process;
    QByteArray output;
    QString status_color;

    // Time status
    process.start("ntpq -pn | grep 127.127.22.0");
    process.waitForFinished();
    output = process.readAllStandardOutput();
    if(output.isEmpty() || output.contains("refused"))
        status_color = StatusColor::red;
    else
    {
        auto offset = abs(output.split(' ')[8].toFloat());
        status_color = offset < 0.1 ? StatusColor::green : StatusColor::yellow;
    }

    time->setProperty("color", status_color);

    // PPS status
    if(output.isEmpty() || output.contains("refused"))
        status_color = output.startsWith("o") ? StatusColor::green : StatusColor::yellow;
    else
        status_color = StatusColor::red;

    // GPS status
    process.start("ntpq -pn | grep 127.127.20.0");
    process.waitForFinished();
    output = process.readAllStandardOutput();
    if(!output.isEmpty() && !output.contains("refused"))
    {
        if(output.split(' ')[6] == "377")
            status_color = StatusColor::green;
        else
            status_color = StatusColor::yellow;
    }
    else status_color = StatusColor::red;

    gps->setProperty("color", status_color);
#else
    time->setProperty("color", StatusColor::red);
    pps->setProperty("color", StatusColor::red);
    gps->setProperty("color", StatusColor::red);
#endif
}

void AppController::RestartNTP()
{
    qDebug() << "Restarting ntp service...";
#ifdef Q_OS_LINUX
    QProcess proc;
    proc.start("sudo service ntp restart");
#else
    qWarning() << "Only works in linux!";
#endif
}
