#include "fileio.h"
#include "spritedata.h"

#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSaveFile>

namespace {

// Bounds checked little endian reader. Reading past the end yields 0 and sets ok = false.
class Reader
{
public:
    explicit Reader(const QByteArray &buf)
        : buf(buf)
    {}

    int u8()
    {
        if (pos >= buf.size()) {
            ok = false;
            return 0;
        }
        return quint8(buf.at(pos++));
    }
    int u16()
    {
        int lo = u8();
        int hi = u8();
        return lo | (hi << 8);
    }
    void skip(qsizetype n)
    {
        if (n < 0 || pos + n > buf.size()) {
            ok = false;
            pos = buf.size();
        } else
            pos += n;
    }
    // returns nullptr if not enough data is left
    const char *read(qsizetype n)
    {
        if (pos + n > buf.size()) {
            ok = false;
            pos = buf.size();
            return nullptr;
        }
        const char *p = buf.constData() + pos;
        pos += n;
        return p;
    }
    void skip_string()
    {
        while (pos < buf.size() && buf.at(pos) != '\0')
            pos++;
        if (pos < buf.size())
            pos++; // terminator
    }
    bool at_end() const { return pos >= buf.size(); }

    bool ok = true;

private:
    const QByteArray &buf;
    qsizetype pos = 0;
};

bool read_file(const QString &path, QByteArray *content, QWidget *parent)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(parent,
                              QObject::tr("Cannot open file"),
                              QObject::tr("Cannot open file \"%1\":\n%2").arg(path, file.errorString()));
        return false;
    }
    *content = file.readAll();
    return true;
}

bool commit_file(QSaveFile &file, const QString &path, QWidget *parent)
{
    if (!file.commit()) {
        QMessageBox::critical(parent,
                              QObject::tr("Cannot write file"),
                              QObject::tr("Cannot write file \"%1\":\n%2").arg(path, file.errorString()));
        return false;
    }
    return true;
}

bool open_for_write(QSaveFile &file, const QString &path, QWidget *parent)
{
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(parent,
                              QObject::tr("Cannot write file"),
                              QObject::tr("Cannot write file \"%1\":\n%2").arg(path, file.errorString()));
        return false;
    }
    return true;
}

void corrupt_file_message(const QString &path, QWidget *parent)
{
    QMessageBox::critical(parent,
                          QObject::tr("Invalid file"),
                          QObject::tr("The file \"%1\" is damaged or not a valid sprite file.").arg(path));
}

} // namespace

QJsonObject FileIO::read_spd(const QString &path, QWidget *parent)
{
    QByteArray content;
    if (!read_file(path, &content, parent))
        return QJsonObject();

    Reader in(content);
    if (content.left(3) != QByteArray("SPD")) {
        QMessageBox::critical(parent,
                              QObject::tr("Invalid SpritePad Format"),
                              QObject::tr("Invalid SpritePad Format"));
        return QJsonObject();
    }
    in.skip(3);

    QJsonObject file_obj;
    bool tiles_exists = false;
    bool sprite_anim_exists = false;
    int version = in.u8();
    file_obj.insert("version", version);

    if (version >= 2) {
        int flags = in.u8();
        tiles_exists = flags & 0x01;
        sprite_anim_exists = flags & 0x02;
        // flags & 0x04: tile animations (not supported)
    }

    int sprite_num = 0;
    int tiles_num = 0;
    int animations_num = 0;

    if (version <= 1) {
        sprite_num = in.u8() + 1;
        animations_num = in.u8() + 1;
    } else if (version == 2) {
        sprite_num = in.u16();
        tiles_num = in.u16();
        animations_num = in.u16();
    } else { //version >= 3
        sprite_num = in.u16();
        tiles_num = in.u16();
        animations_num = in.u8() + 1;
        in.u8(); // number of tile animations - 1 (not supported)
    }

    int tile_width = 0;
    int tile_height = 0;
    if (version >= 2) {
        tile_width = in.u8();  //1-4 for version 2, else 1-8
        tile_height = in.u8(); //1-4 for version 2, else 1-8
    }

    file_obj.insert("background", in.u8() & 0x0F);
    file_obj.insert("mc1", in.u8() & 0x0F);
    file_obj.insert("mc2", in.u8() & 0x0F);

    if (version >= 4) {
        in.u16(); // sprite overlay distance (not supported)
        in.u16(); // tile overlay distance (not supported)
    }

    if (!in.ok) {
        corrupt_file_message(path, parent);
        return QJsonObject();
    }

    //SPRITE DATA (same in all versions)
    QJsonArray sprites;
    for (int i = 0; i < sprite_num; i++) {
        const char *bytes = in.read(SpriteData::BYTES);
        int attributes = in.u8();
        if (!in.ok)
            break;
        sprites.append(SpriteData::sprite_from_binary(bytes, quint8(attributes)));
    }
    file_obj.insert("sprites", sprites);

    if (!in.ok) {
        QMessageBox::warning(parent,
                             QObject::tr("File truncated"),
                             QObject::tr("The file is incomplete. %1 of %2 sprites could be loaded.")
                                 .arg(sprites.count())
                                 .arg(sprite_num));
        file_obj.insert("animations", QJsonArray());
        return file_obj;
    }

    // tile data is currently dropped
    //TILESP_DATA (version 2-5)
    if (version >= 2 && tiles_exists)
        in.skip(qsizetype(tiles_num) * tile_width * tile_height * 2);
    //TILESP_ATTR (version 3-5)
    if (version >= 3 && tiles_exists)
        in.skip(tiles_num);
    //TILESP_TAGS (version 5)
    if (version >= 5)
        in.skip(tiles_num);
    //TILESP_NAME: String zero terminated
    if (version >= 5)
        for (int i = 0; i < tiles_num && !in.at_end(); i++)
            in.skip_string();

    //ANIMATIONS (all versions, but from-to has 2 bytes in version 2-5)
    QJsonArray animations;
    if (in.ok && (version <= 1 || sprite_anim_exists)) {
        QList<int> start, end, timer, attr;
        for (int i = 0; i < animations_num; i++)
            start.append(version < 2 ? in.u8() : in.u16());
        for (int i = 0; i < animations_num; i++)
            end.append(version < 2 ? in.u8() : in.u16());
        for (int i = 0; i < animations_num; i++)
            timer.append(in.u8());
        for (int i = 0; i < animations_num; i++)
            attr.append(in.u8());

        if (in.ok) {
            for (int i = 0; i < animations_num; i++) {
                QJsonObject animation;
                animation.insert("from", start.at(i));
                animation.insert("to", end.at(i));
                animation.insert("timer", timer.at(i));
                animation.insert("pingpong", (attr.at(i) & 0x10) != 0);
                animation.insert("overlay", (attr.at(i) & 0x20) != 0);
                animation.insert("valid", (attr.at(i) & 0x80) != 0);
                animations.append(animation);
            }
        }
    }
    //TODO: tile animations omitted
    file_obj.insert("animations", animations);

    return file_obj;
}

