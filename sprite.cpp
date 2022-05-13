#include "sprite.h"

Sprite::Sprite(QObject *parent) : QObject(parent)
{
    for (int i = 0; i < 64; i++)
        this->sprite_data[i] = 0;
}

char Sprite::get_bit(int x, int y)
{
    char val = sprite_data[3*y + x/8];
    int pos = x%8;
    return (val&(0x01 << (7-pos))) >> (7-pos);
}

void Sprite::set_bit(int x, int y, bool value)
{
    int pos = x%8;
    if (value)
        sprite_data[3*y + x/8] |= (0x01 << (7-pos));
    else
        sprite_data[3*y + x/8] &= ~(0x01 << (7-pos));
}

