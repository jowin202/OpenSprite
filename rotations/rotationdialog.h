#ifndef ROTATIONDIALOG_H
#define ROTATIONDIALOG_H

#include <QWidget>
#include <QKeyEvent>
#include <QList>

#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class RotationDialog;
}

struct options;

class RotationDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RotationDialog(options *opt, QWidget *parent = nullptr);
    ~RotationDialog();

    void keyPressEvent(QKeyEvent *e) override {
        if(e->key() == Qt::Key_Escape)
            this->close();
        else if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_ok_clicked();
    }

private slots:
    void update_rotation();
    void on_button_cancel_clicked();
    void on_button_ok_clicked();

signals:
    void finished();

private:
    QJsonObject rotate_by(QJsonObject sprite, int angle) const;
    QJsonObject rotate_hires_sprite(QJsonObject sprite, int angle) const;
    QJsonObject rotate_multicolor_sprite(QJsonObject sprite, int angle) const;

    QList<QJsonObject> result_list;
    QList<QJsonObject> overlay_list;
    Ui::RotationDialog *ui;
    options *opt;
    int base_index;
    QJsonObject base_sprite;
    QJsonObject overlay_sprite;
};

#endif // ROTATIONDIALOG_H
