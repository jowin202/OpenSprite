#ifndef VIEWER_H
#define VIEWER_H

#include <QObject>
#include <QLabel>
#include <QWidget>
#include <QDebug>
#include <QPainter>
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


    int horizontal_sprites = 8;
    int transparent_color = 6;
    int sprite_color = 5;
    int mc1 = 2;
    int mc2 = 3;
    bool gridlines = true;

    QList<Sprite*> sprite_list;
    QList<QColor> col_list;
    QImage current;

public slots:
    void update_view();

};

#endif // VIEWER_H
