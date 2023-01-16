#ifndef SPRITE_H
#define SPRITE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QList>
#include <QGraphicsSceneMouseEvent>


enum BUTTONS {TRANSPARENT, COLOR, MC1, MC2, OVERLAY_COLOR, OVERLAY_TRANSPARENT };

class SpriteView;
class Sprite;
struct options {
    int left_button = BUTTONS::COLOR;
    int right_button = BUTTONS::TRANSPARENT;
    int mc1;
    int mc2;
    int background;
    int current_sprite = 0;
    int sprite_num;
    int animations_num;
    QList<Sprite*> sprite_list;
    bool show_grid_lines;
    SpriteView *spriteview;
    QStringList col_names = {"Black","White","Red","Cyan","Purple","Green","Blue","Yellow","Orange","Brown",
                             "Pink", "Dark Grey", "Grey","Light Green","Light Blue","Light Grey"};
    QList<QColor> col_list = { QColor(0,0,0),
                               QColor(255,255,255),
                               QColor(0x92,0x4a,0x40),
                               QColor(0x84,0xc5,0xcc),
                               QColor(0x93,0x51,0xb6),
                               QColor(0x72,0xb1,0x4b),
                               QColor(0x48,0x3a,0xaa),
                               QColor(0xd5,0xdf,0x7c),
                               QColor(0x99,0x69,0x2d),
                               QColor(0x67,0x52,0x00),
                               QColor(0xc1,0x81,0x7a),
                               QColor(0x60,0x60,0x60),
                               QColor(0x8a,0x8a,0x8a),
                               QColor(0xb3,0xec,0x91),
                               QColor(0x86,0x7a,0xde),
                               QColor(0xb3,0xb3,0xb3)};
};

class Sprite : public QGraphicsItem
{
public:
    Sprite(options *opt);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;



    void change_tile(QPointF pos);

    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

    char get_bit(int x, int y);
    void set_bit(int x, int y, bool value);

    void slide_up(){
        bool tmp[24];

        for (int x = 0; x < 24; x++)
            tmp[x] = (0 != this->get_bit(x,0));
        for (int y = 0; y < 20; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                this->set_bit(x,y, (0 != this->get_bit(x,y+1)));
            }
        }
        for (int x = 0; x < 24; x++)
            this->set_bit(x,20, tmp[x] != 0);
    }
    void slide_down(){
        bool tmp[24];

        for (int x = 0; x < 24; x++)
            tmp[x] = (0 != this->get_bit(x,20));
        for (int y = 20; y > 0; y--)
        {
            for (int x = 0; x < 24; x++)
            {
                this->set_bit(x,y, (0 != this->get_bit(x,y-1)));
            }
        }
        for (int x = 0; x < 24; x++)
            this->set_bit(x,0, tmp[x] != 0);
    }
    void slide_left(){
        bool tmp[21];

        for (int y = 0; y < 21; y++)
            tmp[y] = (0 != this->get_bit(0,y));
        for (int x = 0; x < 23; x++)
        {
            for (int y = 0; y < 20; y++)
            {
                this->set_bit(x,y, (0 != this->get_bit(x+1,y)));
            }
        }
        for (int y = 0; y < 21; y++)
            this->set_bit(23,y, tmp[y] != 0);
    }
    void slide_right(){
        bool tmp[21];

        for (int y = 0; y < 21; y++)
            tmp[y] = (0 != this->get_bit(23,y));
        for (int x = 23; x > 0; x--)
        {
            for (int y = 0; y < 21; y++)
            {
                this->set_bit(x,y, (0 != this->get_bit(x-1,y)));
            }
        }
        for (int y = 0; y < 21; y++)
            this->set_bit(0,y, tmp[y] != 0);
    }


    int id;
    int sprite_color = 5;
    unsigned char sprite_data[64];
    bool expand_x = false;
    bool expand_y = false;
    bool overlay_next = false;
    bool multi_color_mode = true;


    bool left_pressed = false;
    bool right_pressed = false;

    options *opt;
};

#endif // SPRITE_H
