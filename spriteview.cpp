#include "spriteview.h"
#include "sprite.h"
#include "addbutton.h"

SpriteView::SpriteView(QWidget *parent) : QGraphicsView(parent)
{
    this->setRenderHint(QPainter::Antialiasing, true);
    this->setScene(new QGraphicsScene(this));
    this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void SpriteView::redraw()
{
    if (!opt)
        return;

    this->setBackgroundBrush(QColor::fromRgb(QRgb(settings.value("bgcolor").toInt())));
    opt->selection_color = QColor::fromRgb(QRgb(settings.value("selection_color").toInt()));
    opt->clamp_selection();

    this->scene()->clear();
    this->opt->sprite_list.clear();

    const int sprite_width = 10 * 24;
    const int sprite_height = 10 * 21;
    const int sprite_spacing_x = settings.value("sprite_spacing_x").toInt();
    const int sprite_spacing_y = settings.value("sprite_spacing_y").toInt();
    int sprites_per_row;

    if (settings.value("sprites_per_row_auto", false).toBool()) {
        int available = (int)(this->viewport()->width() / this->transform().m11()) - sprite_spacing_x;
        sprites_per_row = available / (sprite_width + sprite_spacing_x);
    } else {
        sprites_per_row = settings.value("sprites_per_row").toInt();
    }
    sprites_per_row = qMax(1, sprites_per_row);

    const int count = opt->sprite_count();
    auto position = [&](int i) {
        return QPointF(sprite_spacing_x + (sprite_width + sprite_spacing_x) * (i % sprites_per_row),
                       sprite_spacing_y + (sprite_height + sprite_spacing_y) * (i / sprites_per_row));
    };

    int max_x = sprite_spacing_x + (sprite_width + sprite_spacing_x) * sprites_per_row;
    int max_y = sprite_spacing_y + (sprite_height + sprite_spacing_y) * (1 + count / sprites_per_row);
    this->scene()->setSceneRect(0, 0, max_x, max_y);

    opt->sprite_list.reserve(count);
    for (int i = 0; i < count; i++)
    {
        Sprite *sprite = new Sprite(opt, i);
        sprite->setPos(position(i));
        this->scene()->addItem(sprite);
        opt->sprite_list.append(sprite);
    }

    AddButton *add_button = new AddButton();
    add_button->setPos(position(count));
    connect(add_button, &AddButton::clicked, this, &SpriteView::add_new_sprite);
    this->scene()->addItem(add_button);

    this->update();
}

void SpriteView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    if (opt && settings.value("sprites_per_row_auto", false).toBool())
        redraw();
}

void SpriteView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->angleDelta().y() > 0)
            emit zoom_in();
        else if (event->angleDelta().y() < 0)
            emit zoom_out();
        event->accept();
        return;
    }
    QGraphicsView::wheelEvent(event);
}

void SpriteView::add_new_sprite()
{
    if (!opt)
        return;
    opt->push_undo();
    QJsonArray sprites = opt->data.take("sprites").toArray();
    sprites.append(SpriteData::new_sprite());
    opt->data.insert("sprites", sprites);
    opt->selection_from = sprites.count()-1;
    opt->selection_to = sprites.count()-1;
    this->redraw();
    emit current_sprite_changed(opt->selection_to);
}
