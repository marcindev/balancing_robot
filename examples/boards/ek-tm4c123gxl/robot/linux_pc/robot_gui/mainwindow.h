#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "connection.h"
#include "command.h"
#include "logsdialog.h"
#include "eventhandler.h"
#include "logswindow.h"
#include "settings.h"
#include "settingsdialog.h"
#include <QCloseEvent>
#include <QProgressDialog>
#include <QSlider>
#include <QSignalMapper>
#include <map>
#include <vector>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



protected:

    enum StatusLight
    {
        green,
        yellow,
        red
    };

    struct PidStatus
    {
        enum
        {
            proportional = 0,
            integral,
            derivative
        };

        PidStatus() : p(0.0), i(0.0), d(0.0) { }

        float p;
        float i;
        float d;
    };


    using EventRegisterer = Callbacker::EventRegisterer;

    void readSettings();
    void writeSettings();
    void applySettings();
    void initPidSliders();
    void connectToRobot();
    void disconnectToRobot();
    void setPidParam(int param, float value);
    void runCommand(const std::string& cmdString,
                    const std::vector<EventRegisterer>& registerers = std::vector<EventRegisterer>());

    float sliderToPidValue(int sliderVal);
    void closeEvent (QCloseEvent *event);

private slots:
    void handleConnButton();
    void handleDisconnButton();
    void handleActionLogs();
    void handleActionSettings();
    void handleConnected();
    void handleDisconnected();
    void handleGetLogsFinished();
    void handleGetLogsLineReceived();
    void handlePidSliderChanged(QWidget* widget);
    void handlePidApplyButton();
    void handleSetPidParamFinished();
    void handleSetPidParamTimeout();
    void handleSettingsDialogClosed(int result);
signals:


private:
    using CommandsMap = std::map<std::string, std::shared_ptr<Command>>;

    Settings settings;

    Ui::MainWindow *ui;
    LogsDialog* logsDialog;
    EventHandler* eventHandler;
    std::shared_ptr<QProgressDialog> logsProgressDialog;
    LogsWindow* logsWindow;
    QSignalMapper* slidersSigMapper;


    std::shared_ptr<Connection> connection;
    CommandsMap commands;
    std::map<StatusLight, QPixmap> connStatusLight;
    int pidParamCnt;
    unsigned pidParamTimeoutCnt;
    bool isConnected;
    unsigned logsCnt;
    PidStatus pidStatus;

};

#endif // MAINWINDOW_H
