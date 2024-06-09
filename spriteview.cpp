#include "spriteview.h"
#include "sprite.h"
#include "addbutton.h"

SpriteView::SpriteView(QWidget *parent) : QGraphicsView(parent)
{
    this->setRenderHint(QPainter::Antialiasing, true);
    this->setScene(new QGraphicsScene);
    this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

}

void SpriteView::redraw()
{
    this->setBackgroundBrush(QColor(settings.value("bgcolor").toInt()));

    this->scene()->clear();
    this->opt->sprite_list.clear();

    int sprite_spacing_x = settings.value("sprite_spacing_x").toInt();
    int sprite_spacing_y = settings.value("sprite_spacing_y").toInt();
    int sprites_per_row = settings.value("sprites_per_row").toInt();


    int max_x = sprite_spacing_x+(10*24+sprite_spacing_x)*(sprites_per_row);
    int max_y = sprite_spacing_y+(10*21+sprite_spacing_y)*(1+opt->data.value("sprites").toArray().count()/sprites_per_row);
    this->scene()->setSceneRect(0,0,max_x,max_y);

    int i; //use it later
    for (i = 0; i < opt->data.value("sprites").toArray().count(); i++)
    {
        Sprite *sprite = new Sprite(opt, i);
        sprite->setPos(sprite_spacing_x+(10*24+sprite_spacing_x)*(i% sprites_per_row),sprite_spacing_y+(10*21+sprite_spacing_y)*(i/sprites_per_row));
        this->scene()->addItem(sprite);
        opt->sprite_list.append(sprite);
    }

    AddButton *add_button = new AddButton();
    add_button->setPos(sprite_spacing_x+(10*24+sprite_spacing_x)*(i% sprites_per_row),sprite_spacing_y+(10*21+sprite_spacing_y)*(i/sprites_per_row));
    connect(add_button, SIGNAL(clicked()), this, SLOT(add_new_sprite()));
    this->scene()->addItem(add_button);

    this->update();
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

void SpriteView::add_new_sprite()
{
    QJsonObject sprite;

    QJsonArray array_rows;
    for (int y = 0; y < 21; y++)
    {
        QJsonArray array_row;
        for (int x = 0; x < 24; x++)
            array_row.append( 0 );
        array_rows.append(array_row);
    }

    sprite.insert("sprite_data", array_rows);
    sprite.insert("mc_mode", true);
    sprite.insert("overlay_next", false);
    sprite.insert("sprite_color", 5);
    QJsonArray sprites = opt->data.value("sprites").toArray();
    sprites.append(sprite);
    opt->data.insert("sprites", sprites);
    opt->selection_from = sprites.count()-1;
    opt->selection_to = sprites.count()-1;
    this->redraw();
    emit current_sprite_changed(opt->selection_to);
}

