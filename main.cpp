#include "mainwindow.h"
#include "settingsdialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // always enabled in Qt 6
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("jowin202");
    QCoreApplication::setOrganizationDomain("github.com/jowin202");
    QCoreApplication::setApplicationName("OpenSprite");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    //set default settings if not set
    QSettings settings;
    SettingsDialog::apply_defaults(settings, false);

    MainWindow w;
    const QStringList args = QCoreApplication::arguments();
    if (args.size() < 2 || !w.import(args.at(1)))
        w.new_project();
    w.show();
    return a.exec();
}
