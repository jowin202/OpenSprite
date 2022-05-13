#include "palette.h"

Palette::Palette(QWidget *parent)
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

    col_names << "Black";
    col_names << "White";
    col_names << "Red";
    col_names << "Cyan";
    col_names << "Purple";
    col_names << "Green";
    col_names << "Blue";
    col_names << "Yellow";
    col_names << "Orange";
    col_names << "Brown";
    col_names << "Pink";
    col_names << "Dark Grey";
    col_names << "Grey";
    col_names << "Light Green";
    col_names << "Light Blue";
    col_names << "Light Grey";
}

void Palette::showPalette()
{
    QImage img(this->width,this->height,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    for (int i = 0; i < 8; i++)
    {
        painter.fillRect(44*i,0,44,28, col_list.at(i));
        painter.fillRect(44*i,28,44,28, col_list.at(8+i));
    }
    painter.end();

    this->setPixmap(QPixmap::fromImage(img));
}

void Palette::mousePressEvent(QMouseEvent *ev)
{
    if (ev->pos().x() < 0 || ev->pos().x() > this->width)
        return;
    if (ev->pos().y() < 0 || ev->pos().y() > this->height)
        return;

    int color_pos = 0;
    int mouse_button = 0; //left
    if (ev->button() == Qt::RightButton)
        mouse_button = 1; //right

    color_pos += ev->pos().x()/44;
    color_pos += 8*(ev->pos().y()/28);

    emit palette_clicked(mouse_button,color_pos);

}
