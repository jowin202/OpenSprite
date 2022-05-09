#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QPoint>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>

#include <QDebug>
#include <iostream>
#include "sprite.h"

class Editor: public QLabel
{
    Q_OBJECT
public:
    explicit Editor(QWidget *parent = nullptr);

    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

    void update_multiplicator()
    {
        if (this->expand_y)
            this->multiplikator_y = 2*multiplikator;
        else this->multiplikator_y = multiplikator;

        if (this->expand_x)
            this->multiplikator_x = 2*multiplikator;
        else this->multiplikator_x = multiplikator;
    }


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
        this->update_multiplicator();
        this->updateView();
    }
    void set_expand_y(bool expand_y){
        this->expand_y = expand_y;
        this->update_multiplicator();
        this->updateView();
    }
    void setgridlines(bool gridlines){
        this->gridlines = gridlines;
        this->updateView();
    }

    char get_bit(int x, int y)
    {
        if (this->multicol)
        {

        }
        else
        {
            char val = this->sprite->sprite_data[3*y + x/8];
            int pos = x%8;
            return (val&(0x01 << (7-pos))) >> (7-pos);
        }
        return 0;
    }

    void set_bit(int x, int y, bool value)
    {
        if (this->multicol)
        {

        }
        else
        {
            int pos = x%8;
            if (value)
                this->sprite->sprite_data[3*y + x/8] |= (0x01 << (7-pos));
            else
                this->sprite->sprite_data[3*y + x/8] &= ~(0x01 << (7-pos));
        }
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
    void set_sprite(Sprite *sprite)
    {
        this->sprite = sprite;
    }

signals:
    void mouse_updated_cell_updated(int,int);
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
    int multiplikator = 1;
    int multiplikator_x = 1;
    int multiplikator_y = 1;
    bool control_pressed = false;

    bool left_button_pressed = false;

    Sprite *sprite = 0;
};

#endif // EDITOR_H
