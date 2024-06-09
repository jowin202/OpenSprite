#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QKeyEvent>
#include <QColor>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class options;

class SettingsDialog : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsDialog(options *opt, QWidget *parent = nullptr);
    ~SettingsDialog();

    void keyPressEvent(QKeyEvent *e) {
        if(e->key() == Qt::Key_Escape)
            this->on_button_cancel_clicked();
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_ok_clicked();
    }

private slots:
    void on_button_cancel_clicked();
    void on_button_ok_clicked();


    void on_button_spacing_color_clicked();

    void on_button_selection_color_clicked();

    void on_button_defaults_clicked();

signals:
    void finished();


private:
    Ui::SettingsDialog *ui;
    options *opt;

    QColor bgcolor;
    QColor select_color;
    QSettings settings;
};

#endif // SETTINGSDIALOG_H
