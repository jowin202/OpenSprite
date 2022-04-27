#include "editorlabel.h"

EditorLabel::EditorLabel(QWidget *parent) : QLabel(parent)
{
    this->updateView();
}

void EditorLabel::mouseMoveEvent(QMouseEvent *ev)
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
    qDebug() << this->curr_pos;
}

void EditorLabel::mousePressEvent(QMouseEvent *ev)
{

}

void EditorLabel::mouseReleaseEvent(QMouseEvent *ev)
{

}

void EditorLabel::updateView()
{
    QImage img(192,168,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    painter.fillRect(0,0,img.width(),img.height(), Qt::green);
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
