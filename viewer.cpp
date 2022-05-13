#include "viewer.h"

Viewer::Viewer(QWidget *parent)
{
    col_list << QColor(0,0,0);
    col_list << QColor(255,255,255);
    col_list << QColor(0x92,0x4a,0x40);
    col_list << QColor(0x84,0xc5,0xcc);
    col_list << QColor(0x93,0x51,0xb6);
    col_list << QColor(0x72,0xb1,0x4b);
    col_list << QColor(0x48,0x3a,0xaa);
    col_list << QColor(0xd5,0xdf,0x7c);
    col_list << QColor(0x99,0x69,0x2d);
    col_list << QColor(0x67,0x52,0x00);
    col_list << QColor(0xc1,0x81,0x7a);
    col_list << QColor(0x60,0x60,0x60);
    col_list << QColor(0x8a,0x8a,0x8a);
    col_list << QColor(0xb3,0xec,0x91);
    col_list << QColor(0x86,0x7a,0xde);
    col_list << QColor(0xb3,0xb3,0xb3);
}

void Viewer::add_new_sprite()
{
    this->sprite_list.append(new Sprite());
}

void Viewer::add_new_sprites(int n)
{
    for (int i = 0; i < n; i++)
        this->sprite_list.append(new Sprite());
}

void Viewer::mousePressEvent(QMouseEvent *ev)
{
    if (ev->pos().x() < 0 || ev->pos().y() < 0)
        return;

    if (ev->pos().x() > 48*horizontal_sprites || ev->pos().y() > 42*((this->sprite_list.length()-1)/horizontal_sprites+1))
        return;
    qDebug() << ev->pos();

    int x = (ev->pos().x()/48) % this->horizontal_sprites;
    int y = (ev->pos().y()/42);

    if (x + y*this->horizontal_sprites >= this->sprite_list.length())
        return;

    this->current_sprite = x + y*this->horizontal_sprites;
    Sprite *current_sprite_object = this->sprite_list.at(this->current_sprite);
    emit sprite_selected(current_sprite_object);
    emit sprite_update_detail_gui(current_sprite_object->sprite_color, current_sprite_object->expand_x, current_sprite_object->expand_y, current_sprite_object->overlay_next, current_sprite_object->multi_color_mode);
    this->update_view();
}

QImage Viewer::sprite_to_thumbnail(int i)
{
    QImage img(48,42,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    painter.fillRect(0,0,img.width(),img.height(), col_list.at(this->transparent_color));



    if (this->sprite_list.at(i)->multi_color_mode)
    {
        int h = 42/21;
        int w = 48/12;
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 12; x++)
            {
                if (this->sprite_list.at(i)->get_bit(2*x,y)== 1 && this->sprite_list.at(i)->get_bit(2*x+1,y) == 0)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->sprite_list.at(i)->sprite_color));
                }
                else if (this->sprite_list.at(i)->get_bit(2*x,y) == 0 && this->sprite_list.at(i)->get_bit(2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->mc1));
                }
                else if (this->sprite_list.at(i)->get_bit(2*x,y) == 1 && this->sprite_list.at(i)->get_bit(2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->mc2));
                }

            }
        }

    }
    else
    {
        int h = 42/21;
        int w = 48/24;
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (this->sprite_at(i)->get_bit(x,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->sprite_list.at(i)->sprite_color));
                }
            }
        }
    }

    painter.end();
    return img;
}

void Viewer::update_current_sprite()
{
    this->update_view();
}

void Viewer::update_view()
{
    QImage img(48*horizontal_sprites,42*((this->sprite_list.length()-1)/horizontal_sprites+1),QImage::Format_RGB32);
    QPainter painter;
    QImage thumb;
    painter.begin(&img);
    painter.fillRect(0,0,img.width(),img.height(), Qt::black); //col_list.at(this->transparent_color));


    for (int i = 0; i < sprite_list.length(); i++)
    {
        int y = 42*(i/this->horizontal_sprites);
        int x = 48*(i%this->horizontal_sprites);
        thumb = this->sprite_to_thumbnail(i);
        painter.drawImage(QRect(x,y,48,42),thumb,QRect(0,0,48,42));

        if (i == current_sprite)
        {
            painter.setOpacity(0.3);
            painter.fillRect(x,y,48,42, Qt::white);
            painter.setOpacity(1.0);
        }
    }
    if (this->gridlines)
    {
        for (int i = 1; i < this->horizontal_sprites; i++)
            painter.drawLine(48*i,0,48*i,img.height());
        for (int i = 1; i < ((this->sprite_list.length()-1)/horizontal_sprites+1); i++)
            painter.drawLine(0,42*i,img.width(),42*i);
    }



    painter.end();
    this->setPixmap(QPixmap::fromImage(img));
}
