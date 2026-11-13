#ifndef SCALINGDIALOG_H
#define SCALINGDIALOG_H

#include <QWidget>
#include <QKeyEvent>
#include <QList>

#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class ScalingDialog;
}

struct options;

class ScalingDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ScalingDialog(options *opt, QWidget *parent = nullptr);
    ~ScalingDialog();

    void keyPressEvent(QKeyEvent *e) override {
        if(e->key() == Qt::Key_Escape)
            this->close();
        else if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_ok_clicked();
    }

private slots:
    void update_scaling();
    void on_button_cancel_clicked();
    void on_button_ok_clicked();

signals:
    void finished();

private:
    QJsonObject scale_by(QJsonObject sprite, float scale_factor) const;
    QJsonObject scale_hires_sprite(QJsonObject sprite, float scale_factor) const;
    QJsonObject scale_multicolor_sprite(QJsonObject sprite, float scale_factor) const;

    QList<QJsonObject> result_list;
    QList<QJsonObject> overlay_list;
    Ui::ScalingDialog *ui;
    options *opt;
    int base_index;
    QJsonObject base_sprite;
    QJsonObject overlay_sprite;
};

#endif // SCALINGDIALOG_H
