#ifndef ANIMATIONSVIEW_H
#define ANIMATIONSVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QAction>

struct options;

class AnimationsView : public QGraphicsView
{
    Q_OBJECT
public:
    AnimationsView(QWidget *parent);
    void set_opt(options *opt) { this->opt = opt; }
    void redraw();


public slots:
    void add_new_animation();

private:

    QAction action_del;
    options *opt;
};

#endif // ANIMATIONSVIEW_H
