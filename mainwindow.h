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

    void on_radio_transparent_toggled(bool checked);

    void on_radio_sprite_col_toggled(bool checked);

    void on_radioButton_mc1_toggled(bool checked);

    void on_radioButton_mc2_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QStringList col_names;
    QList<Sprite> list;
    QList<QColor> col_list;

};
#endif // MAINWINDOW_H
