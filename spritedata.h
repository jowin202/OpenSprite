#ifndef SPRITEDATA_H
#define SPRITEDATA_H

#include <QByteArray>
#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <array>

class QPainter;

// Central helpers for the JSON based sprite model.
//
// A project is a QJsonObject with the keys
//   "background", "mc1", "mc2"  : color indices (0-15)
//   "sprites"                   : array of sprite objects
//   "animations"                : array of animation objects
// A sprite object holds "sprite_data" (21 rows of 24 ints 0/1), "mc_mode",
// "exp_x", "exp_y", "overlay_next" and "sprite_color".
namespace SpriteData {

constexpr int WIDTH = 24;    // hires pixels per row
constexpr int HEIGHT = 21;   // rows
constexpr int MC_WIDTH = 12; // multicolor (fat) pixels per row
constexpr int BYTES = 63;    // bytes of bitmap data per sprite

// Multicolor pixel value = left bit * 2 + right bit (as on the C64)
enum McPixel { MC_BACKGROUND = 0, MC_MC1 = 1, MC_SPRITE = 2, MC_MC2 = 3 };

struct Bits
{
    std::array<std::array<bool, WIDTH>, HEIGHT> px{};

    static bool inside(int x, int y) { return x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT; }
    bool get(int x, int y) const { return inside(x, y) && px[y][x]; }
    void set(int x, int y, bool v)
    {
        if (inside(x, y))
            px[y][x] = v;
    }

    // multicolor access, cx = 0..11
    int mc(int cx, int y) const { return (get(2 * cx, y) ? 2 : 0) | (get(2 * cx + 1, y) ? 1 : 0); }
    void set_mc(int cx, int y, int v)
    {
        set(2 * cx, y, (v & 2) != 0);
        set(2 * cx + 1, y, (v & 1) != 0);
    }

    bool operator==(const Bits &o) const { return px == o.px; }
    bool operator!=(const Bits &o) const { return px != o.px; }

    static Bits from_json(const QJsonObject &sprite);
    QJsonArray to_json() const;

    static Bits from_bytes(const char *data); // reads BYTES bytes
    QByteArray to_bytes() const;              // returns BYTES bytes
};

// --- project / sprite access ------------------------------------------------
QJsonObject new_sprite(int color = 5, bool mc_mode = true);
QJsonObject new_animation(int from, int to, bool valid);

int sprite_count(const QJsonObject &data);
QJsonObject sprite_at(const QJsonObject &data, int id);
void replace_sprite(QJsonObject &data, int id, const QJsonObject &sprite);
void set_sprite_value(QJsonObject &data, int id, const QString &key, const QJsonValue &value);

Bits bits_at(const QJsonObject &data, int id);
void set_bits(QJsonObject &data, int id, const Bits &bits);
// sets a single pixel; returns true if the pixel actually changed
bool set_pixel(QJsonObject &data, int id, int x, int y, bool value);

// --- binary (SpritePad / PRG) -----------------------------------------------
// attribute byte: bit7 multicolor, bit6 expand x, bit5 expand y, bit4 overlay, bit0-3 color
QJsonObject sprite_from_binary(const char *data, quint8 attributes);
quint8 spritepad_attributes(const QJsonObject &sprite);

// --- rendering --------------------------------------------------------------
QColor color(const QList<QColor> &palette, int index);

// Renders a sprite (and optionally the overlay sprite on top) into the rectangle
// (0, 0, 24 * cw, 21 * ch). cw/ch = size of one hires pixel.
void render(QPainter *painter,
            const QList<QColor> &palette,
            const QJsonObject &data,
            const QJsonObject &sprite,
            const QJsonObject *overlay,
            qreal cw,
            qreal ch,
            bool grid);

void render_grid(QPainter *painter, bool mc_mode, qreal cw, qreal ch);

} // namespace SpriteData

#endif // SPRITEDATA_H
