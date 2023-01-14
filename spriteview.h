#ifndef SPRITEVIEW_H
#define SPRITEVIEW_H

#include <QGraphicsView>
#include <QWidget>

struct options;

class SpriteView : public QGraphicsView
{
    Q_OBJECT
public:
    SpriteView(QWidget *parent);
    void set_opt(options *opt) { this->opt = opt; }
    void redraw();

    void change_current_sprite(int id) { emit current_sprite_changed(id); }

signals:
    void current_sprite_changed(int id);

private:
    options *opt;

};

#endif // SPRITEVIEW_H
