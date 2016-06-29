#ifndef LOGSWINDOW_H
#define LOGSWINDOW_H

#include <QMainWindow>

namespace Ui {
class LogsWindow;
}

class LogsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LogsWindow(QWidget *parent = 0);
    ~LogsWindow();

private:
    Ui::LogsWindow *ui;
};

#endif // LOGSWINDOW_H