bool FileIO::write_spd(const QString &path, const QJsonObject &file_obj, QWidget *parent)
{
    const QJsonArray sprites = file_obj.value("sprites").toArray();
    QJsonArray animations = file_obj.value("animations").toArray();

    if (sprites.isEmpty() || sprites.count() > SPD_V1_MAX_SPRITES) {
        QMessageBox::critical(parent,
                              QObject::tr("Cannot save"),
                              QObject::tr("The SPD format supports 1 to %1 sprites, the project has %2.\n"
                                          "Please use Export (PRG/BIN) for larger projects.")
                                  .arg(SPD_V1_MAX_SPRITES)
                                  .arg(sprites.count()));
        return false;
    }
    // SpritePad needs at least one animation; the count is stored as count-1
    if (animations.isEmpty())
        animations.append(SpriteData::new_animation(0, 0, false));
    while (animations.count() > 256)
        animations.removeLast();

    QByteArray out;
    out.reserve(9 + sprites.count() * 64 + animations.count() * 4);
    out.append("SPD");
    out.append(char(1)); // version
    out.append(char(sprites.count() - 1));
    out.append(char(animations.count() - 1));
    out.append(char(file_obj.value("background").toInt()));
    out.append(char(file_obj.value("mc1").toInt()));
    out.append(char(file_obj.value("mc2").toInt()));

    for (const QJsonValue &v : sprites) {
        const QJsonObject sprite = v.toObject();
        out.append(SpriteData::Bits::from_json(sprite).to_bytes());
        out.append(char(SpriteData::spritepad_attributes(sprite)));
    }

    auto anim = [&](int i) { return animations.at(i).toObject(); };
    for (int i = 0; i < animations.count(); i++)
        out.append(char(qBound(0, anim(i).value("from").toInt(), 255)));
    for (int i = 0; i < animations.count(); i++)
        out.append(char(qBound(0, anim(i).value("to").toInt(), 255)));
    for (int i = 0; i < animations.count(); i++)
        out.append(char(anim(i).value("timer").toInt()));
    for (int i = 0; i < animations.count(); i++) {
        int options = 0;
        if (anim(i).value("pingpong").toBool())
            options |= (1 << 4);
        if (anim(i).value("overlay").toBool())
            options |= (1 << 5);
        if (anim(i).value("valid").toBool())
            options |= (1 << 7);
        out.append(char(options));
    }

    QSaveFile file(path);
    if (!open_for_write(file, path, parent))
        return false;
    file.write(out);
    return commit_file(file, path, parent);
}

