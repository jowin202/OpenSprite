                                                                                     #ifndef ROTATIONDIALOG_H
#define ROTATIONDIALOG_H

#include <QWidget>
#include <QKeyEvent>
#include <QList>
#include <QDebug>

#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

class Sprite;

namespace Ui {
class RotationDialog;
}

class options;

class RotationDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RotationDialog(options *opt, QWidget *parent = nullptr);
    ~RotationDialog();

    void keyPressEvent(QKeyEvent *e) {
        if(e->key() == Qt::Key_Escape)
            this->close();
        else if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_ok_clicked();
    }

private slots:
    void update_rotation();
    QJsonObject rotate_by(QJsonObject sprite, int angle);
    QJsonObject rotate_hires_sprite(QJsonObject sprite, int angle);
    QJsonObject rotate_multicolor_sprite(QJsonObject sprite, int angle);

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
    Ui::RotationDialog *ui;
    options *opt;
    QJsonObject base_sprite;
    QJsonObject overlay_sprite;
    QSettings settings;
};

#endif // ROTATIONDIALOG_H
