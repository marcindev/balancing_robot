#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(Settings &_settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    settings(_settings),
    tempSettings(settings),
    prevPidIndex(Settings::Pid::PidParam::proportional)
{
    ui->setupUi(this);
    ui->sliderMinDoubleSpinBox->setRange(0.0, 10000.0);
    ui->sliderMaxDoubleSpinBox->setRange(0.0, 10000.0);
    ui->sliderIntervalDoubleSpinBox->setRange(0.0, 1000.0);
    ui->pidComboBox->setCurrentIndex(Settings::Pid::proportional);
    readPidSettings(Settings::Pid::proportional);

    connect(ui->pidComboBox, SIGNAL (currentIndexChanged(int)), this, SLOT (handlePidComboSelChanged(int)));
    connect(ui->buttonBox, SIGNAL (accepted()), this, SLOT (handleAccpeted()));
    connect(ui->buttonBox, SIGNAL (rejected()), this, SLOT (handleRejected()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    ui->pidComboBox->setCurrentIndex(Settings::Pid::proportional);
    readPidSettings(Settings::Pid::proportional);
}


void SettingsDialog::saveSettings()
{
    saveTempPidSettings(static_cast<Settings::Pid::PidParam>(ui->pidComboBox->currentIndex()));

    settings = tempSettings;
}

void SettingsDialog::handlePidComboSelChanged(int index)
{
    Settings::Pid::PidParam pidIndex = static_cast<Settings::Pid::PidParam>(index);

    if(pidIndex == prevPidIndex)
        return;

    saveTempPidSettings(prevPidIndex);

    readTempPidSettings(pidIndex);

    prevPidIndex = pidIndex;
}

void SettingsDialog::handleAccpeted()
{
    saveSettings();
}

void SettingsDialog::handleRejected()
{
    tempSettings = settings;
}

void SettingsDialog::readPidSettings(Settings::Pid::PidParam pid)
{
    ui->sliderMinDoubleSpinBox->setValue(settings.pid.pidSettings.at(pid).min);
    ui->sliderMaxDoubleSpinBox->setValue(settings.pid.pidSettings.at(pid).max);
    ui->sliderIntervalDoubleSpinBox->setValue(settings.pid.pidSettings.at(pid).interval);
}

void SettingsDialog::readTempPidSettings(Settings::Pid::PidParam pid)
{
    ui->sliderMinDoubleSpinBox->setValue(tempSettings.pid.pidSettings.at(pid).min);
    ui->sliderMaxDoubleSpinBox->setValue(tempSettings.pid.pidSettings.at(pid).max);
    ui->sliderIntervalDoubleSpinBox->setValue(tempSettings.pid.pidSettings.at(pid).interval);
}

void SettingsDialog::saveTempPidSettings(Settings::Pid::PidParam pid)
{
    tempSettings.pid.pidSettings.at(pid).min = ui->sliderMinDoubleSpinBox->value();
    tempSettings.pid.pidSettings.at(pid).max = ui->sliderMaxDoubleSpinBox->value();
    tempSettings.pid.pidSettings.at(pid).interval = ui->sliderIntervalDoubleSpinBox->value();

}
