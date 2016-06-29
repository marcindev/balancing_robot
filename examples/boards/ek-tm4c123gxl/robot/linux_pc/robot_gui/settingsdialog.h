#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <map>
#include "settings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit SettingsDialog(Settings& settings, QWidget *parent = 0);
    ~SettingsDialog();

protected:
    void showEvent( QShowEvent* event );
    void saveSettings();
    void readPidSettings(Settings::Pid::PidParam pid);
    void readTempPidSettings(Settings::Pid::PidParam pid);
    void saveTempPidSettings(Settings::Pid::PidParam pid);

private slots:
    void handlePidComboSelChanged(int index);
    void handleAccpeted();
    void handleRejected();

private:


    Ui::SettingsDialog *ui;
    Settings& settings;
    Settings tempSettings;
    Settings::Pid::PidParam prevPidIndex;
};

#endif // SETTINGSDIALOG_H
