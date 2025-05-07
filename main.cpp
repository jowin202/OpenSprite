#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("jowin202");
    QCoreApplication::setOrganizationDomain("github.com/jowin202");
    QCoreApplication::setApplicationName("OpenSprite");


    //set default settings if not set
    QSettings settings;
    if (!settings.contains("bgcolor"))
        settings.setValue("bgcolor", 0xd9d6c8);

    if (!settings.contains("selection_color"))
        settings.setValue("selection_color", 0x00ff00);

    if (!settings.contains("sprite_spacing_x"))
        settings.setValue("sprite_spacing_x", 30);

    if (!settings.contains("sprite_spacing_y"))
        settings.setValue("sprite_spacing_y", 30);

    if (!settings.contains("sprites_per_row"))
        settings.setValue("sprites_per_row", 4);




    MainWindow w;
    if (argc > 1)
    {
        w.import(QString(argv[1]));
    }
    else
    {
        w.new_project();
    }
    w.show();
    return a.exec();
}
