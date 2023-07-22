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
            this->close();
        else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            this->close();
    }


private slots:

private:
    Ui::AnimationDialog *ui;
    options *opt;
};

#endif // ANIMATIONDIALOG_H
