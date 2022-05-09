#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>

class Sprite : public QObject
{
    Q_OBJECT
public:
    explicit Sprite(QObject *parent = nullptr);

    unsigned char sprite_data[64];
    bool expand_x = false;
    bool expand_y = false;
    bool overlay_next = false;
    bool multi_color_mode = false;


signals:

private:

};

#endif // SPRITE_H
