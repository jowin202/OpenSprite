#include "spriteview.h"
#include "sprite.h"

SpriteView::SpriteView(QWidget *parent) : QGraphicsView(parent)
{
    this->setScene(new QGraphicsScene);
    this->scene()->setSceneRect(0,0,2200,1000);
}

void SpriteView::redraw()
{
    this->setBackgroundBrush(opt->col_list.at(opt->data.value("background").toInt()));

    this->scene()->clear();
    this->opt->sprite_list.clear();
    this->update();

    for (int i = 0; i < opt->data.value("sprites").toArray().count(); i++)
    {
        Sprite *sprite = new Sprite(opt);
        sprite->id = i;
        sprite->setPos(opt->sprite_spacing_x+(10*24+opt->sprite_spacing_x)*(i% opt->sprites_per_row),opt->sprite_spacing_y+(10*21+opt->sprite_spacing_y)*(i/opt->sprites_per_row));
        this->scene()->addItem(sprite);
        opt->sprite_list.append(sprite);
    }
}

void SpriteView::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::Modifier::CTRL) != 0)
    {
        if (event->angleDelta().y() > 0)
            emit zoom_in();
        else
            emit zoom_out();

        return;
    }
    event->accept();
    QGraphicsView::wheelEvent(event);
}

