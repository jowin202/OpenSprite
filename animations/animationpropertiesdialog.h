#ifndef ANIMATIONPROPERTIESDIALOG_H
#define ANIMATIONPROPERTIESDIALOG_H

#include <QWidget>
#include <QKeyEvent>

struct options;


namespace Ui {
class AnimationPropertiesDialog;
}

class AnimationPropertiesDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AnimationPropertiesDialog(options *opt, int id, QWidget *parent = nullptr);
    ~AnimationPropertiesDialog();


    void keyPressEvent(QKeyEvent *e) {
        if(e->key() == Qt::Key_Escape)
            this->on_button_cancel_clicked();
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->on_button_ok_clicked();
    }


signals:
    void finished();

private slots:
    void on_button_cancel_clicked();
    void on_button_ok_clicked();

private:
    Ui::AnimationPropertiesDialog *ui;
    int current_id;
    options *opt;
};

#endif // ANIMATIONPROPERTIESDIALOG_H
