#include "spritedata.h"

#include <QImage>
#include <QLineF>
#include <QPainter>
#include <QVarLengthArray>

namespace SpriteData {

// --- Bits -------------------------------------------------------------------

Bits Bits::from_json(const QJsonObject &sprite)
{
    Bits b;
    const QJsonArray rows = sprite.value("sprite_data").toArray();
    const int row_count = qMin<int>(rows.size(), HEIGHT);
    for (int y = 0; y < row_count; y++) {
        const QJsonArray row = rows.at(y).toArray();
        const int col_count = qMin<int>(row.size(), WIDTH);
        for (int x = 0; x < col_count; x++)
            b.px[y][x] = row.at(x).toInt() > 0;
    }
    return b;
}

QJsonArray Bits::to_json() const
{
    QJsonArray rows;
    for (int y = 0; y < HEIGHT; y++) {
        QJsonArray row;
        for (int x = 0; x < WIDTH; x++)
            row.append(px[y][x] ? 1 : 0);
        rows.append(row);
    }
    return rows;
}

Bits Bits::from_bytes(const char *data)
{
    Bits b;
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            b.px[y][x] = (quint8(data[3 * y + x / 8]) >> (7 - (x % 8))) & 1;
    return b;
}

QByteArray Bits::to_bytes() const
{
    QByteArray out(BYTES, '\0');
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            if (px[y][x])
                out[3 * y + x / 8] = char(quint8(out[3 * y + x / 8]) | (0x80 >> (x % 8)));
    return out;
}

// --- project / sprite access ------------------------------------------------

QJsonObject new_sprite(int color, bool mc_mode)
{
    QJsonObject sprite;
    sprite.insert("sprite_data", Bits().to_json());
    sprite.insert("mc_mode", mc_mode);
    sprite.insert("exp_x", false);
    sprite.insert("exp_y", false);
    sprite.insert("overlay_next", false);
    sprite.insert("sprite_color", color);
    return sprite;
}

QJsonObject new_animation(int from, int to, bool valid)
{
    QJsonObject anim;
    anim.insert("from", from);
    anim.insert("to", to);
    anim.insert("timer", 16);
    anim.insert("overlay", false);
    anim.insert("pingpong", false);
    anim.insert("valid", valid);
    return anim;
}

int sprite_count(const QJsonObject &data)
{
    return data.value("sprites").toArray().size();
}

QJsonObject sprite_at(const QJsonObject &data, int id)
{
    return data.value("sprites").toArray().at(id).toObject();
}

void replace_sprite(QJsonObject &data, int id, const QJsonObject &sprite)
{
    // take() avoids an additional reference, so the array is not copied needlessly
    QJsonArray sprites = data.take("sprites").toArray();
    if (id >= 0 && id < sprites.size())
        sprites.replace(id, sprite);
    data.insert("sprites", sprites);
}

void set_sprite_value(QJsonObject &data, int id, const QString &key, const QJsonValue &value)
{
    if (id < 0 || id >= sprite_count(data))
        return;
    QJsonObject sprite = sprite_at(data, id);
    sprite.insert(key, value);
    replace_sprite(data, id, sprite);
}

Bits bits_at(const QJsonObject &data, int id)
{
    return Bits::from_json(sprite_at(data, id));
}

void set_bits(QJsonObject &data, int id, const Bits &bits)
{
    set_sprite_value(data, id, "sprite_data", bits.to_json());
}

bool set_pixel(QJsonObject &data, int id, int x, int y, bool value)
{
    if (!Bits::inside(x, y) || id < 0 || id >= sprite_count(data))
        return false;

    QJsonObject sprite = sprite_at(data, id);
    QJsonArray rows = sprite.value("sprite_data").toArray();
    QJsonArray row = rows.at(y).toArray();
    if ((row.at(x).toInt() > 0) == value)
        return false;

    // repair rows that are too short (e.g. from damaged files)
    while (rows.size() < HEIGHT)
        rows.append(QJsonArray());
    while (row.size() < WIDTH)
        row.append(0);

    row.replace(x, value ? 1 : 0);
    rows.replace(y, row);
    sprite.insert("sprite_data", rows);
    replace_sprite(data, id, sprite);
    return true;
}

// --- binary -----------------------------------------------------------------

QJsonObject sprite_from_binary(const char *data, quint8 attributes)
{
    QJsonObject sprite;
    sprite.insert("sprite_data", Bits::from_bytes(data).to_json());
    sprite.insert("mc_mode", (attributes & 0x80) != 0);
    sprite.insert("exp_x", (attributes & 0x40) != 0);
    sprite.insert("exp_y", (attributes & 0x20) != 0);
    sprite.insert("overlay_next", (attributes & 0x10) != 0);
    sprite.insert("sprite_color", attributes & 0x0F);
    return sprite;
}

quint8 spritepad_attributes(const QJsonObject &sprite)
{
    quint8 attr = quint8(sprite.value("sprite_color").toInt() & 0x0F);
    if (sprite.value("mc_mode").toBool())
        attr |= 0x80;
    if (sprite.value("exp_x").toBool())
        attr |= 0x40;
    if (sprite.value("exp_y").toBool())
        attr |= 0x20;
    if (sprite.value("overlay_next").toBool())
        attr |= 0x10;
    return attr;
}

// --- rendering --------------------------------------------------------------

QColor color(const QList<QColor> &palette, int index)
{
    if (palette.isEmpty())
        return QColor(Qt::black);
    return palette.at(qBound(0, index, int(palette.size()) - 1));
}

// Builds a tiny image with one pixel per sprite pixel (12 or 24 wide).
// If transparent_background is set, background pixels stay transparent (used for overlays).
static QImage sprite_image(const QList<QColor> &palette,
                           const QJsonObject &data,
                           const QJsonObject &sprite,
                           bool transparent_background)
{
    const Bits bits = Bits::from_json(sprite);
    const bool mc = sprite.value("mc_mode").toBool();

    const QRgb bg = transparent_background ? qRgba(0, 0, 0, 0)
                                           : color(palette, data.value("background").toInt()).rgb();
    const QRgb lut[4] = {
        bg,
        color(palette, data.value("mc1").toInt()).rgb(),
        color(palette, sprite.value("sprite_color").toInt()).rgb(),
        color(palette, data.value("mc2").toInt()).rgb(),
    };

    const int w = mc ? MC_WIDTH : WIDTH;
    QImage img(w, HEIGHT, transparent_background ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    for (int y = 0; y < HEIGHT; y++) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < w; x++)
            line[x] = mc ? lut[bits.mc(x, y)] : (bits.px[y][x] ? lut[MC_SPRITE] : bg);
    }
    return img;
}

