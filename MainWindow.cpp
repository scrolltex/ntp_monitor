#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), dateTimeUpdate(this), statusUpdate(this)
{
    // Init gui
    CreateGUI();
    setWindowTitle("ntp monitor");
    resize(320, 240);

#ifndef DEBUG
    this->showFullScreen();
#endif

    // Connecting timers
    connect(&dateTimeUpdate, SIGNAL(timeout()), this, SLOT(UpdateDateTime()));
    dateTimeUpdate.start(250);

    connect(&statusUpdate, SIGNAL(timeout()), this, SLOT(UpdateStatus()));
    statusUpdate.start(2000);

    qInfo("NTP monitor started");

    // First update
    UpdateDateTime();
    UpdateStatus();
}

MainWindow::~MainWindow()
{

}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
        this->close();
}

void MainWindow::UpdateButtonColor(QWidget* widget, QString color)
{
    const QString style = QString("color: #000000; "
                          "background-color: %1; "
                          "border: 1px solid; "
                          "border-radius: 2px; "
                          "font-size: 45px; ").arg(color);
    widget->setStyleSheet(style);
}

void MainWindow::CreateGUI()
{
    this->setStyleSheet("background-color: #000000");

    time = new QLabel("00:00:00");
    time->setFont(QFont("Arial", 45));
    time->setStyleSheet("color: #ffffff;");

    date = new QLabel("00.00.0000");
    date->setFont(QFont("Arial", 30));
    date->setStyleSheet("color: #ffffff;");

    v_layout = new QVBoxLayout();
    v_layout->addWidget(time, 0, Qt::AlignHCenter);
    v_layout->addWidget(date, 0, Qt::AlignHCenter);

    pps_button = new QPushButton("PPS");
    UpdateButtonColor(pps_button);

    gps_button = new QPushButton("GPS");
    UpdateButtonColor(gps_button);

    rst_button = new QPushButton("RST");
    rst_button->setFocus();
    UpdateButtonColor(rst_button);
    connect(rst_button, SIGNAL(pressed()), this, SLOT(RestartNTP()));

    h_layout = new QHBoxLayout();
    h_layout->addWidget(pps_button);
    h_layout->addWidget(gps_button);
    h_layout->addWidget(rst_button);

    v_layout->addLayout(h_layout);
    setLayout(v_layout);
}

QString MainWindow::SyncVarToString(SyncVar var)
{
    switch(var)
    {
        case SyncVar::Time: return "Time";
        case SyncVar::PPS: return "PPS";
        case SyncVar::GPS: return "GPS";
        default: return "Unknown";
    }
}

void MainWindow::ReportSyncStatus(SyncVar var, SyncVarStatus status)
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

void MainWindow::UpdateDateTime()
{
    auto now = QDateTime::currentDateTime();
    time->setText(now.toString("HH:mm:ss"));
    date->setText(now.toString("dd.MM.yyyy"));
}

void MainWindow::UpdateStatus()
{
    QString output;

#if defined(Q_OS_LINUX) && !defined(DEBUG)
    QProcess process;
    process.start("ntpq -pn");

    if(!process.waitForFinished(5000))
    {
        qCritical() << "ntpq start error: " << process.errorString();

        time->setStyleSheet(QString("color : %1;").arg(StatusColor::red));
        UpdateButtonColor(pps_button, StatusColor::red);
        UpdateButtonColor(gps_button, StatusColor::red);
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

        time->setStyleSheet(QString("color : %1;").arg(StatusColor::red));
        UpdateButtonColor(pps_button, StatusColor::red);
        UpdateButtonColor(gps_button, StatusColor::red);
        return;
    }

    if(output.contains("Connection refused"))
    {
        qCritical() << "ntpq error: connection refused";

        time->setStyleSheet(QString("color : %1;").arg(StatusColor::red));
        UpdateButtonColor(pps_button, StatusColor::red);
        UpdateButtonColor(gps_button, StatusColor::red);
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

    time->setStyleSheet(QString("color : %1;").arg(status_color));

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

    UpdateButtonColor(pps_button, status_color);

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

    UpdateButtonColor(gps_button, status_color);
}

void MainWindow::RestartNTP()
{
#if defined(Q_OS_LINUX) && !defined(DEBUG)
    qInfo() << "Restarting ntp service";
    QProcess::execute("service ntp restart");
#else
    qDebug() << "RST pressed";
#endif
}
