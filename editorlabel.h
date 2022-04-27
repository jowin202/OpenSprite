#ifndef EDITORLABEL_H
#define EDITORLABEL_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QPoint>
#include <QMouseEvent>
#include <QPainter>

class EditorLabel : public QLabel
{
    Q_OBJECT
public:
    explicit EditorLabel(QWidget *parent = nullptr);

    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

    void setmulticol(bool multicol){
        this->multicol = multicol;
        this->updateView();
    }
    void setgridlines(bool multicol){
        this->gridlines = multicol;
        this->updateView();
    }

    void updateView();

signals:

private:
    QPoint curr_pos;
    bool multicol = true;
    bool gridlines = true;
};

#endif // EDITORLABEL_H
