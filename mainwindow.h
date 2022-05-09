#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPainter>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

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
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
    void wheelEvent(QWheelEvent *ev);



public slots:
    void update_editor();
    void color_palette();
    QIcon createIconFromColor(QColor col);
    void show_current_cell(int x, int y);

private slots:
    void on_checkBox_editor_grid_lines_toggled(bool checked);

    void on_checkbox_multicolor_toggled(bool checked);


    void on_checkBox_expand_x_toggled(bool checked);

    void on_checkBox_expand_y_toggled(bool checked);


    void on_combo_zoom_currentIndexChanged(int index);

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_radio_transparent_left_toggled(bool checked);
    void on_radio_sprite_left_toggled(bool checked);
    void on_radio_mc1_left_toggled(bool checked);
    void on_radio_mc2_left_toggled(bool checked);

    void on_radio_transparent_right_toggled(bool checked);
    void on_radio_sprite_right_toggled(bool checked);
    void on_radio_mc1_right_toggled(bool checked);
    void on_radio_mc2_right_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QStringList col_names;
    QList<Sprite> list;
    QList<QColor> col_list;
    bool control_pressed = false;

};
#endif // MAINWINDOW_H
