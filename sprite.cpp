#include "sprite.h"
#include "spriteview.h"

Sprite::Sprite(options *opt)
{
    this->opt = opt;
}

void Sprite::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);


    painter->drawRect(QRectF(0,0,10*24 *(expand_x ? 2 : 1), 10*21 *(expand_y ? 2 : 1)));
    if (this->opt->data.value("sprites").toArray().at(id).toObject().value("mc_mode").toBool())
    {
        int w = 20;
        int h = 10;
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 12; x++)
            {
                if (this->get_bit(2*x,y)== 0 && this->get_bit(2*x+1,y) == 0)
                {
                    painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(opt->data.value("background").toInt()));
                }
                else if (this->get_bit(2*x,y)== 1 && this->get_bit(2*x+1,y) == 0)
                {
                    painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(id).toObject().value("sprite_color").toInt()));
                }
                else if (this->get_bit(2*x,y) == 0 && this->get_bit(2*x+1,y) == 1)
                {
                    painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->data.value("mc1").toInt()));
                }
                else if (this->get_bit(2*x,y) == 1 && this->get_bit(2*x+1,y) == 1)
                {
                    painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->data.value("mc2").toInt()));
                }

            }
        }
    }
    else //not multicol
    {
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (this->get_bit(x,y) == 1)
                {
                    painter->fillRect(10*x,10*y,10,10,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(id).toObject().value("sprite_color").toInt()));
                }
                else
                {
                    painter->fillRect(10*x,10*y,10,10,this->opt->col_list.at(opt->data.value("background").toInt()));
                }
            }
        }
    }


    if (this->opt->data.value("sprites").toArray().at(id).toObject().value("overlay_next").toBool() && opt->sprite_list.length() > this->id+1)
    {
        if (this->opt->data.value("sprites").toArray().at(id+1).toObject().value("mc_mode").toBool())
        {
            int w = 20;
            int h = 10;
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 12; x++)
                {
                    if (opt->sprite_list.at(id+1)->get_bit(2*x,y)== 1 && opt->sprite_list.at(id+1)->get_bit(2*x+1,y) == 0)
                    {
                        painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(id+1).toObject().value("sprite_color").toInt()));
                    }
                    else if (opt->sprite_list.at(id+1)->get_bit(2*x,y) == 0 && opt->sprite_list.at(id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->data.value("mc1").toInt()));
                    }
                    else if (opt->sprite_list.at(id+1)->get_bit(2*x,y) == 1 && opt->sprite_list.at(id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->data.value("mc2").toInt()));
                    }

                }
            }
        }
        else
        {
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 24; x++)
                {
                    if (opt->sprite_list.at(id+1)->get_bit(x,y) == 1)
                    {
                        painter->fillRect(10*x,10*y,10,10,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(id+1).toObject().value("sprite_color").toInt()));
                    }
                }
            }
        }
    }

    if (opt->show_grid_lines)
    {
        for (int y = 0; y < 21; y++)
            painter->drawLine(0, 10*y, 24*10, 10*y);
        for (int x = 0; x < 24; x+=2)
            painter->drawLine(10*x, 0, 10*x, 10*21);

        if (!this->opt->data.value("sprites").toArray().at(id).toObject().value("mc_mode").toBool())
            for (int x = 1; x < 24; x+=2)
                painter->drawLine(10*x, 0, 10*x, 10*21);
    }


    if (id == opt->current_sprite)
    {
        QPen pen;
        pen.setWidth(2);
        pen.setColor(Qt::green);
        painter->setPen(pen);
        painter->setOpacity(1);
        painter->drawRect(this->boundingRect());
    }
}

QRectF Sprite::boundingRect() const
{
    return QRectF(0,0,10*24 *(expand_x ? 2 : 1), 10*21 *(expand_y ? 2 : 1));
}

