#ifndef SPRITEVIEW_H
#define SPRITEVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QWheelEvent>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct options;

class SpriteView : public QGraphicsView
{
    Q_OBJECT
public:
    SpriteView(QWidget *parent);
    void set_opt(options *opt) { this->opt = opt; }
    void redraw();

    void change_current_sprite(int id) { emit current_sprite_changed(id); }

    void wheelEvent(QWheelEvent *event) override;


signals:
    void current_sprite_changed(int id);
    void zoom_in();
    void zoom_out();

private:
    options *opt;

};

#endif // SPRITEVIEW_H
