#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QWidget>
#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QKeyEvent>

class options;

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ExportDialog(options *opt, QWidget *parent = nullptr);
    ~ExportDialog();


    void keyPressEvent(QKeyEvent *e) {
        if(e->key() == Qt::Key_Escape)
            this->on_button_close_clicked();
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_export_clicked();
    }

signals:
    void statusmsg(QString);

private slots:
    void on_button_export_clicked();
    void on_button_close_clicked();

private:
    Ui::ExportDialog *ui;
    options *opt;
};

#endif // EXPORTDIALOG_H
