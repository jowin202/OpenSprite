#ifndef IODIALOG_H
#define IODIALOG_H

#include <QWidget>

namespace Ui {
class IODialog;
}

class IODialog : public QWidget
{
    Q_OBJECT

public:
    explicit IODialog(QWidget *parent = nullptr);
    ~IODialog();

private:
    Ui::IODialog *ui;
};

#endif // IODIALOG_H