bool FileIO::write_prg(const QString &path, const QJsonObject &file_obj, int address, int attribute, QWidget *parent)
{
    const QJsonArray sprites = file_obj.value("sprites").toArray();
    QByteArray out;
    out.reserve(2 + sprites.count() * 64);

    if (address >= 0) {
        out.append(char(address & 0xFF));
        out.append(char((address >> 8) & 0xFF));
    }

    for (const QJsonValue &v : sprites) {
        const QJsonObject sprite = v.toObject();
        out.append(SpriteData::Bits::from_json(sprite).to_bytes());

        quint8 attr = 0;
        if (attribute == 1) {
            //attribute spritepad
            if (sprite.value("mc_mode").toBool())
                attr |= (1 << 7);
            if (sprite.value("overlay_next").toBool())
                attr |= (1 << 4);
            attr |= sprite.value("sprite_color").toInt() & 0x0F;
        } else if (attribute == 2) {
            //seuck
            if (sprite.value("mc_mode").toBool())
                attr |= (1 << 4);
            attr |= sprite.value("sprite_color").toInt() & 0x0F;
        }
        out.append(char(attr));
    }

    QSaveFile file(path);
    if (!open_for_write(file, path, parent))
        return false;
    file.write(out);
    return commit_file(file, path, parent);
}

//read prg and bin
QJsonObject FileIO::read_prg_bin(const QString &path, int *address, QWidget *parent)
{
    QByteArray content;
    if (!read_file(path, &content, parent))
        return QJsonObject();

    Reader in(content);
    if (path.endsWith(".prg", Qt::CaseInsensitive)) {
        //PRG File needs address
        int addr = in.u16();
        if (address)
            *address = addr;
    }

    QJsonArray sprites;
    while (true) {
        const char *bytes = in.read(SpriteData::BYTES);
        if (!bytes)
            break;
        // the attribute byte may be missing at the very end of the file
        int attributes = in.at_end() ? 0 : in.u8();
        sprites.append(SpriteData::sprite_from_binary(bytes, quint8(attributes)));
    }

    if (sprites.isEmpty()) {
        corrupt_file_message(path, parent);
        return QJsonObject();
    }

    QJsonObject file_obj;
    file_obj.insert("sprites", sprites);
    file_obj.insert("animations", QJsonArray()); //it's empty
    return file_obj;
}

QJsonObject FileIO::read_spm(const QString &path, QWidget *parent)
{
    QByteArray content;
    if (!read_file(path, &content, parent))
        return QJsonObject();

    QJsonParseError error;
    const QJsonObject src_obj = QJsonDocument::fromJson(content, &error).object();
    if (error.error != QJsonParseError::NoError || !src_obj.value("sprites").isArray()) {
        corrupt_file_message(path, parent);
        return QJsonObject();
    }

    QJsonObject file_obj;
    const QJsonObject colors = src_obj.value("colors").toObject();
    file_obj.insert("background", colors.value("0").toInt() & 0x0F);
    file_obj.insert("mc1", colors.value("2").toInt() & 0x0F);
    file_obj.insert("mc2", colors.value("3").toInt() & 0x0F);

    QJsonArray sprites;
    for (const QJsonValue &v : src_obj.value("sprites").toArray()) {
        const QJsonObject file_sprite = v.toObject();
        const QJsonArray pixels = file_sprite.value("pixels").toArray();
        const bool mc = file_sprite.value("multicolor").toBool();

        SpriteData::Bits bits;
        for (int y = 0; y < SpriteData::HEIGHT; y++) {
            const QJsonArray row = pixels.at(y).toArray();
            if (mc) {
                // one fat pixel = two entries; value 0 = background, 1 = sprite color, 2 = mc1, 3 = mc2
                for (int cx = 0; cx < SpriteData::MC_WIDTH; cx++) {
                    int px = row.at(2 * cx).toInt();
                    if (px == 0)
                        px = row.at(2 * cx + 1).toInt();
                    static const int to_c64[4] = {SpriteData::MC_BACKGROUND,
                                                  SpriteData::MC_SPRITE,
                                                  SpriteData::MC_MC1,
                                                  SpriteData::MC_MC2};
                    bits.set_mc(cx, y, to_c64[qBound(0, px, 3)]);
                }
            } else {
                for (int x = 0; x < SpriteData::WIDTH; x++)
                    bits.set(x, y, row.at(x).toInt() != 0);
            }
        }

        QJsonObject sprite;
        sprite.insert("sprite_data", bits.to_json());
        sprite.insert("mc_mode", mc);
        sprite.insert("exp_y", file_sprite.value("double_y").toBool());
        sprite.insert("exp_x", file_sprite.value("double_x").toBool());
        sprite.insert("overlay_next", file_sprite.value("overlay").toBool());
        sprite.insert("sprite_color", file_sprite.value("color").toInt() & 0x0F);
        sprites.append(sprite);
    }

    if (sprites.isEmpty()) {
        corrupt_file_message(path, parent);
        return QJsonObject();
    }

    file_obj.insert("sprites", sprites);
    file_obj.insert("animations", QJsonArray()); //it's empty
    return file_obj;
}
