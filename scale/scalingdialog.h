#ifndef SCALINGDIALOG_H
#define SCALINGDIALOG_H

#include <QWidget>
#include <QKeyEvent>
#include <QList>
#include <QDebug>

#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

class Sprite;

namespace Ui {
class ScalingDialog;
}

class options;

class ScalingDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ScalingDialog(options *opt, QWidget *parent = nullptr);
    ~ScalingDialog();

    void keyPressEvent(QKeyEvent *e) {
        if(e->key() == Qt::Key_Escape)
            this->close();
        else if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_ok_clicked();
    }

private slots:
    void update_scaling();
    QJsonObject scale_by(QJsonObject sprite, float scale_factor);
    QJsonObject scale_hires_sprite(QJsonObject sprite, float scale_factor);
    QJsonObject scale_multicolor_sprite(QJsonObject sprite, float scale_factor);

    QImage draw_sprite(int sprite_id);
    int get_sprite_bit(int sprite_id, int x, int y);
    int get_overlay_bit(int sprite_id, int x, int y);

    void on_button_cancel_clicked();
    void on_button_ok_clicked();

signals:
    void finished();

private:
    QList<QJsonObject> result_list;
    QList<QJsonObject> overlay_list;
    Ui::ScalingDialog *ui;
    options *opt;
    QJsonObject base_sprite;
    QJsonObject overlay_sprite;
    QSettings settings;
};

#endif // SCALINGDIALOG_H