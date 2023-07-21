#include "addbutton.h"

AddButton::AddButton()
{

}

void AddButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->drawRect(QRectF(0,0,10*24, 10*21));
    painter->drawImage(QRectF(0,0,10*24, 10*21), QImage(":/icons/plus.svg"));
}

QRectF AddButton::boundingRect() const
{
    return QRectF(0,0,10*24, 10*21);
}

void AddButton::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    Q_UNUSED(e);
    emit clicked();
}