void render(QPainter *painter,
            const QList<QColor> &palette,
            const QJsonObject &data,
            const QJsonObject &sprite,
            const QJsonObject *overlay,
            qreal cw,
            qreal ch,
            bool grid)
{
    const QRectF target(0, 0, WIDTH * cw, HEIGHT * ch);

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawImage(target, sprite_image(palette, data, sprite, false));
    if (overlay)
        painter->drawImage(target, sprite_image(palette, data, *overlay, true));
    painter->restore();

    if (grid)
        render_grid(painter, sprite.value("mc_mode").toBool(), cw, ch);
}

void render_grid(QPainter *painter, bool mc_mode, qreal cw, qreal ch)
{
    QVarLengthArray<QLineF, HEIGHT + WIDTH> lines;
    for (int y = 0; y < HEIGHT; y++)
        lines.append(QLineF(0, ch * y, WIDTH * cw, ch * y));
    for (int x = 0; x < WIDTH; x += (mc_mode ? 2 : 1))
        lines.append(QLineF(cw * x, 0, cw * x, HEIGHT * ch));

    QPen pen(Qt::gray);
    pen.setWidthF(0.5);
    painter->save();
    painter->setPen(pen);
    painter->drawLines(lines.constData(), int(lines.size()));
    painter->restore();
}

} // namespace SpriteData
