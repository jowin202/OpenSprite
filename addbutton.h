#ifndef ADDBUTTON_H
#define ADDBUTTON_H

#include <QGraphicsObject>
#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QMouseEvent>

class AddButton : public QGraphicsObject
{
    Q_OBJECT
public:

    AddButton();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;

    void mousePressEvent(QGraphicsSceneMouseEvent *e) override;

signals:
    void clicked();
};

#endif // ADDBUTTON_H
