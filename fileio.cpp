#include "fileio.h"

FileIO::FileIO(QObject *parent)
    : QObject{parent}
{

}

QJsonObject FileIO::read_spd(QString path)
{
    QJsonObject file_obj;
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    if (!file.exists() || !file.isOpen())
    {
        QMessageBox::critical(0, QString("File does not exist"), QString("File does not exist"));
        file.close();
        return file_obj;
    }



    QByteArray header = file.read(3);
    if (header.toHex() != "535044")
    {
        QMessageBox::critical(0, QString("Invalid SpritePad Format"), QString("Invalid SpritePad Format"));
        file.close();
        return file_obj;
    }


    int vers = file.read(1).at(0);
    file_obj.insert("version", vers);


    int sprite_num = file.read(1).at(0) + 1;
    int animations_num = file.read(1).at(0) + 1;

    file_obj.insert("background", file.read(1).at(0));
    file_obj.insert("mc1", file.read(1).at(0));
    file_obj.insert("mc2", file.read(1).at(0));

    QJsonArray sprites;
    QJsonArray animations;
    for (int i = 0; i < sprite_num; i++)
    {
        QJsonObject sprite;
        char sprite_content[63];
        file.read(sprite_content, 63);

        QJsonArray array_rows;
        for (int y = 0; y < 21; y++)
        {
            QJsonArray array_row;
            for (int x = 0; x < 24; x++)
            {
                array_row.append((sprite_content[3*y + x/8] &(0x01 << (7-(x%8)))) >> (7-(x%8))   );
            }
            array_rows.append(array_row);
        }

        sprite.insert("sprite_data", array_rows);
        int options = (0xFF & file.read(1).at(0));
        sprite.insert("mc_mode", (0x1 & (options >> 7)) != 0);
        sprite.insert("exp_y", (0x1 & (options >> 5)) != 0);
        sprite.insert("exp_x", (0x1 & (options >> 6)) != 0);
        sprite.insert("overlay_next", (0x1 & (options >> 4)) != 0);
        sprite.insert("sprite_color", options & 0xF);
        sprites.append(sprite);
    }
    file_obj.insert("sprites", sprites);


    QList<int> start;
    QList<int> end;
    QList<int> timer;
    QList<bool> pingpong;
    QList<bool> overlay;
    QList<bool> valid;

    for (int i = 0; i < animations_num; i++)
        start.append(file.read(1).at(0));
    for (int i = 0; i < animations_num; i++)
        end.append(file.read(1).at(0));
    for (int i = 0; i < animations_num; i++)
        timer.append(file.read(1).at(0));

    for (int i = 0; i < animations_num; i++)
    {
        int options = file.read(1).at(0);

        pingpong.append( (0x1 & (options >> 4)) != 0);
        overlay.append( (0x1 & (options >> 5)) != 0);
        valid.append( (0x1 & (options >> 7)) != 0);
    }

    for (int i = 0; i < animations_num; i++)
    {
        QJsonObject animation;
        animation.insert("from", start.at(i));
        animation.insert("to", end.at(i));
        animation.insert("timer", timer.at(i));
        animation.insert("pingpong", pingpong.at(i));
        animation.insert("overlay", overlay.at(i));
        animation.insert("valid", valid.at(i));

        animations.append(animation);
    }

    file_obj.insert("animations", animations);

    file.close();
    return file_obj;
}

void FileIO::write_spd(QString path, QJsonObject file_obj)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);

    file.write(QByteArray::fromHex("535044"));

    file.write(QByteArray(1,file_obj.value("version").toInt()));
    file.write(QByteArray(1,file_obj.value("sprites").toArray().count()-1));
    file.write(QByteArray(1,file_obj.value("animations").toArray().count()-1));


    file.write(QByteArray(1,file_obj.value("background").toInt()));
    file.write(QByteArray(1,file_obj.value("mc1").toInt()));
    file.write(QByteArray(1,file_obj.value("mc2").toInt()));


    for (int i = 0; i < file_obj.value("sprites").toArray().count(); i++)
    {
        char sprite_data[64];
        for (int i = 0; i < 64; i++) sprite_data[i] = 0;

        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (file_obj.value("sprites").toArray().at(i).toObject().value("sprite_data").toArray().at(y).toArray().at(x).toInt() == 1)
                    sprite_data[3*y + x/8] |= (0x01 << (7-(x%8)));
                else
                    sprite_data[3*y + x/8] &= ~(0x01 << (7-(x%8)));
            }
        }
        sprite_data[63] = 0;
        if (file_obj.value("sprites").toArray().at(i).toObject().value("mc_mode").toBool())
            sprite_data[63] |=  ((1 << 7));
        if (file_obj.value("sprites").toArray().at(i).toObject().value("exp_x").toBool())
            sprite_data[63] |=  ((1 << 6));
        if (file_obj.value("sprites").toArray().at(i).toObject().value("exp_y").toBool())
            sprite_data[63] |=  ((1 << 5));
        if (file_obj.value("sprites").toArray().at(i).toObject().value("overlay_next").toBool())
            sprite_data[63] |=  ((1 << 4));
        sprite_data[63] |= file_obj.value("sprites").toArray().at(i).toObject().value("sprite_color").toInt();
        file.write(sprite_data,64);
    }

    for (int i = 0; i < file_obj.value("animations").toArray().count(); i++)
        file.write(QByteArray(1,file_obj.value("animations").toArray().at(i).toObject().value("from").toInt()));
    for (int i = 0; i < file_obj.value("animations").toArray().count(); i++)
        file.write(QByteArray(1,file_obj.value("animations").toArray().at(i).toObject().value("to").toInt()));
    for (int i = 0; i < file_obj.value("animations").toArray().count(); i++)
        file.write(QByteArray(1,file_obj.value("animations").toArray().at(i).toObject().value("timer").toInt()));

    for (int i = 0; i < file_obj.value("animations").toArray().count(); i++)
    {
        int options = 0;

        if (file_obj.value("animations").toArray().at(i).toObject().value("pingpong").toBool())
            options |= (1 << 4);
        if (file_obj.value("animations").toArray().at(i).toObject().value("overlay").toBool())
            options |= (1 << 5);
        if (file_obj.value("animations").toArray().at(i).toObject().value("valid").toBool())
            options |= (1 << 7);

        file.write(QByteArray(1,options));
    }



    file.close();
}

