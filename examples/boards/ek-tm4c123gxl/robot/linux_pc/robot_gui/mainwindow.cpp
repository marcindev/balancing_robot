#include <math.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logsdialog.h"
#include "logswindow.h"
#include "commandFactory.h"
#include "getLogsCommand.h"
#include "setPidParamCmd.h"
#include <string>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <QMessageBox>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(),
    logsDialog(new LogsDialog(this)),
    eventHandler(new EventHandler(this)),
    logsProgressDialog(0),
    logsWindow(new LogsWindow(this)),
    slidersSigMapper(new QSignalMapper(this)),
//    settingsDialog(new SettingsDialog(settings, this)),
    pidParamCnt(0),
    pidParamTimeoutCnt(0),
    isConnected(false),
    logsCnt(0)
{
    ui->setupUi(this);

    initPidSliders();

    readSettings();
    applySettings();

    connStatusLight[green] = QPixmap(":/img/green_light.png");
    connStatusLight[yellow] = QPixmap(":/img/yellow_light.png");
    connStatusLight[red] = QPixmap(":/img/red_light.png");


    logsWindow->setWindowModality(Qt::NonModal);

    ui->connStatusLight->setScaledContents(true);
    ui->connStatusLight->setPixmap(connStatusLight[red]);
//    ui->connStatusLight->setMask(connStatusLight[red].mask());

    connect(ui->connectButton, SIGNAL (released()), this, SLOT (handleConnButton()));
    connect(ui->ipLineEdit, SIGNAL (returnPressed()), ui->connectButton, SIGNAL (released()));
    connect(ui->disconnectButton, SIGNAL (released()), this, SLOT (handleDisconnButton()));
    connect(ui->actionLogs, SIGNAL (triggered()), this, SLOT (handleActionLogs()));
    connect(ui->pidApplyButton, SIGNAL (released()), this, SLOT (handlePidApplyButton()));
    connect(ui->actionSettings, SIGNAL (triggered()), this, SLOT (handleActionSettings()));
//    connect(settingsDialog, SIGNAL (finished(int)), this, SLOT (handleSettingsDialogClosed(int)));


    connect(eventHandler, SIGNAL (connected()), this, SLOT (handleConnected()));
    connect(eventHandler, SIGNAL (disconnected()), this, SLOT (handleDisconnected()));
    connect(eventHandler, SIGNAL (getLogsFinished()), this, SLOT (handleGetLogsFinished()));
    connect(eventHandler, SIGNAL (getLogsLineReceived()), this, SLOT (handleGetLogsLineReceived()));
    connect(eventHandler, SIGNAL (setPidParamFinished()), this, SLOT (handleSetPidParamFinished()));
    connect(eventHandler, SIGNAL (setPidParamTimeout()), this, SLOT (handleSetPidParamTimeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{

    if(connection.get())
    {
        connection->disconnect();
        connection->wait();
    }

    writeSettings();
    event->accept();
}

void MainWindow::readSettings()
{
    QSettings qsettings("MarcinSoft", "RobotGui");

    qsettings.beginGroup("connection");
    ui->ipLineEdit->setText(qsettings.value("ip").toString());
    qsettings.endGroup();

    qsettings.beginGroup("settings/pid");
    settings.pid.pidSettings.at(Settings::Pid::proportional).min = qsettings.value("proMin").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::proportional).max = qsettings.value("proMax").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::proportional).interval = qsettings.value("proInterval").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::integral).min = qsettings.value("intMin").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::integral).max = qsettings.value("intMax").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::integral).interval = qsettings.value("intInterval").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::derivative).min = qsettings.value("derMin").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::derivative).max = qsettings.value("derMax").toFloat();
    settings.pid.pidSettings.at(Settings::Pid::derivative).interval = qsettings.value("derInterval").toFloat();
    qsettings.endGroup();

}

void MainWindow::writeSettings()
{
    QSettings qsettings("MarcinSoft", "RobotGui");

    qsettings.beginGroup("connection");
    qsettings.setValue("ip", ui->ipLineEdit->text());
    qsettings.endGroup();

    qsettings.beginGroup("settings/pid");
    qsettings.setValue("proMin", settings.pid.pidSettings.at(Settings::Pid::proportional).min);
    qsettings.setValue("proMax", settings.pid.pidSettings.at(Settings::Pid::proportional).max);
    qsettings.setValue("proInterval", settings.pid.pidSettings.at(Settings::Pid::proportional).interval);
    qsettings.setValue("intMin", settings.pid.pidSettings.at(Settings::Pid::integral).min);
    qsettings.setValue("intMax", settings.pid.pidSettings.at(Settings::Pid::integral).max);
    qsettings.setValue("intInterval", settings.pid.pidSettings.at(Settings::Pid::integral).interval);
    qsettings.setValue("derMin", settings.pid.pidSettings.at(Settings::Pid::derivative).min);
    qsettings.setValue("derMax", settings.pid.pidSettings.at(Settings::Pid::derivative).max);
    qsettings.setValue("derInterval", settings.pid.pidSettings.at(Settings::Pid::derivative).interval);
    qsettings.endGroup();

}

