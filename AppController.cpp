#include "AppController.hpp"

AppController::AppController(QObject *parent) : QObject(parent), component(&engine),
                                                statusUpdate(this)
{
    // Init qml engine
    component.loadUrl(QUrl(QStringLiteral("qrc:/main.qml")));
    if(!component.isReady())
    {
        qCritical() << component.errorString();
        exit(-1);
    }

    root = component.create();

#ifdef DEBUG
    // Run in windowed mode when debug
    QMetaObject::invokeMethod(root, "toggle_fullscreen");
#endif

    // Find links to qml objects
    time = root->findChild<QObject*>("time");
    pps = root->findChild<QObject*>("pps");
    gps = root->findChild<QObject*>("gps");

    // Connecting events
    connect(&engine, SIGNAL(quit()), qApp, SLOT(quit()));
    connect(root, SIGNAL(restart_ntp()), this, SLOT(RestartNTP()));

    // Init timers
    connect(&statusUpdate, SIGNAL(timeout()), this, SLOT(UpdateStatus()));
    statusUpdate.start(2000);

    // First update call
    UpdateStatus();
}

AppController::~AppController()
{
    // Delete QML objects
    delete root;
}

void AppController::UpdateStatus()
{
    QString output;

#if defined(Q_OS_LINUX) && !defined(DEBUG)
    QProcess process;
    process.start("ntpq -pn");

    if(!process.waitForFinished(5000))
    {
        qWarning() << "ntpq start error: " << process.errorString();

        time->setProperty("color", StatusColor::red);
        pps->setProperty("color", StatusColor::red);
        gps->setProperty("color", StatusColor::red);
        return;
    }

    // reading output
    output = process.readAll();
#else
    QFile file(qApp->applicationDirPath() + "/ntpq_output.txt");
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        output = file.readAll();
        file.close();
    }
#endif

    if(output.isEmpty())
    {
        qWarning() << "ntpq output is empty!";

        time->setProperty("color", StatusColor::red);
        pps->setProperty("color", StatusColor::red);
        gps->setProperty("color", StatusColor::red);
        return;
    }

    if(output.contains("Connection refused"))
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
    lines.removeFirst(); //     remote           refid      st t when poll reach   delay   offset  jitter
    lines.removeFirst(); //==============================================================================

    // Custom grep
    auto get_line = [lines](QString tofind) -> QString {
        foreach(QString str, lines)
        {
            if(str.contains(tofind))
                return str;
        }

        return "";
    };

    QString status;
    QString status_color;

    // Time status
    status = get_line("127.127.22.0");
    if(!status.isEmpty())
    {
        float offset = abs(status.split(' ', QString::SkipEmptyParts)[8].toFloat());
        status_color = offset < 0.1 ? StatusColor::green : StatusColor::yellow;
    }
    else
        status_color = StatusColor::red;

    time->setProperty("color", status_color);

    // PPS status
    if(!status.isEmpty())
        status_color = status.startsWith("o") ? StatusColor::green : StatusColor::yellow;
    else
        status_color = StatusColor::red;

    pps->setProperty("color", status_color);

    // GPS status
    status = get_line("127.127.20.0");
    if(!status.isEmpty())
    {
        if(status.split(' ', QString::SkipEmptyParts)[6] == "377")
            status_color = StatusColor::green;
        else
            status_color = StatusColor::yellow;
    }
    else
        status_color = StatusColor::red;

    gps->setProperty("color", status_color);
}

void AppController::RestartNTP()
{
#if defined(Q_OS_LINUX) && !defined(DEBUG)
    QProcess::execute("service ntp restart");
#else
    qDebug() << "RST pressed";
#endif
}
