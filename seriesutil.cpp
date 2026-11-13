#include "seriesutil.h"
#include "sprite.h"

#include <QPainter>
#include <QSettings>

namespace SeriesUtil {

QPixmap preview(const options *opt, const QList<QJsonObject> &sprites, const QList<QJsonObject> &overlays)
{
    QSettings settings;
    const int spacing_x = settings.value("sprite_spacing_x").toInt();
    const int spacing_y = settings.value("sprite_spacing_y").toInt();
    const int per_row = qMax(1, settings.value("sprites_per_row").toInt());
    const int w = 10 * SpriteData::WIDTH;
    const int h = 10 * SpriteData::HEIGHT;

    QImage wallpaper(spacing_x + (w + spacing_x) * per_row,
                     spacing_y + (h + spacing_y) * (int(sprites.count()) / per_row + 1),
                     QImage::Format_RGB32);
    wallpaper.fill(QColor::fromRgb(QRgb(settings.value("bgcolor").toInt())));

    QPainter painter(&wallpaper);
    for (int i = 0; i < sprites.count(); i++) {
        const QJsonObject &sprite = sprites.at(i);
        const bool has_overlay = sprite.value("overlay_next").toBool() && i < overlays.count();
        painter.save();
        painter.translate(spacing_x + (w + spacing_x) * (i % per_row),
                          spacing_y + (h + spacing_y) * (i / per_row));
        SpriteData::render(&painter,
                           opt->col_list,
                           opt->data,
                           sprite,
                           has_overlay ? &overlays.at(i) : nullptr,
                           10,
                           10,
                           opt->show_grid_lines);
        painter.restore();
    }
    painter.end();

    return QPixmap::fromImage(wallpaper.scaledToHeight(wallpaper.height() / 2, Qt::SmoothTransformation));
}

bool insert(options *opt, int base_index, const QList<QJsonObject> &sprites, const QList<QJsonObject> &overlays)
{
    if (base_index < 0 || base_index >= opt->sprite_count())
        return false;

    const bool pairs = !overlays.isEmpty() && overlays.count() == sprites.count()
                       && base_index + 1 < opt->sprite_count();

    opt->push_undo();
    QJsonArray array = opt->data.take("sprites").toArray();
    for (int i = 1; i < sprites.count(); i++) {
        if (pairs) {
            array.insert(base_index + 2 * i, sprites.at(i));
            array.insert(base_index + 2 * i + 1, overlays.at(i));
        } else {
            array.insert(base_index + i, sprites.at(i));
        }
    }
    opt->data.insert("sprites", array);
    return true;
}

} // namespace SeriesUtil
