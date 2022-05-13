#ifndef VIEWER_H
#define VIEWER_H

#include <QObject>
#include <QLabel>
#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include "sprite.h"

class Viewer : public QLabel
{
    Q_OBJECT
public:
    Viewer(QWidget *parent = 0);
    void add_new_sprite();
    void add_new_sprites(int n);
    Sprite *sprite_at(int n)
    {
        return sprite_list.at(n);
    }


    void mousePressEvent(QMouseEvent *ev);


    QImage sprite_to_thumbnail(int i);

    int current_sprite = 0;
    int horizontal_sprites = 8;
    int transparent_color = 6;
    int mc1 = 2;
    int mc2 = 3;
    bool gridlines = true;

    QList<Sprite*> sprite_list;
    QList<QColor> col_list;
    QImage current;


signals:
    void sprite_selected(Sprite *sprite);
    void sprite_update_detail_gui(int,bool,bool,bool,bool);

public slots:
    void update_view();
    void update_current_sprite();
    void set_mc1(int mc1)
    {this->mc1 = mc1; this->update_view(); }
    void set_mc2(int mc2)
    {this->mc2 = mc2; this->update_view(); }
    void set_gridlines(bool gridlines)
    {this->gridlines = gridlines; this->update_view(); }

};

#endif // VIEWER_H
