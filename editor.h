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

    enum {TRANSPARENT, COLOR, MC1, MC2, OVERLAY };

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
        this->update_view();
        if (this->sprite != 0)
            this->sprite->multi_color_mode = multicol;
    }
    void set_overlay(bool overlay){
        this->overlay = overlay;
        if (this->sprite != 0)
            this->sprite->overlay_next = overlay;

        if (this->overlay_sprite != 0 && overlay == true)
        {
            this->overlay_sprite->multi_color_mode = false; // no multicolor if overlay
        }
        this->update_view();
    }
    void set_expand_x(bool expand_x){
        this->expand_x = expand_x;
        this->update_multiplicator();
        this->update_view();
    }
    void set_expand_y(bool expand_y){
        this->expand_y = expand_y;
        this->update_multiplicator();
        this->update_view();
    }
    void setgridlines(bool gridlines){
        this->gridlines = gridlines;
        this->update_view();
    }




    void update_view();

signals:


public slots:
    void set_transparent(int color)
    {
        this->transparent_color = color;
        this->update_view();
    }
    void set_sprite_color(int color)
    {
        this->sprite_color = color;
        this->update_view();
        if (sprite != 0)
            this->sprite->sprite_color = color;
    }
    void set_mc1(int mc1)
    {
        this->mc1 = mc1;
        this->update_view();
    }
    void set_mc2(int mc2)
    {
        this->mc2 = mc2;
        this->update_view();
    }
    void set_overlay_color(int oc)
    {
        if (this->overlay_sprite != 0)
            this->overlay_sprite->sprite_color = oc;
        this->update_view();
    }
    void set_sprite(Sprite *sprite)
    {
        this->sprite = sprite;
        this->update_view();
    }
    void set_overlay_sprite(Sprite *sprite)
    {
        this->overlay_sprite = sprite;
        this->update_view();
    }
    void set_multiplicator(int m)
    {
        this->multiplikator = m;
        this->update_multiplicator();
        this->update_view();
    }
    void set_right_button(int v)
    {this->right_button = v;}
    void set_left_button(int v)
    {this->left_button = v;}



signals:
    void mouse_updated_cell_updated(int,int);
    void update_viewer();

private:
    QPoint curr_pos;
    bool multicol = true;
    bool overlay = false;
    bool expand_x = false;
    bool expand_y = false;


    bool gridlines = true;
    QList<QColor> col_list;

    int transparent_color = 6;
    int sprite_color = 5;
    int mc1 = 2;
    int mc2 = 3;
    int multiplikator = 1;
    int multiplikator_x = 1;
    int multiplikator_y = 1;

    int left_button = COLOR;
    int right_button = TRANSPARENT;

    bool left_button_pressed = false;
    bool right_button_pressed = false;

    Sprite * sprite = 0;
    Sprite * overlay_sprite = 0;
};

#endif // EDITOR_H
