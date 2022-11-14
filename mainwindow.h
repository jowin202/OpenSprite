#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QMimeData>

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

    void dragEnterEvent(QDragEnterEvent *e)
    {
        if (e->mimeData()->hasUrls())
            e->acceptProposedAction();
    }
    void dropEvent(QDropEvent *event)
    {
        if (event->mimeData()->hasUrls() && event->mimeData()->urls().length() == 1)
            this->open_file(event->mimeData()->urls().first().toLocalFile());
    }


    void open_file(QString file);

    void import_spritepad_file(QString file_path);
    void import_prg_file(QString file_path);


public slots:
    QIcon createIconFromColor(QColor col);
    void show_current_cell(int x, int y);
    void color_change_from_palette(int mouse, int color_num);
    void update_gui(int color, bool expand_x, bool expand_y, bool overlay, bool multicolor);



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

    void on_check_lock_multicolors_toggled(bool checked);


    void on_checkBox_browser_grid_lines_toggled(bool checked);
    void on_actionOpen_triggered();


private:
    Ui::MainWindow *ui;
    QStringList col_names;
    QList<QColor> col_list;
    bool control_pressed = false;
    QButtonGroup leftradio;
    QButtonGroup rightradio;

    int prg_address = 0;

};
#endif // MAINWINDOW_H