void FileIO::write_prg(QString path, QJsonObject file_obj, int address, int attribute)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);

    if (address >= 0)
    {
        file.write( QByteArray(1, address & 0xFF)  );
        file.write( QByteArray(1, address >> 8)  );
    }

    for (int i = 0; i < file_obj.value("sprites").toArray().count(); i++)
    {
        char sprite_data[64];
        for (int i = 0; i < 64; i++) sprite_data[i] = 0;

        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (file_obj.value("sprites").toArray().at(i).toObject().value("sprite_data").toArray().at(y).toArray().at(x).toInt() == 1)
                    sprite_data[3*y + x/8] |= (0x01 << (7-(x%8)));
                else
                    sprite_data[3*y + x/8] &= ~(0x01 << (7-(x%8)));
            }
        }
        sprite_data[63] = 0;
        if (attribute == 1)
        {
            //attribute spritepad
            if (file_obj.value("sprites").toArray().at(i).toObject().value("mc_mode").toBool())
                sprite_data[63] |=  ((1 << 7));
            if (file_obj.value("sprites").toArray().at(i).toObject().value("overlay_next").toBool())
                sprite_data[63] |=  ((1 << 4));
            sprite_data[63] |= file_obj.value("sprites").toArray().at(i).toObject().value("sprite_color").toInt();
        }
        else if (attribute == 2)
        {
            //seuck
            if (file_obj.value("sprites").toArray().at(i).toObject().value("mc_mode").toBool())
                sprite_data[63] |=  ((1 << 4));
            sprite_data[63] |= file_obj.value("sprites").toArray().at(i).toObject().value("sprite_color").toInt();
        }
        file.write(sprite_data,64);
    }

    file.close();
}

//read prg and bin
QJsonObject FileIO::read_prg_bin(QString path, int *address)
{
    QJsonObject file_obj;
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    if (!file.exists() || !file.isOpen())
    {
        QMessageBox::critical(0, QString("File does not exist"), QString("File does not exist"));
        file.close();
        return file_obj;
    }

    if (path.endsWith(".prg", Qt::CaseInsensitive))
    {
        //PRG File needs address
        int addr1 = file.read(1).at(0);
        int addr2 = file.read(1).at(0);
        *address = (addr1 & 0xFF) | ((addr2 & 0xFF) << 8);
    }

    QJsonArray sprites;
    QJsonArray animations;
    while(true)
    {
        QJsonObject sprite;
        char sprite_content[63];
        if (file.read(sprite_content, 63) != 63)
            break;

        QJsonArray array_rows;
        for (int y = 0; y < 21; y++)
        {
            QJsonArray array_row;
            for (int x = 0; x < 24; x++)
            {
                array_row.append((sprite_content[3*y + x/8] &(0x01 << (7-(x%8)))) >> (7-(x%8))   );
            }
            array_rows.append(array_row);
        }

        sprite.insert("sprite_data", array_rows);
        int options = (0xFF & file.read(1).at(0));
        sprite.insert("mc_mode", (0x1 & (options >> 7)) != 0);
        sprite.insert("exp_y", (0x1 & (options >> 5)) != 0);
        sprite.insert("exp_x", (0x1 & (options >> 6)) != 0);
        sprite.insert("overlay_next", (0x1 & (options >> 4)) != 0);
        sprite.insert("sprite_color", options & 0xF);
        sprites.append(sprite);
    }
    file_obj.insert("sprites", sprites);


    file_obj.insert("animations", animations); //it's empty

    file.close();
    return file_obj;
}



