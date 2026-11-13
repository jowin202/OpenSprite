#ifndef SERIESUTIL_H
#define SERIESUTIL_H

#include <QJsonObject>
#include <QList>
#include <QPixmap>

struct options;

// Shared helpers for dialogs that generate a series of sprites (rotation, scaling).
namespace SeriesUtil {

// Preview of all generated sprites, laid out like the main view (half size).
QPixmap preview(const options *opt, const QList<QJsonObject> &sprites, const QList<QJsonObject> &overlays);

// Inserts the generated sprites (except the first one, which is the original)
// behind base_index. If overlays are given, sprite/overlay pairs are inserted.
// Returns false if base_index is no longer valid.
bool insert(options *opt, int base_index, const QList<QJsonObject> &sprites, const QList<QJsonObject> &overlays);

// Collapses a fat-pixel color count to the most frequent color index (0-3).
inline int most_frequent(const int count[4])
{
    int best = 0;
    for (int c = 1; c < 4; c++)
        if (count[c] > count[best])
            best = c;
    return best;
}

} // namespace SeriesUtil

#endif // SERIESUTIL_H
