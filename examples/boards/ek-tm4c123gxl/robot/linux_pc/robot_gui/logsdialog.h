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
    explicit LogsDialog(QWidget *parent = 0);
    ~LogsDialog();

private:
    Ui::LogsDialog *ui;
};

#endif // LOGSDIALOG_H