void MainWindow::applySettings()
{
    ui->proportionalSlider->setMinimum(static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::proportional).min * 1000.0)));
    ui->integralSlider->setMinimum(static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::integral).min * 1000.0)));
    ui->derivativeSlider->setMinimum(static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::derivative).min * 1000.0)));

    ui->proportionalSlider->setMaximum(static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::proportional).max * 1000.0)));
    ui->integralSlider->setMaximum(static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::integral).max * 1000.0)));
    ui->derivativeSlider->setMaximum(static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::derivative).max * 1000.0)));

    int interval = static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::proportional).interval * 1000.0));
    ui->proportionalSlider->setTickInterval(interval);
    ui->proportionalSlider->setSingleStep(interval);

    interval = static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::integral).interval * 1000.0));
    ui->integralSlider->setTickInterval(interval);
    ui->integralSlider->setSingleStep(interval);
    interval = static_cast<int>(nearbyint(settings.pid.pidSettings.at(Settings::Pid::derivative).interval * 1000.0));
    ui->derivativeSlider->setTickInterval(interval);
    ui->derivativeSlider->setSingleStep(interval);


    ui->proportionalLabel->setText(QString::number(sliderToPidValue(ui->proportionalSlider->value())));
    ui->integralLabel->setText(QString::number(sliderToPidValue(ui->integralSlider->value())));
    ui->derivativeLabel->setText(QString::number(sliderToPidValue(ui->derivativeSlider->value())));
}

void MainWindow::initPidSliders()
{
    ui->proportionalSlider->setTracking(true);
    ui->integralSlider->setTracking(true);
    ui->derivativeSlider->setTracking(true);

    ui->proportionalSlider->setMinimum(0);
    ui->integralSlider->setMinimum(0);
    ui->derivativeSlider->setMinimum(0);

    ui->proportionalSlider->setMaximum(5000);
    ui->integralSlider->setMaximum(5000);
    ui->derivativeSlider->setMaximum(5000);

    connect(ui->proportionalSlider, SIGNAL(valueChanged(int)), slidersSigMapper, SLOT(map()));
    slidersSigMapper->setMapping(ui->proportionalSlider, ui->proportionalLabel);

    connect(ui->integralSlider, SIGNAL(valueChanged(int)), slidersSigMapper, SLOT(map()));
    slidersSigMapper->setMapping(ui->integralSlider, ui->integralLabel);

    connect(ui->derivativeSlider, SIGNAL(valueChanged(int)), slidersSigMapper, SLOT(map()));
    slidersSigMapper->setMapping(ui->derivativeSlider, ui->derivativeLabel);

    connect(slidersSigMapper, SIGNAL(mapped(QWidget*)),this, SLOT(handlePidSliderChanged(QWidget*)));
}

void MainWindow::handlePidSliderChanged(QWidget* widget)
{
    QSlider* slider = qobject_cast<QSlider*>(slidersSigMapper->mapping(widget));
    QLabel* label = qobject_cast<QLabel*>(widget);

    if(!slider)
        return;

    int tickInterval = slider->tickInterval();
    int slidValue = slider->value();
    int remainder = slidValue % tickInterval;

    if(remainder)
    {
        if(remainder > (tickInterval / 2))
        {
            slider->setSliderPosition(slidValue + (tickInterval - remainder));
        }
        else
        {
            slider->setSliderPosition(slidValue - remainder);
        }
    }

    pidStatus.p = sliderToPidValue(ui->proportionalSlider->value());
    pidStatus.i = sliderToPidValue(ui->integralSlider->value());
    pidStatus.d = sliderToPidValue(ui->derivativeSlider->value());

    ui->pidApplyButton->setEnabled(true);

    if(label)
    {
        label->setText(QString::number(sliderToPidValue(slider->value())));
    }
}

void MainWindow::handleSetPidParamFinished()
{
    static const int MAX_PID_PARAM_CNT = 2;

    ++pidParamCnt;

    if(pidParamCnt > MAX_PID_PARAM_CNT)
        return;

    float pidValue = 0.0f;

    switch(pidParamCnt)
    {
    case PidStatus::integral:
        pidValue = pidStatus.i;
        break;
    case PidStatus::derivative:
        pidValue = pidStatus.d;
        break;
    default:
        break;
    }

    setPidParam(pidParamCnt, pidValue);
}

void MainWindow::handleSetPidParamTimeout()
{
    if(pidParamTimeoutCnt++)
        return;

    QMessageBox msgBox;

    msgBox.warning(0, "SetPidParam timeout", "Couldn't set pid parameter");
    msgBox.setFixedSize(500,200);
}

void MainWindow::handlePidApplyButton()
{
    pidParamCnt = 0;
    pidParamTimeoutCnt = 0;

    setPidParam(0, pidStatus.p);
    ui->pidApplyButton->setEnabled(false);
}

void MainWindow::handleConnButton()
{
    connectToRobot();
}

void MainWindow::handleDisconnButton()
{
    disconnectToRobot();
}

