#include "logsdialog.h"
#include "ui_logsdialog.h"

LogsDialog::LogsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogsDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL ( accepted() ), this, SLOT (handleAccepted()));
}

LogsDialog::~LogsDialog()
{
    delete ui;
}

void LogsDialog::handleAccepted()
{
    saveStatus();
}


void LogsDialog::saveStatus()
{
    status.logType = ui->radioButtonLogs->isChecked() ? Status::LogType::Logs : Status::LogType::Postortem;
    status.board = ui->radioButtonMaster->isChecked() ? Status::Board::Master : Status::Board::Slave;
    status.logLevel.info = ui->checkBoxInfo->isChecked();
    status.logLevel.debug = ui->checkBoxDebug->isChecked();
    status.logLevel.warning = ui->checkBoxWarning->isChecked();
    status.logLevel.error = ui->checkBoxError->isChecked();

}
