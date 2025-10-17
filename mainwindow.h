#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QColorDialog>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMainWindow>
#include <QMimeData>
#include <QSettings>

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QPushButton>

#include "sprite.h"

class AnimationDialog;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    options opt;

    void closeEvent(QCloseEvent *ev);

public slots:
    QIcon createIconFromColor(QColor col)
    {
        QImage img(QSize(22, 14), QImage::Format_RGB32);
        QPainter painter;
        painter.begin(&img);
        painter.fillRect(0, 0, 22, 14, col);
        painter.drawRect(0, 0, 22, 14);
        painter.end();
        return QIcon(QPixmap::fromImage(img));
    }

    void dragEnterEvent(QDragEnterEvent *event) { event->acceptProposedAction(); }
    void dropEvent(QDropEvent *event)
    {
        const QMimeData *mimeData = event->mimeData();
        if (mimeData->hasUrls() && mimeData->urls().length() == 1)
            this->import(mimeData->urls().at(0).toLocalFile());
    }

public slots:
    void import(QString path);
    void new_project();

private slots:
    void on_actionOpenProject_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionPaste_Into_triggered();
    void on_actionClear_triggered();
    void on_actionSlide_Up_triggered();
    void on_actionSlide_Down_triggered();
    void on_actionSlide_Left_triggered();
    void on_actionSlide_Right_triggered();
    void on_slider_scale_valueChanged(int value);
    void on_actionFlip_Top_to_Bottom_triggered();
    void on_actionFlip_Left_to_Right_triggered();
    void on_actionSave_Project_triggered();
    void on_actionSave_Project_As_triggered();
    void on_actionDelete_Sprite_triggered();
    void on_actionAdd_Sprite_triggered();
    void on_actionAbout_triggered();
    void on_actionExport_as_triggered();
    void on_actionExport_triggered();

    void on_actionReflect_Left_To_Right_triggered();

    void on_actionReflect_Top_to_Bottom_triggered();

    void on_actionNew_triggered();

    void on_actionUndo_triggered();

    void on_actionAnimations_Editor_triggered();

    void on_actionSprite_Color_MC_1_triggered();

    void on_actionSprite_Color_MC_2_triggered();

    void on_actionMC_1_MC_2_triggered();

    void on_actionRotate_triggered();

    void on_actionPreferences_triggered();

    void on_actionSelection_to_Animation_triggered();

    void on_actionSelect_All_triggered();

    void on_actionFlood_Fill_triggered();

    void on_actionZoomIn_triggered();

    void on_actionZoomOut_triggered();

    void on_actionScale_Dialog_triggered();

private:
    Ui::MainWindow *ui;
    AnimationDialog *animation_dialog = 0;

    QButtonGroup leftradio;
    QButtonGroup rightradio;

    QAction num0;
    QAction num1;
    QAction num2;
    QAction num3;
    QAction num4;
    QAction num5;

    //QJsonObject copied_sprite;
};
#endif // MAINWINDOW_H
