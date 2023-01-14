#include "palette.h"
#include "sprite.h"

Palette::Palette(QWidget *parent) : QLabel(parent)
{
}

void Palette::showPalette(options *opt)
{
    this->opt = opt;
    QImage img(this->width,this->height,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    for (int i = 0; i < 8; i++)
    {
        painter.fillRect(44*i,0,44,28, opt->col_list.at(i));
        painter.fillRect(44*i,28,44,28, opt->col_list.at(8+i));
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

    color_pos += ev->pos().x()/44;
    color_pos += 8*(ev->pos().y()/28);

    emit palette_clicked(ev->button(),color_pos);

}
