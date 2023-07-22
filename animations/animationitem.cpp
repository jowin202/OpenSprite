#include "animationitem.h"
#include "animationpropertiesdialog.h"
#include "../sprite.h"

AnimationItem::AnimationItem(options *opt, int id)
{
    this->opt = opt;
    this->animation_id = id;

    this->from = opt->data.value("animations").toArray().at(id).toObject().value("from").toInt();
    this->to = opt->data.value("animations").toArray().at(id).toObject().value("to").toInt();
    this->timer = opt->data.value("animations").toArray().at(id).toObject().value("timer").toInt();

    this->overlay = opt->data.value("animations").toArray().at(id).toObject().value("overlay").toBool();
    this->pingpong = opt->data.value("animations").toArray().at(id).toObject().value("pingpong").toBool();
    this->valid = opt->data.value("animations").toArray().at(id).toObject().value("valid").toBool();

    for (int i = this->from; i <= this->to; i++)
    {
        this->animation_images.append(this->draw_sprite(i));
    }

}

void AnimationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);


    QFont font = painter->font();
    font.setPixelSize(16);
    painter->setFont(font);

    if (valid)
        painter->drawImage(this->boundingRect().x(), this->boundingRect().y(), this->animation_images.at(0));

    /*
    QPen pen;
    pen.setColor(Qt::white);
    painter->setPen(pen);
    */
    painter->drawText(QRect(10*24+10,0,2*10*24,10*21), Qt::AlignLeft | Qt::AlignVCenter, QString("From: %1\nTo: %2\nDelay: %3\nOverlay: %4\nPing-Pong: %5")
                      .arg(this->from).arg(this->to).arg(this->timer).arg(this->overlay).arg(this->pingpong));

    if (this->animation_id == opt->current_animation)
    {
        QPen pen;
        pen.setWidth(2);
        pen.setColor(Qt::green);
        painter->setPen(pen);
        painter->setOpacity(1);
        painter->drawRect(this->boundingRect());
    }

}

QRectF AnimationItem::boundingRect() const
{
    return QRectF(0,0,10*24 * 3, 10*21 );
}

void AnimationItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if (opt->current_animation != this->animation_id)
    {
        opt->current_animation = this->animation_id;
        emit trigger_redraw();
    }
}

void AnimationItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    AnimationPropertiesDialog *dialog = new AnimationPropertiesDialog(opt, animation_id);
    connect(dialog, &AnimationPropertiesDialog::finished, [=](){ emit trigger_redraw(); });
    dialog->show();
}

QImage AnimationItem::draw_sprite(int sprite_id)
{
    QPainter painter;
    QImage img(10*24, 10*21, QImage::Format_RGB16);
    painter.begin(&img);
    bool expand_x = this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("exp_x").toBool();
    bool expand_y = this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("exp_y").toBool();


    if (this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("mc_mode").toBool())
    {
        int w = 20 * (expand_y ? 0.5 : 1);
        int h = 10 * (expand_x ? 0.5 : 1);
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 12; x++)
            {
                if (this->get_sprite_bit(sprite_id,2*x,y)== 0 && this->get_sprite_bit(sprite_id,2*x+1,y) == 0)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(opt->data.value("background").toInt()));
                }
                else if (this->get_sprite_bit(sprite_id,2*x,y)== 1 && this->get_sprite_bit(sprite_id,2*x+1,y) == 0)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("sprite_color").toInt()));
                }
                else if (this->get_sprite_bit(sprite_id,2*x,y) == 0 && this->get_sprite_bit(sprite_id,2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc1").toInt()));
                }
                else if (this->get_sprite_bit(sprite_id,2*x,y) == 1 && this->get_sprite_bit(sprite_id,2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc2").toInt()));
                }

            }
        }
    }
    else //not multicol
    {
        int w = 10 * (expand_y ? 0.5 : 1);
        int h = 10 * (expand_x ? 0.5 : 1);
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (this->get_sprite_bit(sprite_id,x,y) == 1)
                {
                    painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("sprite_color").toInt()));
                }
                else
                {
                    painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(opt->data.value("background").toInt()));
                }
            }
        }
    }


    if (this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("overlay_next").toBool() && opt->sprite_list.length() > sprite_id+1)
    {
        if (this->opt->data.value("sprites").toArray().at(sprite_id+1).toObject().value("mc_mode").toBool())
        {
            int w = 20 * (expand_y ? 0.5 : 1);
            int h = 10 * (expand_x ? 0.5 : 1);
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 12; x++)
                {
                    if (opt->sprite_list.at(sprite_id+1)->get_bit(2*x,y)== 1 && opt->sprite_list.at(sprite_id+1)->get_bit(2*x+1,y) == 0)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id+1).toObject().value("sprite_color").toInt()));
                    }
                    else if (opt->sprite_list.at(sprite_id+1)->get_bit(2*x,y) == 0 && opt->sprite_list.at(sprite_id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc1").toInt()));
                    }
                    else if (opt->sprite_list.at(sprite_id+1)->get_bit(2*x,y) == 1 && opt->sprite_list.at(sprite_id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc2").toInt()));
                    }

                }
            }
        }
        else
        {
            int w = 10 * (expand_y ? 0.5 : 1);
            int h = 10 * (expand_x ? 0.5 : 1);
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 24; x++)
                {
                    if (opt->sprite_list.at(sprite_id+1)->get_bit(x,y) == 1)
                    {
                        painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id+1).toObject().value("sprite_color").toInt()));
                    }
                }
            }
        }
    }



    if (opt->show_grid_lines)
    {

        int w = 10 * (expand_y ? 0.5 : 1);
        int h = 10 * (expand_x ? 0.5 : 1);

        QPen pen;
        pen.setColor(Qt::gray);
        pen.setWidthF(0.5);
        painter.setPen(pen);
        for (int y = 0; y < 21; y++)
            painter.drawLine(0, h*y, 24*w, h*y);
        for (int x = 0; x < 24; x+=2)
            painter.drawLine(w*x, 0, w*x, h*21);

        if (!this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("mc_mode").toBool())
            for (int x = 1; x < 24; x+=2)
                painter.drawLine(w*x, 0, w*x, h*21);
    }



    return img;
}

int AnimationItem::get_sprite_bit(int sprite_id, int x, int y)
{
    if (x < 0 || y < 0 || x >= 24 || y >= 21) return false;
    if (opt->data.value("sprites").toArray().count() <= sprite_id) return false;
    return opt->data.value("sprites").toArray().at(sprite_id).toObject().value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}
