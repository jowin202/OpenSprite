#ifndef FILEIO_H
#define FILEIO_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class QWidget;

// Reading and writing of the supported file formats.
// Read functions return an empty object on error (after showing a message).
// Write functions return false on error (after showing a message).
class FileIO
{
public:
    static QJsonObject read_spd(const QString &path, QWidget *parent = nullptr);
    static bool write_spd(const QString &path, const QJsonObject &file_obj, QWidget *parent = nullptr);

    // address < 0: no load address (bin file)
    // attribute: 0 = none, 1 = spritepad, 2 = seuck
    static bool write_prg(const QString &path,
                          const QJsonObject &file_obj,
                          int address,
                          int attribute,
                          QWidget *parent = nullptr);
    static QJsonObject read_prg_bin(const QString &path, int *address, QWidget *parent = nullptr);

    static QJsonObject read_spm(const QString &path, QWidget *parent = nullptr);

    static constexpr int SPD_V1_MAX_SPRITES = 256;
};

#endif // FILEIO_H
