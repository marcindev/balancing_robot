#ifndef LOGSDIALOG_H
#define LOGSDIALOG_H

#include <QDialog>

namespace Ui {
class LogsDialog;
}

class LogsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Status
    {
        enum class LogType { Logs, Postortem };
        enum class Board { Master, Slave };

        struct LogLevel
        {
            LogLevel() : info(false), debug(false), warning(false), error(false) { }

            bool info, debug, warning, error;
        };

        Status() : logType(LogType::Logs), board(Board::Slave) { }

        LogType logType;
        Board board;
        LogLevel logLevel;
    };

    explicit LogsDialog(QWidget *parent = 0);
    ~LogsDialog();

    Status getStatus() { return status; }

private slots:
    void handleAccepted();

private:
    void saveStatus();

    Status status;
    Ui::LogsDialog *ui;
};

#endif // LOGSDIALOG_H
