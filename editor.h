#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QPoint>
#include <QMouseEvent>
#include <QPainter>

class Editor: public QLabel
{
    Q_OBJECT
public:
    explicit Editor(QWidget *parent = nullptr);

    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

    void set_multicol(bool multicol){
        this->multicol = multicol;
        this->updateView();
    }
    void set_overlay(bool overlay){
        this->overlay = overlay;
        this->updateView();
    }
    void set_expand_x(bool expand_x){
        this->expand_x = expand_x;
        this->updateView();
    }
    void set_expand_y(bool expand_y){
        this->expand_y = expand_y;
        this->updateView();
    }
    void setgridlines(bool gridlines){
        this->gridlines = gridlines;
        this->updateView();
    }

    void updateView();

signals:


public slots:
    void set_transparent(int color)
    {
        this->transparent_color = color;
        this->updateView();
    }
    void set_sprite_color(int color)
    {
        this->sprite_color = color;
        this->updateView();
    }

private:
    QPoint curr_pos;
    bool multicol = true;
    bool overlay = true;
    bool expand_x = false;
    bool expand_y = false;


    bool gridlines = true;
    QList<QColor> col_list;

    int transparent_color = 6;
    int sprite_color = 5;
};

#endif // EDITOR_H
