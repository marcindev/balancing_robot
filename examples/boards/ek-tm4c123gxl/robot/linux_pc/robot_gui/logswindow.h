#ifndef LOGSWINDOW_H
#define LOGSWINDOW_H

#include <QMainWindow>
#include <string>

namespace Ui {
class LogsWindow;
}

class LogsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LogsWindow(QWidget *parent = 0);
    ~LogsWindow();

    void setLogsText(const std::string& logsText);

private:
    Ui::LogsWindow *ui;
};

#endif // LOGSWINDOW_H
