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
    if (ev->pos().x() > 192 || ev->pos().x() < 0)
        return;
    if (ev->pos().y() > 168 || ev->pos().y() < 0)
        return;

    if (this->multicol)
    {
        this->curr_pos.setX(ev->pos().x()/16);
        this->curr_pos.setY(ev->pos().y()/8);
    }
    else
    {
        this->curr_pos.setX(ev->pos().x()/32);
        this->curr_pos.setY(ev->pos().y()/8);
    }
}

void Editor::mousePressEvent(QMouseEvent *ev)
{

}

void Editor::mouseReleaseEvent(QMouseEvent *ev)
{

}

void Editor::updateView()
{
    QImage img(192,168,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    painter.fillRect(0,0,img.width(),img.height(), col_list.at(this->transparent_color));
    if (this->gridlines)
    {
        if (!this->multicol)
        {
            for (int i = 0; i < 12; i++)
            {
                painter.drawLine(7+16*i,0,7+16*i,img.height());
            }
        }
        for (int i = 1; i < 12; i++)
        {
            painter.drawLine(-1+16*i,0,-1+16*i,img.height());
        }
        for (int j = 1; j < 21; j++)
        {
            painter.drawLine(0,-1+8*j,img.width(), -1+8*j);
        }
    }
    painter.end();
    this->setPixmap(QPixmap::fromImage(img));
}

