#ifndef ANIMATIONITEM_H
#define ANIMATIONITEM_H

#include <QGraphicsObject>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QTimer>

#include <QMouseEvent>

class options;

class AnimationItem : public QGraphicsObject
{
    Q_OBJECT
public:
    AnimationItem(options *opt, int id);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    QImage draw_sprite(int sprite_id);
    int get_sprite_bit(int sprite_id, int x, int y);

    int animation_id;
    options *opt;

    int from = 0;
    int to = 0;
    int timer = 0;
    bool valid = true;
    bool overlay = false;
    bool pingpong = false;

    QList<QImage> animation_images;

signals:
    void trigger_redraw();
};

#endif // ANIMATIONITEM_H
