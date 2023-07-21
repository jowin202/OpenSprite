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
        w.import(QString(argv[1]));
    }
    else
    {
        w.new_project();
    }
    w.show();
    return a.exec();
}
