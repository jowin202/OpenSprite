#include "spriteview.h"
#include "sprite.h"

SpriteView::SpriteView(QWidget *parent) : QGraphicsView(parent)
{
    this->setScene(new QGraphicsScene);
    this->scene()->setSceneRect(0,0,2500,1080);
}

void SpriteView::redraw()
{
    this->setBackgroundBrush(opt->col_list.at(opt->background));


    for (int i = 0; i < opt->sprite_num; i++)
    {
        opt->sprite_list.at(i)->id = i;
        opt->sprite_list.at(i)->setPos(30+(10*24+30)*(i%8),30+(10*21+30)*(i/8));
        this->scene()->addItem(opt->sprite_list.at(i));
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

