#include "fileimport.h"
#include "sprite.h"

FileImport::FileImport(QString path, options *opt)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    QByteArray header = file.read(3);
    if (header.toHex() != "535044")
    {
        QMessageBox::critical(0, QString("Invalid SpritePad Format"), QString("Invalid SpritePad Format"));
        file.close();
        return;
    }

    int vers = file.read(1).at(0);
    Q_UNUSED(vers);

    opt->sprite_num = file.read(1).at(0) + 1;
    //this->ui->label_viewer->set_sprite_count(sprites_num);

    opt->animations_num = file.read(1).at(0) + 1;

    opt->background = file.read(1).at(0);
    opt->mc1 = file.read(1).at(0);
    opt->mc2 = file.read(1).at(0);

    //this->ui->combo_transparent->setCurrentIndex(background);
    //this->ui->combo_multicol_1->setCurrentIndex(mc1);
    //this->ui->combo_multicol_2->setCurrentIndex(mc2);



    opt->sprite_list.clear();
    for (int i = 0; i < opt->sprite_num; i++) //should be the same length as sprites in file
    {
        opt->sprite_list.append(new Sprite(opt));
        file.read((char*)(&opt->sprite_list.at(i)->sprite_data), 64);
    }

    file.close();
}
