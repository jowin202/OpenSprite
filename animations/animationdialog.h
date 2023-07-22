#ifndef ANIMATIONDIALOG_H
#define ANIMATIONDIALOG_H

#include <QWidget>
#include <QKeyEvent>

struct options;

namespace Ui {
class AnimationDialog;
}

class AnimationDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AnimationDialog(options *opt);
    ~AnimationDialog();


    void keyPressEvent(QKeyEvent *e) {
        if(e->key() == Qt::Key_Escape)
            this->on_button_cancel_clicked();
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_ok_clicked();
    }


private slots:
    void on_button_cancel_clicked();

    void on_button_ok_clicked();

private:
    Ui::AnimationDialog *ui;
    options *opt;
};

#endif // ANIMATIONDIALOG_H
