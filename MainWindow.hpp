#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QProcess>
#include <QDebug>
#include <QApplication>

#if !defined(Q_OS_LINUX) || defined(DEBUG)
#include <QFile>
#endif

namespace StatusColor {
    const QString green = "#05952d";
    const QString yellow = "#ffd812";
    const QString red = "#b70808";
}

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void keyPressEvent(QKeyEvent *event);

private:
    QVBoxLayout *v_layout;
    QHBoxLayout *h_layout;
    QLabel *time;
    QLabel *date;
    QPushButton *pps_button;
    QPushButton *gps_button;
    QPushButton *rst_button;

    QTimer dateTimeUpdate;
    QTimer statusUpdate;

    float offset = 0;
    int sync_bits = 0;
    char pps_key = ' ';

    enum SyncVar
    {
        Time,
        PPS,
        GPS
    };

    enum SyncVarStatus
    {
        Undefined,
        Successfull,
        Warning,
        Error
    };

    QString SyncVarToString(SyncVar var);
    void ReportSyncStatus(SyncVar var, SyncVarStatus status);

    void CreateGUI();
    void UpdateButtonColor(QWidget* widget, QString color = "red");

public slots:
    void UpdateDateTime();
    void UpdateStatus();
    void RestartNTP();
};

#endif // MAINWINDOW_HPP