void MainWindow::handleActionLogs()
{
    LogsDialog::Status status;

    logsCnt = 0;

    if(logsDialog->exec() == QDialog::Accepted)
    {
        status = logsDialog->getStatus();

        std::string cmdStr;

        cmdStr = (status.logType == LogsDialog::Status::LogType::Logs) ? "getLogs" : "getPostmortem";
        cmdStr += status.board == (LogsDialog::Status::Board::Master) ? " 1" : " 0";

        std::vector<EventRegisterer> registerers;
        registerers.push_back(EventRegisterer(GetLogsCommand::Event::finished,
                                              &EventHandler::getLogsFinishedCallback,
                                              *eventHandler
                                              ));

        registerers.push_back(EventRegisterer(GetLogsCommand::Event::log_line_received,
                                              &EventHandler::getLogsLineReceivedCallback,
                                              *eventHandler
                                              ));


        runCommand(cmdStr, registerers);

    }

}

void MainWindow::handleActionSettings()
{
    SettingsDialog settingsDialog(settings);

    if(settingsDialog.exec() == QDialog::Accepted)
    {
        applySettings();
    }
}

void MainWindow::handleSettingsDialogClosed(int result)
{
    if(result == QDialog::Accepted)
    {
        applySettings();
    }
}

void MainWindow::connectToRobot()
{
    if(isConnected)
        return;

    ui->connStatusLight->setPixmap(connStatusLight[yellow]);

    QString ipStr = ui->ipLineEdit->text();

    std::string ipUtf8 = ipStr.toUtf8().constData();

    if(!ipUtf8.empty())
        connection.reset(new Connection(ipUtf8));
    else
        connection.reset(new Connection());

    connection->start();

    connection->registerCallback(Connection::Event::connected,
                                 &EventHandler::connectedCallback,
                                 *eventHandler);

    connection->registerCallback(Connection::Event::disconnected,
                                 &EventHandler::disconnectedCallback,
                                 *eventHandler);
}

void MainWindow::runCommand(const std::string& cmdString, const std::vector<EventRegisterer>& registerers)
{
    if(!isConnected)
    {
        QMessageBox msgBox;

        msgBox.warning(0, "Not connected", "Not connected to robot");
        msgBox.setFixedSize(500,200);

        return;
    }


    std::string cmdName;
    std::vector<std::string> args;
    std::istringstream iss(cmdString);
    std::istream_iterator<std::string> issIter(iss);

    cmdName = *issIter++;


    std::copy(issIter, std::istream_iterator<std::string>(), std::back_inserter(args));

    commands[cmdName] = CommandFactory(connection).createCommand(cmdName, args);

    for(auto const& reg : registerers)
    {
        reg.registerToCallbacker(*commands.at(cmdName));
    }

    commands.at(cmdName)->execute();
}

void MainWindow::disconnectToRobot()
{
    if(!isConnected)
        return;

    connection->disconnect();
    connection->wait();
    isConnected = false;
    ui->connStatusLight->setPixmap(connStatusLight[red]);
}

float MainWindow::sliderToPidValue(int sliderVal)
{
    return sliderVal / 1000.0f;
}

void MainWindow::setPidParam(int param, float value)
{
    std::string cmdStr;
    std::stringstream ss;

    std::string strParam, strValue;

    ss << param;
    ss >> strParam;

    ss.str("");
    ss.clear();

    ss << value;
    ss >> strValue;

    cmdStr = "setPidParam " + strParam + " " + strValue;

    std::vector<EventRegisterer> registerers;
    registerers.push_back(EventRegisterer(SetPidParamCmd::Event::finished,
                                          &EventHandler::setPidParamFinishedCallback,
                                          *eventHandler
                                          ));


    registerers.push_back(EventRegisterer(SetPidParamCmd::Event::response_timeout,
                                          &EventHandler::setPidParamTimeoutCallback,
                                          *eventHandler
                                          ));



    runCommand(cmdStr, registerers);
}



void MainWindow::handleConnected()
{
    isConnected = true;
    ui->connStatusLight->setPixmap(connStatusLight[green]);
}

void MainWindow::handleDisconnected()
{
    isConnected = false;
    ui->connStatusLight->setPixmap(connStatusLight[red]);
}

void MainWindow::handleGetLogsFinished()
{
//    logsProgressDialog->hide();

    std::string logText(std::dynamic_pointer_cast<GetLogsCommand>(commands.at("getLogs"))->getLogText());

    logsWindow->setLogsText(logText);
    logsWindow->show();

}

void MainWindow::handleGetLogsLineReceived()
{
    ++logsCnt;

    unsigned lineNum = std::dynamic_pointer_cast<GetLogsCommand>(commands.at("getLogs"))->getLogNum();

    if(logsCnt == 1)
    {
//        logsProgressDialog = std::make_shared<QProgressDialog>(new QProgressDialog("Getting logs...", "Cancel", 0, lineNum, this));
//        logsProgressDialog->setWindowModality(Qt::ApplicationModal);

    }

//    logsProgressDialog->setValue(logsCnt);

}

