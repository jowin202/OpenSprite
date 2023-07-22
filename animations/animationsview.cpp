#include "animationsview.h"
#include "../sprite.h"
#include "../addbutton.h"
#include "animationitem.h"


AnimationsView::AnimationsView(QWidget *parent) : QGraphicsView(parent)
{
    this->setRenderHint(QPainter::Antialiasing, true);
    this->setScene(new QGraphicsScene);
    this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void AnimationsView::redraw()
{
    this->setBackgroundBrush(opt->background);

    this->scene()->clear();
    QJsonArray animation_list = opt->data.value("animations").toArray();


    int max_x = opt->sprite_spacing_x+(10*24+opt->sprite_spacing_x)*4;
    int max_y = opt->sprite_spacing_y+(10*21+opt->sprite_spacing_y)*(1+opt->data.value("animations").toArray().count());
    this->scene()->setSceneRect(0,0,max_x,max_y);



    int i; //use it later
    for (i = 0; i < opt->data.value("animations").toArray().count(); i++)
    {
        AnimationItem *item = new AnimationItem(opt, i);
        item->setPos(opt->sprite_spacing_x/*+(10*24+opt->sprite_spacing_x)*(i% opt->sprites_per_row)*/,
                     opt->sprite_spacing_y+(10*21+opt->sprite_spacing_y)*(i));
        connect(item, &AnimationItem::trigger_redraw, [=](){ this->redraw(); });
        this->scene()->addItem(item);
    }


    AddButton *add_button = new AddButton();
    add_button->setPos(opt->sprite_spacing_x,opt->sprite_spacing_y+(10*21+opt->sprite_spacing_y)*(i));
    connect(add_button, SIGNAL(clicked()), this, SLOT(add_new_animation()));
    this->scene()->addItem(add_button);



    this->update();
}

void AnimationsView::add_new_animation()
{
    qDebug() << opt->data.value("animations").toArray().first();
}
