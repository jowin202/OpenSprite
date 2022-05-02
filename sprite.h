#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>

class Sprite : public QObject
{
    Q_OBJECT
public:
    explicit Sprite(QObject *parent = nullptr);

signals:

private:
    char sprite_data[64];
    bool expand_x = false;
    bool expand_y = false;
    bool overlay_next = false;
    bool multi_color_mode = false;

};

#endif // SPRITE_H
