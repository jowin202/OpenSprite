#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QButtonGroup>
#include "sprite.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    options opt;

public slots:
    QIcon createIconFromColor(QColor col)
    {    QImage img(QSize(22,14), QImage::Format_RGB32);
         QPainter painter;
         painter.begin(&img);
         painter.fillRect(0,0,22,14,col);
         painter.drawRect(0,0,22,14);
         painter.end();
         return QIcon(QPixmap::fromImage(img));}

private slots:
    void on_actionImport_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionPaste_Into_triggered();
    void on_actionClear_triggered();
    void on_actionSlide_Up_triggered();
    void on_actionSlide_Down_triggered();
    void on_actionSlide_Left_triggered();
    void on_actionSlide_Right_triggered();

private:
    Ui::MainWindow *ui;
    int current_sprite;

    QButtonGroup leftradio;
    QButtonGroup rightradio;


    unsigned char copied_sprite_data[64];
};
#endif // MAINWINDOW_H
