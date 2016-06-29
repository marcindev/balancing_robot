#include "logsdialog.h"
#include "ui_logsdialog.h"

LogsDialog::LogsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogsDialog)
{
    ui->setupUi(this);
}

LogsDialog::~LogsDialog()
{
    delete ui;
}