void Sprite::change_tile(QPointF pos)
{
    if (this->opt->data.value("sprites").toArray().at(id).toObject().value("mc_mode").toBool())
    {
        int x = pos.x()/20;
        int y = pos.y()/10;

        if ((left_pressed && opt->left_button == TRANSPARENT) ||
                (right_pressed && opt->right_button == TRANSPARENT))
        {
            this->set_bit(2*x,y, false);
            this->set_bit(2*x+1,y, false);
        }
        else if ((left_pressed && opt->left_button == COLOR) ||
                (right_pressed && opt->right_button == COLOR))
        {
            this->set_bit(2*x,y, true);
            this->set_bit(2*x+1,y, false);
        }
        else if ((left_pressed && opt->left_button == MC1) ||
                (right_pressed && opt->right_button == MC1))
        {
            this->set_bit(2*x,y, false);
            this->set_bit(2*x+1,y, true);
        }
        else if ((left_pressed && opt->left_button == MC2) ||
                (right_pressed && opt->right_button == MC2))
        {
            this->set_bit(2*x,y, true);
            this->set_bit(2*x+1,y, true);
        }
    }
    else //singlecolor
    {
        int x = pos.x()/10;
        int y = pos.y()/10;

        this->set_bit(x, y, (right_pressed && opt->right_button == COLOR) ||
                      (left_pressed && opt->left_button == COLOR));
    }


    if (((left_pressed && opt->left_button == OVERLAY_COLOR) ||
            (right_pressed && opt->right_button == OVERLAY_COLOR))
            && opt->sprite_list.length() > this->id+1)
    {
        this->opt->sprite_list.at(id+1)->set_bit(pos.x()/10, pos.y()/10, true);
        this->opt->sprite_list.at(id+1)->update();
    }
    else if (((left_pressed && opt->left_button == OVERLAY_TRANSPARENT) ||
                (right_pressed && opt->right_button == OVERLAY_TRANSPARENT))
                && opt->sprite_list.length() > this->id+1)
    {
        this->opt->sprite_list.at(id+1)->set_bit(pos.x()/10, pos.y()/10, false);
        this->opt->sprite_list.at(id+1)->update();
    }


    if (this->id > 0)
        this->opt->sprite_list.at(id-1)->update();


    this->update();
}

void Sprite::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if (this->opt->current_sprite != id)
    {
        Sprite *old = opt->sprite_list.at(this->opt->current_sprite);
        this->opt->current_sprite = id;
        old->update();
        opt->spriteview->change_current_sprite(id);
        this->update();
        return; //dont do anything else when switching sprite
    }
    if (ev->button() == Qt::LeftButton)
        this->left_pressed = true;
    else if (ev->button() == Qt::RightButton)
        this->right_pressed = true;

    this->change_tile(ev->pos());
}

void Sprite::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    if (this->boundingRect().contains(ev->pos()))
        this->change_tile(ev->pos());
}

void Sprite::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        this->left_pressed = false;
    else if (ev->button() == Qt::RightButton)
        this->right_pressed = false;
}


bool Sprite::get_bit(int x, int y)
{
    if (x >= 24 || y >= 21) return false;
    if (opt->data.value("sprites").toArray().count() <= id) return false;
    return opt->data.value("sprites").toArray().at(id).toObject().value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}

void Sprite::set_bit(int x, int y, bool value)
{
    if (x >= 24 || y >= 21) return;

    QJsonObject current_sprite_obj = opt->data.value("sprites").toArray().at(id).toObject();
    QJsonArray array_y = current_sprite_obj.value("sprite_data").toArray();
    QJsonArray array_x = array_y.at(y).toArray();
    array_x.removeAt(x);
    array_x.insert(x, value ? 1 : 0);

    array_y.removeAt(y);
    array_y.insert(y, array_x);
    current_sprite_obj.insert("sprite_data", array_y);

    QJsonArray sprites_array = opt->data.value("sprites").toArray();
    sprites_array.removeAt(id);
    sprites_array.insert(id, current_sprite_obj);
    opt->data.insert("sprites", sprites_array);
}
