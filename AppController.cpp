#include "AppController.hpp"

AppController::AppController(QObject *parent) : QObject(parent), component(&engine),
                                                statusUpdate(this)
{
    // Init qml engine
    component.loadUrl(QUrl(QStringLiteral("qrc:/main.qml")));
    if(!component.isReady())
    {
        qFatal(qUtf8Printable(component.errorString()));
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

    qInfo("NTP monitor started");

    // First update call
    UpdateStatus();
}

AppController::~AppController()
{
    // Delete QML objects
    delete root;
}

QString AppController::SyncVarToString(SyncVar var)
{
    switch(var)
    {
        case SyncVar::Time: return "Time";
        case SyncVar::PPS: return "PPS";
        case SyncVar::GPS: return "GPS";
        default: return "Unknown";
    }
}

void AppController::ReportSyncStatus(SyncVar var, SyncVarStatus status)
{
    static SyncVarStatus last_time = SyncVarStatus::Undefined;
    static SyncVarStatus last_pps = SyncVarStatus::Undefined;
    static SyncVarStatus last_gps = SyncVarStatus::Undefined;

    if((var == SyncVar::Time && status != last_time) ||
       (var == SyncVar::PPS && status != last_pps) ||
       (var == SyncVar::GPS && status != last_gps))
    {
        if(status == SyncVarStatus::Successfull)
            qInfo("%s: sync complete.", qUtf8Printable(SyncVarToString(var)));
        else if(status == SyncVarStatus::Error)
            qCritical("%s: sync failed.", qUtf8Printable(SyncVarToString(var)));
        else
        {
            if(var == SyncVar::Time)
                qWarning("Time: large offset (%4.3f)", offset);
            else if(var == SyncVar::PPS)
                qWarning("PPS: not used. Key is '%c'", pps_key);
            else if(var == SyncVar::GPS)
                qWarning("GPS: Error in one of the attempts (%s; %s)", qUtf8Printable(QString::number(sync_bits, 8)), qUtf8Printable(QString::number(sync_bits, 2)));
        }
    }

    switch(var)
    {
        case SyncVar::Time: last_time = status; break;
        case SyncVar::PPS: last_pps = status; break;
        case SyncVar::GPS: last_gps = status; break;
        default: break;
    }
}

void AppController::UpdateStatus()
{
    QString output;

#if defined(Q_OS_LINUX) && !defined(DEBUG)
    QProcess process;
    process.start("ntpq -pn");

    if(!process.waitForFinished(5000))
    {
        qCritical() << "ntpq start error: " << process.errorString();

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
        qCritical() << "ntpq output is empty!";

        time->setProperty("color", StatusColor::red);
        pps->setProperty("color", StatusColor::red);
        gps->setProperty("color", StatusColor::red);
        return;
    }

    if(output.contains("Connection refused"))
    {
        qCritical() << "ntpq error: connection refused";

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
        offset = qAbs(status.split(' ', QString::SkipEmptyParts)[8].toFloat());
        bool ok = offset < 0.1;
        status_color = ok ? StatusColor::green : StatusColor::yellow;
        ReportSyncStatus(SyncVar::Time, ok ? SyncVarStatus::Successfull : SyncVarStatus::Warning);
    }
    else
    {
        status_color = StatusColor::red;
        ReportSyncStatus(SyncVar::Time, SyncVarStatus::Error);
    }
    time->setProperty("color", status_color);

    // PPS status
    if(!status.isEmpty())
    {
        pps_key = status[0].toLatin1();
        bool ok = status.startsWith("o");
        status_color = ok ? StatusColor::green : StatusColor::yellow;
        ReportSyncStatus(SyncVar::PPS, ok ? SyncVarStatus::Successfull : SyncVarStatus::Warning);
    }
    else
    {
        status_color = StatusColor::red;
        ReportSyncStatus(SyncVar::PPS, SyncVarStatus::Error);
    }

    pps->setProperty("color", status_color);

    // GPS status
    status = get_line("127.127.20.0");
    if(!status.isEmpty())
    {
        bool ok = false;
        sync_bits = status.split(' ', QString::SkipEmptyParts)[6].toInt(&ok, 8);
        ok = sync_bits == 255;
        status_color = ok ? StatusColor::green : StatusColor::yellow;
        ReportSyncStatus(SyncVar::GPS, ok ? SyncVarStatus::Successfull : SyncVarStatus::Warning);
    }
    else
    {
        status_color = StatusColor::red;
        ReportSyncStatus(SyncVar::GPS, SyncVarStatus::Error);
    }

    gps->setProperty("color", status_color);
}

void AppController::RestartNTP()
{
#if defined(Q_OS_LINUX) && !defined(DEBUG)
    qInfo() << "Restarting ntp service";
    QProcess::execute("service ntp restart");
#else
    qDebug() << "RST pressed";
#endif
}
