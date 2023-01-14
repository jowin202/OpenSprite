#ifndef PALETTE_H
#define PALETTE_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>

#include <QDebug>

struct options;

class Palette : public QLabel
{
    Q_OBJECT
public:
    Palette(QWidget *parent = 0);
    void showPalette(options *opt);

    void mousePressEvent(QMouseEvent *ev);

signals:
    void palette_clicked(int,int);



private:
    int width = 352;
    int height = 56;
    options *opt;



};

#endif // PALETTE_H
