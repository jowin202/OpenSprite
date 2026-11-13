#ifndef SPRITE_H
#define SPRITE_H

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QPainter>
#include <QSettings>
#include <QStringList>

#include "spritedata.h"

enum BUTTONS { TRANSPARENT, COLOR, MC1, MC2, OVERLAY_COLOR, OVERLAY_TRANSPARENT };
enum PEN { DEFAULT, FLOOD };

class SpriteView;
class Sprite;

struct options
{
    static constexpr int MAX_UNDO = 1000;

    int left_button = BUTTONS::COLOR;
    int right_button = BUTTONS::TRANSPARENT;
    int pen = PEN::DEFAULT;
    QJsonObject data;
    int selection_from = 0;
    int selection_to = 0;
    QList<Sprite *> sprite_list;
    SpriteView *spriteview = nullptr;
    bool show_grid_lines = true;
    bool show_numbers = false;
    QColor selection_color = QColor(0x00, 0xff, 0x00); // cached from QSettings

    //auto export
    QString last_exported_file;
    int export_address = 0x3000;
    int export_file_format = 0;      // 0 = prg, 1 = bin
    int export_attribute_format = 0; // 0 = none, 1 = spritepad, 2 = seuck
    //auto save
    QString last_saved_file;

    //change detection: state of the last load/save
    QJsonObject saved_data;
    bool is_modified() const { return data != saved_data; }
    void mark_saved() { saved_data = data; }

    //undo / redo
    QList<QJsonObject> undoDB;
    QList<QJsonObject> redoDB;

    // store current state before a single-step operation
    void push_undo()
    {
        end_edit();
        push_limited(undoDB, data);
        redoDB.clear();
    }
    // start of an interactive edit (mouse press); only recorded if something changes
    void begin_edit()
    {
        end_edit();
        pending_undo = data;
        edit_pending = true;
    }
    void end_edit()
    {
        if (edit_pending && pending_undo != data) {
            push_limited(undoDB, pending_undo);
            redoDB.clear();
        }
        edit_pending = false;
        pending_undo = QJsonObject();
    }
    bool undo()
    {
        end_edit();
        if (undoDB.isEmpty())
            return false;
        push_limited(redoDB, data);
        data = undoDB.takeLast();
        return true;
    }
    bool redo()
    {
        end_edit();
        if (redoDB.isEmpty())
            return false;
        push_limited(undoDB, data);
        data = redoDB.takeLast();
        return true;
    }
    void clear_history()
    {
        undoDB.clear();
        redoDB.clear();
        edit_pending = false;
        pending_undo = QJsonObject();
    }

    int sprite_count() const { return SpriteData::sprite_count(data); }
    void clamp_selection()
    {
        const int last = qMax(0, sprite_count() - 1);
        selection_from = qBound(0, selection_from, last);
        selection_to = qBound(selection_from, selection_to, last);
    }

    QStringList col_names = {"Black", "White", "Red", "Cyan", "Purple", "Green", "Blue", "Yellow",
                             "Orange", "Brown", "Pink", "Dark Grey", "Grey", "Light Green",
                             "Light Blue", "Light Grey"};
    QList<QColor> col_list = {QColor(0, 0, 0),
                              QColor(255, 255, 255),
                              QColor(0x92, 0x4a, 0x40),
                              QColor(0x84, 0xc5, 0xcc),
                              QColor(0x93, 0x51, 0xb6),
                              QColor(0x72, 0xb1, 0x4b),
                              QColor(0x48, 0x3a, 0xaa),
                              QColor(0xd5, 0xdf, 0x7c),
                              QColor(0x99, 0x69, 0x2d),
                              QColor(0x67, 0x52, 0x00),
                              QColor(0xc1, 0x81, 0x7a),
                              QColor(0x60, 0x60, 0x60),
                              QColor(0x8a, 0x8a, 0x8a),
                              QColor(0xb3, 0xec, 0x91),
                              QColor(0x86, 0x7a, 0xde),
                              QColor(0xb3, 0xb3, 0xb3)};

private:
    QJsonObject pending_undo;
    bool edit_pending = false;

    static void push_limited(QList<QJsonObject> &list, const QJsonObject &obj)
    {
        list.append(obj);
        if (list.size() > MAX_UNDO)
            list.removeFirst();
    }
};

class Sprite : public QGraphicsItem
{
public:
    Sprite(options *opt, int id);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;

    void change_tile(QPointF pos);
    void flood_fill(QPointF pos);

    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

    int get_id() const { return id; }
    QJsonObject object() const { return SpriteData::sprite_at(opt->data, id); }
    bool is_mc() const { return object().value("mc_mode").toBool(); }

    bool get_bit(int x, int y) const;
    void set_bit(int x, int y, bool value);
    SpriteData::Bits bits() const { return SpriteData::bits_at(opt->data, id); }
    void set_bits(const SpriteData::Bits &bits) { SpriteData::set_bits(opt->data, id, bits); }

    // Used by QuickEditorWidget to simulate mouse button state
    void set_left_pressed(bool v) { left_pressed = v; }
    void set_right_pressed(bool v) { right_pressed = v; }

    // whole-sprite operations
    void slide_up();
    void slide_down();
    void slide_left();
    void slide_right();
    void flip_left();
    void flip_top();
    void reflect_top();
    void reflect_left();
    void switch_col_to_mc1();
    void switch_col_to_mc2();
    void switch_mc1_to_mc2();

private:
    // size of one hires pixel in item coordinates (expansion keeps the aspect ratio)
    qreal cell_w(const QJsonObject &sprite) const;
    qreal cell_h(const QJsonObject &sprite) const;
    int active_button() const;
    void update_neighbours();
    void swap_mc_colors(int a, int b);

    int id;
    bool left_pressed = false;
    bool right_pressed = false;
    options *opt;
};

#endif // SPRITE_H
