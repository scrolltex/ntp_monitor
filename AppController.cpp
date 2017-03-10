#include "AppController.hpp"

AppController::AppController(QObject *parent) : QObject(parent), component(&engine),
                                                timeUpdate(this), statusUpdate(this)
{
    // Init qml engine
    component.loadUrl(QUrl(QStringLiteral("qrc:/main.qml")));
    if(!component.isReady())
    {
        qCritical() << component.errorString();
        exit(-1);
    }

    root = component.create();

    // Find links to qml objects
    time = root->findChild<QObject*>("time");
    date = root->findChild<QObject*>("date");
    pps = root->findChild<QObject*>("pps");
    gps = root->findChild<QObject*>("gps");

    // Connecting events
    connect(&engine, SIGNAL(quit()), qApp, SLOT(quit()));
    connect(root, SIGNAL(restart_ntp()), this, SLOT(RestartNTP()));

    // Init timers
    connect(&timeUpdate, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timeUpdate.start(250);

    connect(&statusUpdate, SIGNAL(timeout()), this, SLOT(UpdateStatus()));
    statusUpdate.start(2000);

    // First update call
    UpdateTime();
    UpdateStatus();
}

AppController::~AppController()
{
    // Delete QML objects
    delete root;
}

void AppController::UpdateTime()
{
    time->setProperty("text", QTime::currentTime().toString());
    date->setProperty("text", QDate::currentDate().toString("dd.MM.yyyy"));
}

void AppController::UpdateStatus()
{
#if defined(Q_OS_LINUX) || defined(DEBUG)
    QProcess process;
    process.setWorkingDirectory(qApp->applicationDirPath());
    process.start("ntpq -pn");
    process.waitForFinished(5000);

    if(process.error() != QProcess::ProcessError::UnknownError)
    {
        qWarning() << "ntpq start error: " << process.errorString();
        time->setProperty("color", StatusColor::red);
        pps->setProperty("color", StatusColor::red);
        gps->setProperty("color", StatusColor::red);
        return;
    }

    // reading output
    QString err_output = process.readAllStandardError();
    QString output = process.readAllStandardOutput();

#ifdef Q_OS_WIN
    // removing \r symbols
    err_output.remove('\r');
    output.remove('\r');
#endif

    if(output.isEmpty() || (output.isEmpty() && err_output.isEmpty()))
    {
        qWarning() << "ntpq output is empty!";
        time->setProperty("color", StatusColor::red);
        pps->setProperty("color", StatusColor::red);
        gps->setProperty("color", StatusColor::red);
        return;
    }

    if(err_output.contains("Connection refused") ||
           output.contains("Connection refused"))
    {
        qWarning() << "ntpq error: connection refused";
        time->setProperty("color", StatusColor::red);
        pps->setProperty("color", StatusColor::red);
        gps->setProperty("color", StatusColor::red);
        return;
    }

    // Split ntpq output
    QStringList lines = output.split('\n');
    // Removing ntpq table header
    lines.removeAll("     remote           refid      st t when poll reach   delay   offset  jitter");
    lines.removeAll("==============================================================================");

    // Custom grep
    auto grep = [lines](QString tofind) -> QString {
        foreach(QString str, lines)
        {
            if(str.contains(tofind))
                return str;
        }

        return "";
    };

    QString grepped;
    QString status_color;

    // Time status
    grepped = grep("127.127.22.0");
    if(!grepped.isEmpty())
    {
        auto offset = abs(grepped.split(' ', QString::SkipEmptyParts)[8].toFloat());
        status_color = offset < 0.1 ? StatusColor::green : StatusColor::yellow;
    }
    else
        status_color = StatusColor::red;

    time->setProperty("color", status_color);

    // PPS status
    if(!grepped.isEmpty())
        status_color = grepped.startsWith("o") ? StatusColor::green : StatusColor::yellow;
    else
        status_color = StatusColor::red;

    pps->setProperty("color", status_color);

    // GPS status
    grepped = grep("127.127.20.0");
    if(!grepped.isEmpty())
    {
        if(grepped.split(' ', QString::SkipEmptyParts)[6] == "377")
            status_color = StatusColor::green;
        else
            status_color = StatusColor::yellow;
    }
    else
        status_color = StatusColor::red;

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
    QProccess::exec("sudo service ntp restart");
#else
    qWarning() << "Only works in linux!";
#endif
}
