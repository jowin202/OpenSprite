#ifndef PALETTE_H
#define PALETTE_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>

#include <QDebug>

class Palette : public QLabel
{
    Q_OBJECT
public:
    Palette(QWidget *parent = 0);
    void showPalette();

    void mousePressEvent(QMouseEvent *ev);

    QList<QColor> col_list;
    QStringList col_names;

signals:
    void palette_clicked(int,int);



private:
    int width = 352;
    int height = 56;



};

#endif // PALETTE_H
