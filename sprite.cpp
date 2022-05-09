#include "sprite.h"

Sprite::Sprite(QObject *parent) : QObject(parent)
{
    for (int i = 0; i < 64; i++)
        this->sprite_data[i] = 0;
}
