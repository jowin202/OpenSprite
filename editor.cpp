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

    this->updateView();
}

void Editor::mouseMoveEvent(QMouseEvent *ev)
{
    if (ev->pos().x() > this->pixmap()->width() || ev->pos().x() < 0)
        return;
    if (ev->pos().y() > this->pixmap()->height() || ev->pos().y() < 0)
        return;



    if (this->multicol)
    {
        this->curr_pos.setX(ev->pos().x()/(multiplikator_x*16));
        this->curr_pos.setY(ev->pos().y()/(multiplikator_y*8));
    }
    else
    {
        this->curr_pos.setX(ev->pos().x()/(multiplikator_x*8));
        this->curr_pos.setY(ev->pos().y()/(multiplikator_y*8));

        if (this->left_button_pressed)
        {
            this->set_bit(this->curr_pos.x(), this->curr_pos.y(), left_button == COLOR);
            this->updateView();
        }
        else if (this->right_button_pressed)
        {
            this->set_bit(this->curr_pos.x(), this->curr_pos.y(), right_button == COLOR);
            this->updateView();
        }

    }
    emit mouse_updated_cell_updated(this->curr_pos.x(), this->curr_pos.y());
}

void Editor::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        this->left_button_pressed = true;
        this->set_bit(curr_pos.x(), curr_pos.y(), left_button == COLOR);
    }
    else if (ev->button() == Qt::RightButton)
    {
        this->right_button_pressed = true;
        this->set_bit(curr_pos.x(), curr_pos.y(), right_button == COLOR);
    }
        this->updateView();
}

void Editor::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        this->left_button_pressed = false;
    if (ev->button() == Qt::RightButton)
        this->right_button_pressed = false;
}



void Editor::updateView()
{
    QImage img(192*multiplikator_x,168*multiplikator_y,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    painter.fillRect(0,0,img.width(),img.height(), col_list.at(this->transparent_color));
    if (this->gridlines)
    {
        /*
        if (!this->multicol)
        {
            for (int i = 0; i < 12; i++)
            {
                painter.drawLine(multiplikator_x*(7+16*i),0,multiplikator_x*(7+16*i),img.height());
            }
        }
        for (int i = 1; i < 12; i++)
        {
            painter.drawLine(multiplikator_x*(-1+16*i),0,multiplikator_x*(-1+16*i),img.height());
        }
        for (int j = 1; j < 21; j++)
        {
            painter.drawLine(0,-1+multiplikator_y*8*j,img.width(), -1+multiplikator_y*8*j);
        }
        */


        for (int i = 1; i < (this->multicol ? 12 : 24); i++)
        {
            painter.drawLine(img.width()/(this->multicol ? 12.0 : 24.) * i,0,img.width()/(this->multicol ? 12.0 : 24.) * i, img.height());
        }
        for (int j = 1; j < 21; j++)
        {
            painter.drawLine(0,img.height()/21.0*j,img.width(), img.height()/21.0*j);
        }

    }

    if (this->multicol)
    {

    }
    else
    {
        int h = img.height()/21;
        int w = img.width()/24;
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (get_bit(x,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w,h,this->col_list.at(this->sprite_color));
                }
            }
        }
    }

    painter.end();
    this->setPixmap(QPixmap::fromImage(img));
}

