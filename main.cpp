#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("jowin202");
    QCoreApplication::setOrganizationDomain("github.com/jowin202");
    QCoreApplication::setApplicationName("OpenSprite");


    MainWindow w;
    if (argc > 1)
    {
        w.open_file(argv[1]);
    }
    w.show();
    return a.exec();
}
