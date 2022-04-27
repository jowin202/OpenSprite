#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPainter>

#include <QDebug>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void update_editor();
    void color_palette();
    QIcon createIconFromColor(QColor col);

private slots:
    void on_checkBox_editor_grid_lines_toggled(bool checked);

    void on_checkbox_multicolor_toggled(bool checked);


private:
    Ui::MainWindow *ui;
    QList<QColor> col_list;
    QStringList col_names;

};
#endif // MAINWINDOW_H
