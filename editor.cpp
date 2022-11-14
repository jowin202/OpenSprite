#include "editor.h"

Editor::Editor(QWidget *parent) : QLabel(parent)
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

    this->update_view();
}

void Editor::mouseMoveEvent(QMouseEvent *ev)
{
    if (ev->pos().x() >= this->pixmap()->width() || ev->pos().x() < 0)
        return;
    if (ev->pos().y() >= this->pixmap()->height() || ev->pos().y() < 0)
        return;


    //qDebug() << this->pixmap()->width() << ev->pos().x();

    if (this->multicol)
    {
        this->curr_pos.setX(ev->pos().x()/(multiplikator_x*16));
        this->curr_pos.setY(ev->pos().y()/(multiplikator_y*8));

        if (left_button_pressed)
        {
            if (this->left_button == TRANSPARENT)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
            }
            else if (this->left_button == COLOR)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
            }
            else if (this->left_button == MC1)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
            }
            else if (this->left_button == MC2)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
            }
            this->update_view();
        }

        if (right_button_pressed)
        {
            if (this->right_button == TRANSPARENT)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
            }
            else if (this->right_button == COLOR)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
            }
            else if (this->right_button == MC1)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
            }
            else if (this->right_button == MC2)
            {
                this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
            }
            this->update_view();
        }

    }
    else
    {
        this->curr_pos.setX(ev->pos().x()/(multiplikator_x*8));
        this->curr_pos.setY(ev->pos().y()/(multiplikator_y*8));

        if (this->left_button_pressed)
        {
            this->sprite->set_bit(this->curr_pos.x(), this->curr_pos.y(), left_button == COLOR);
            this->update_view();
        }
        else if (this->right_button_pressed)
        {
            this->sprite->set_bit(this->curr_pos.x(), this->curr_pos.y(), right_button == COLOR);
            this->update_view();
        }

    }
    emit mouse_updated_cell_updated(this->curr_pos.x(), this->curr_pos.y());
}

void Editor::mousePressEvent(QMouseEvent *ev)
{
    if (this->sprite == 0)
        return;

    if (ev->button() == Qt::LeftButton)
    {
        this->left_button_pressed = true;
        if (multicol)
        {
            if (left_button_pressed)
            {
                if (this->left_button == TRANSPARENT)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
                }
                else if (this->left_button == COLOR)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
                }
                else if (this->left_button == MC1)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
                }
                else if (this->left_button == MC2)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
                }
                this->update_view();
            }
        }
        else
            this->sprite->set_bit(curr_pos.x(), curr_pos.y(), left_button == COLOR);
    }
    else if (ev->button() == Qt::RightButton)
    {
        this->right_button_pressed = true;
        if (multicol)
        {

            if (right_button_pressed)
            {
                if (this->right_button == TRANSPARENT)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
                }
                else if (this->right_button == COLOR)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), false);
                }
                else if (this->right_button == MC1)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), false);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
                }
                else if (this->right_button == MC2)
                {
                    this->sprite->set_bit(2*curr_pos.x(), curr_pos.y(), true);
                    this->sprite->set_bit(2*curr_pos.x()+1, curr_pos.y(), true);
                }
                this->update_view();
            }
        }
        else
            this->sprite->set_bit(curr_pos.x(), curr_pos.y(), right_button == COLOR);
    }
        this->update_view();
}

void Editor::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        this->left_button_pressed = false;
    if (ev->button() == Qt::RightButton)
        this->right_button_pressed = false;

    emit update_viewer();
}



void Editor::update_view()
{
    if (this->sprite == 0)
        return;

    QImage img(192*multiplikator_x,168*multiplikator_y,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    painter.fillRect(0,0,img.width(),img.height(), col_list.at(this->transparent_color));



    if (this->multicol)
    {
        int h = img.height()/21;
        int w = img.width()/12;
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 12; x++)
            {
                if (this->sprite->get_bit(2*x,y)== 1 && this->sprite->get_bit(2*x+1,y) == 0)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->sprite_color));
                }
                else if (this->sprite->get_bit(2*x,y) == 0 && this->sprite->get_bit(2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->mc1));
                }
                else if (this->sprite->get_bit(2*x,y) == 1 && this->sprite->get_bit(2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->mc2));
                }

            }
        }

    }
    else
    {
        int h = img.height()/21;
        int w = img.width()/24;
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (this->sprite->get_bit(x,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->sprite_color));
                }
            }
        }
    }

    if (this->gridlines)
    {
        for (int i = 1; i < (this->multicol ? 12 : 24); i++)
        {
            painter.drawLine(img.width()/(this->multicol ? 12.0 : 24.) * i,0,img.width()/(this->multicol ? 12.0 : 24.) * i, img.height());
        }
        for (int j = 1; j < 21; j++)
        {
            painter.drawLine(0,img.height()/21.0*j,img.width(), img.height()/21.0*j);
        }

    }

    painter.end();
    this->setPixmap(QPixmap::fromImage(img));
}

