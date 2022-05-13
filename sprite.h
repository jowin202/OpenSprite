#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>

class Sprite : public QObject
{
    Q_OBJECT
public:
    explicit Sprite(QObject *parent = nullptr);

    char get_bit(int x, int y);
    void set_bit(int x, int y, bool value);

    int sprite_color = 5;
    unsigned char sprite_data[64];
    bool expand_x = false;
    bool expand_y = false;
    bool overlay_next = false;
    bool multi_color_mode = true;



signals:

private:

};

#endif // SPRITE_H
