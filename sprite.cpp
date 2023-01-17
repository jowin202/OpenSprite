#include "sprite.h"
#include "spriteview.h"

Sprite::Sprite(options *opt)
{
    this->opt = opt;
    for (int i = 0; i < 64; i++)
        this->sprite_data[i] = 0;
}

void Sprite::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);


    painter->drawRect(QRectF(0,0,10*24 *(expand_x ? 2 : 1), 10*21 *(expand_y ? 2 : 1)));
    if (this->multi_color_mode)
    {
        int w = 20;
        int h = 10;
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 12; x++)
            {
                if (this->get_bit(2*x,y)== 1 && this->get_bit(2*x+1,y) == 0)
                {
                    painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->sprite_color));
                }
                else if (this->get_bit(2*x,y) == 0 && this->get_bit(2*x+1,y) == 1)
                {
                    painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->mc1));
                }
                else if (this->get_bit(2*x,y) == 1 && this->get_bit(2*x+1,y) == 1)
                {
                    painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->mc2));
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
                    painter->fillRect(10*x,10*y,10,10,this->opt->col_list.at(this->sprite_color));
                }
            }
        }
    }


    if (this->overlay_next && opt->sprite_list.length() > this->id+1)
    {
        if (opt->sprite_list.at(id+1)->multi_color_mode)
        {
            int w = 20;
            int h = 10;
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 12; x++)
                {
                    if (opt->sprite_list.at(id+1)->get_bit(2*x,y)== 1 && opt->sprite_list.at(id+1)->get_bit(2*x+1,y) == 0)
                    {
                        painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(opt->sprite_list.at(id+1)->sprite_color));
                    }
                    else if (opt->sprite_list.at(id+1)->get_bit(2*x,y) == 0 && opt->sprite_list.at(id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->mc1));
                    }
                    else if (opt->sprite_list.at(id+1)->get_bit(2*x,y) == 1 && opt->sprite_list.at(id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter->fillRect(x*w,y*h,w,h,this->opt->col_list.at(this->opt->mc2));
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
                        painter->fillRect(10*x,10*y,10,10,this->opt->col_list.at(opt->sprite_list.at(id+1)->sprite_color));
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

        if (!this->multi_color_mode)
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
    if (this->multi_color_mode)
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


char Sprite::get_bit(int x, int y)
{
    char val = sprite_data[3*y + x/8];
    int pos = x%8;
    return (val&(0x01 << (7-pos))) >> (7-pos);
}

void Sprite::set_bit(int x, int y, bool value)
{
    if (x >= 24 || y >= 21) return;

    int pos = x%8;
    if (value)
        sprite_data[3*y + x/8] |= (0x01 << (7-pos));
    else
        sprite_data[3*y + x/8] &= ~(0x01 << (7-pos));
}
