#include "logswindow.h"
#include "ui_logswindow.h"

LogsWindow::LogsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LogsWindow)
{
    ui->setupUi(this);
}

LogsWindow::~LogsWindow()
{
    delete ui;
}


void LogsWindow::setLogsText(const std::string& logsText)
{
    ui->plainTextEdit->document()->setPlainText(QString::fromStdString(logsText));
}
