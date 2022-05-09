#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Sprite *sprite = new Sprite;
    this->ui->label_editor->set_sprite(sprite);


    col_list << QColor(0,0,0);
    col_list << QColor(255,255,255);
    col_list << QColor(0x92,0x4a,0x40);
    col_list << QColor(0x84,0xc5,0xcc);
    col_list << QColor(0x93,0x51,0xb6);
    col_list << QColor(0x72,0xb1,0x4b);
    col_list << QColor(0x48,0x3a,0xaa);
    col_list << QColor(0xd5,0xdf,0x7c);
    col_list << QColor(0x99,0x69,0x2d);
    col_list << QColor(0x67,0x52,0x00);
    col_list << QColor(0xc1,0x81,0x7a);
    col_list << QColor(0x60,0x60,0x60);
    col_list << QColor(0x8a,0x8a,0x8a);
    col_list << QColor(0xb3,0xec,0x91);
    col_list << QColor(0x86,0x7a,0xde);
    col_list << QColor(0xb3,0xb3,0xb3);

    col_names << "Black";
    col_names << "White";
    col_names << "Red";
    col_names << "Cyan";
    col_names << "Purple";
    col_names << "Green";
    col_names << "Blue";
    col_names << "Yellow";
    col_names << "Orange";
    col_names << "Brown";
    col_names << "Pink";
    col_names << "Dark Grey";
    col_names << "Grey";
    col_names << "Light Green";
    col_names << "Light Blue";
    col_names << "Light Grey";

    for (int i = 0; i < 16; i++)
    {
        this->ui->combo_transparent->addItem(col_names.at(i));
        this->ui->combo_transparent->setItemIcon(i,this->createIconFromColor(col_list.at(i)));
        this->ui->combo_sprite_col->addItem(col_names.at(i));
        this->ui->combo_sprite_col->setItemIcon(i,this->createIconFromColor(col_list.at(i)));
        this->ui->combo_multicol_1->addItem(col_names.at(i));
        this->ui->combo_multicol_1->setItemIcon(i,this->createIconFromColor(col_list.at(i)));
        this->ui->combo_multicol_2->addItem(col_names.at(i));
        this->ui->combo_multicol_2->setItemIcon(i,this->createIconFromColor(col_list.at(i)));
    }

    connect(this->ui->combo_transparent, SIGNAL(currentIndexChanged(int)), this->ui->label_editor, SLOT(set_transparent(int)));
    connect(this->ui->combo_sprite_col, SIGNAL(currentIndexChanged(int)), this->ui->label_editor, SLOT(set_sprite_color(int))); this->ui->combo_transparent->setCurrentIndex(6);
    this->ui->combo_sprite_col->setCurrentIndex(5);

    connect(this->ui->label_editor, SIGNAL(mouse_updated_cell_updated(int,int)), this, SLOT(show_current_cell(int,int)));
    this->update_editor();
    this->color_palette();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    this->ui->label_editor->keyPressEvent(ev);
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    this->ui->label_editor->keyReleaseEvent(ev);
}


void MainWindow::update_editor()
{

}



void MainWindow::color_palette()
{




    QImage img(176,28,QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    for (int i = 0; i < 8; i++)
    {
        painter.fillRect(22*i,0,22,14, col_list.at(i));
        painter.fillRect(22*i,14,22,14, col_list.at(8+i));
    }
    painter.end();

    this->ui->label_palette->setPixmap(QPixmap::fromImage(img));
}

QIcon MainWindow::createIconFromColor(QColor col)
{
    QImage img(QSize(22,14), QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&img);
    painter.fillRect(0,0,22,14,col);
    painter.drawRect(0,0,22,14);
    painter.end();
    return QIcon(QPixmap::fromImage(img));
}

void MainWindow::show_current_cell(int x, int y)
{
    this->ui->statusbar->showMessage(QString("Current cell: (%1,%2)").arg(x).arg(y));
}


void MainWindow::on_checkBox_editor_grid_lines_toggled(bool checked)
{
    this->ui->label_editor->setgridlines(checked);
}

void MainWindow::on_checkbox_multicolor_toggled(bool checked)
{
    this->ui->label_editor->set_multicol(checked);
    this->ui->radioButton_mc1->setEnabled(checked);
    this->ui->radioButton_mc2->setEnabled(checked);
    this->ui->combo_multicol_1->setEnabled(checked);
    this->ui->combo_multicol_2->setEnabled(checked);
    if (!checked)
    {
        if (this->ui->radioButton_mc1->isChecked() || this->ui->radioButton_mc2->isChecked())
            this->ui->radio_transparent->setChecked(true);
    }
    this->update_editor();
}

void MainWindow::on_checkBox_expand_x_toggled(bool checked)
{
    this->ui->label_editor->set_expand_x(checked);
}

void MainWindow::on_checkBox_expand_y_toggled(bool checked)
{
    this->ui->label_editor->set_expand_y(checked);
}

void MainWindow::on_radio_transparent_toggled(bool checked)
{
    qDebug() << "transparent: " << checked;
}

void MainWindow::on_radio_sprite_col_toggled(bool checked)
{
    qDebug() << "col: " << checked;
}

void MainWindow::on_radioButton_mc1_toggled(bool checked)
{
    qDebug() << "mc1: " << checked;
}

void MainWindow::on_radioButton_mc2_toggled(bool checked)
{
    qDebug() << "mc2: " << checked;
}
