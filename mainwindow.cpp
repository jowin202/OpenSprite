#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    leftradio.addButton(this->ui->radio_transparent_left,0);
    leftradio.addButton(this->ui->radio_sprite_left,1);
    leftradio.addButton(this->ui->radio_mc1_left,2);
    leftradio.addButton(this->ui->radio_mc2_left,3);
    leftradio.addButton(this->ui->radio_overlay_left,4);

    rightradio.addButton(this->ui->radio_transparent_right,0);
    rightradio.addButton(this->ui->radio_sprite_right,1);
    rightradio.addButton(this->ui->radio_mc1_right,2);
    rightradio.addButton(this->ui->radio_mc2_right,3);
    rightradio.addButton(this->ui->radio_overlay_right,4);
    this->ui->radio_transparent_right->setChecked(true);

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
        this->ui->combo_overlay->addItem(col_names.at(i));
        this->ui->combo_overlay->setItemIcon(i,this->createIconFromColor(col_list.at(i)));
    }

    this->ui->combo_transparent->setCurrentIndex(6);
    this->ui->combo_sprite_col->setCurrentIndex(5);
    this->ui->combo_multicol_1->setCurrentIndex(2);
    this->ui->combo_multicol_2->setCurrentIndex(3);
    this->ui->combo_overlay->setCurrentIndex(7);



    this->ui->label_viewer->add_new_sprites(12);
    this->ui->label_editor->set_sprite(this->ui->label_viewer->sprite_at(0));
    this->ui->label_editor->updateView();
    this->ui->label_viewer->update_view();



    connect(this->ui->combo_transparent, SIGNAL(currentIndexChanged(int)), this->ui->label_editor, SLOT(set_transparent(int)));
    connect(this->ui->combo_sprite_col, SIGNAL(currentIndexChanged(int)), this->ui->label_editor, SLOT(set_sprite_color(int)));
    connect(this->ui->combo_multicol_1, SIGNAL(currentIndexChanged(int)), this->ui->label_editor, SLOT(set_mc1(int)));
    connect(this->ui->combo_multicol_2, SIGNAL(currentIndexChanged(int)), this->ui->label_editor, SLOT(set_mc2(int)));
    connect(this->ui->combo_multicol_1, SIGNAL(currentIndexChanged(int)), this->ui->label_viewer, SLOT(set_mc1(int)));
    connect(this->ui->combo_multicol_2, SIGNAL(currentIndexChanged(int)), this->ui->label_viewer, SLOT(set_mc2(int)));

    connect(this->ui->label_editor, SIGNAL(mouse_updated_cell_updated(int,int)), this, SLOT(show_current_cell(int,int)));
    connect(this->ui->label_editor, SIGNAL(update_viewer()), this->ui->label_viewer, SLOT(update_current_sprite()));


    connect(this->ui->label_viewer, SIGNAL(sprite_selected(Sprite*)), this->ui->label_editor, SLOT(set_sprite(Sprite*)));
    connect(this->ui->label_viewer, SIGNAL(sprite_update_detail_gui(int,bool,bool,bool,bool)), this, SLOT(update_gui(int,bool,bool,bool,bool)));
    connect(this->ui->label_palette, SIGNAL(palette_clicked(int,int)), this, SLOT(color_change_from_palette(int,int)));
    this->ui->label_palette->showPalette();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Control)
        this->control_pressed = true;
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Control)
        this->control_pressed = false;
}

void MainWindow::wheelEvent(QWheelEvent *ev)
{
    if (control_pressed)
    {
        if (ev->delta() > 10)
            this->on_actionZoom_In_triggered();
        else if (ev->delta() < -10)
            this->on_actionZoom_Out_triggered();
    }
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

void MainWindow::color_change_from_palette(int mouse, int color_num)
{
    if (mouse == 0) //Left
    {
        if (this->leftradio.checkedId() == 0) // transparent
        {
            this->ui->combo_transparent->setCurrentIndex(color_num);
        }
        else if (this->leftradio.checkedId() == 1) // sprite color
        {
            this->ui->combo_sprite_col->setCurrentIndex(color_num);
        }
        else if (this->leftradio.checkedId() == 2) // multi color 1
        {
            this->ui->combo_multicol_1->setCurrentIndex(color_num);
        }
        else if (this->leftradio.checkedId() == 3) // multi color 2
        {
            this->ui->combo_multicol_2->setCurrentIndex(color_num);
        }
        else if (this->leftradio.checkedId() == 4) // overlay color
        {
            this->ui->combo_overlay->setCurrentIndex(color_num);
        }
    }
    else //Right
    {
        if (this->rightradio.checkedId() == 0) // transparent
        {
            this->ui->combo_transparent->setCurrentIndex(color_num);
        }
        else if (this->rightradio.checkedId() == 1) // sprite color
        {
            this->ui->combo_sprite_col->setCurrentIndex(color_num);
        }
        else if (this->rightradio.checkedId() == 2) // multi color 1
        {
            this->ui->combo_multicol_1->setCurrentIndex(color_num);
        }
        else if (this->rightradio.checkedId() == 3) // multi color 2
        {
            this->ui->combo_multicol_2->setCurrentIndex(color_num);
        }
        else if (this->rightradio.checkedId() == 4) // overlay color
        {
            this->ui->combo_overlay->setCurrentIndex(color_num);
        }
    }
}

void MainWindow::update_gui(int color, bool expand_x, bool expand_y, bool overlay, bool multicolor)
{
    this->ui->combo_sprite_col->setCurrentIndex(color);
    this->ui->checkBox_expand_x->setChecked(expand_x);
    this->ui->checkBox_expand_y->setChecked(expand_y);
    this->ui->check_overlay->setChecked(overlay);
    this->ui->checkbox_multicolor->setChecked(multicolor);
}


void MainWindow::on_checkBox_editor_grid_lines_toggled(bool checked)
{
    this->ui->label_editor->setgridlines(checked);
}

void MainWindow::on_checkbox_multicolor_toggled(bool checked)
{
    this->ui->label_editor->set_multicol(checked);
    this->ui->radio_mc1_left->setEnabled(checked);
    this->ui->radio_mc2_left->setEnabled(checked);
    this->ui->radio_mc1_right->setEnabled(checked);
    this->ui->radio_mc2_right->setEnabled(checked);
    this->ui->combo_multicol_1->setEnabled(checked);
    this->ui->combo_multicol_2->setEnabled(checked);
    if (!checked)
    {
        if (this->ui->radio_mc1_left->isChecked() || this->ui->radio_mc2_left->isChecked())
            this->ui->radio_sprite_left->setChecked(true);
        if (this->ui->radio_mc1_right->isChecked() || this->ui->radio_mc2_right->isChecked())
            this->ui->radio_transparent_right->setChecked(true);
    }
    this->ui->label_viewer->update_view();
}

void MainWindow::on_checkBox_expand_x_toggled(bool checked)
{
    this->ui->label_editor->set_expand_x(checked);
}

void MainWindow::on_checkBox_expand_y_toggled(bool checked)
{
    this->ui->label_editor->set_expand_y(checked);
}


void MainWindow::on_combo_zoom_currentIndexChanged(int index)
{
    this->ui->label_editor->set_multiplicator(index+1);
}

void MainWindow::on_actionZoom_In_triggered()
{
    if (this->ui->combo_zoom->currentIndex() == 4)
        return;
    this->ui->combo_zoom->setCurrentIndex(this->ui->combo_zoom->currentIndex()+1);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    if (this->ui->combo_zoom->currentIndex() == 0)
        return;
    this->ui->combo_zoom->setCurrentIndex(this->ui->combo_zoom->currentIndex()-1);
}

void MainWindow::on_radio_transparent_left_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_left_button(Editor::TRANSPARENT);
}

void MainWindow::on_radio_sprite_left_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_left_button(Editor::COLOR);
}

void MainWindow::on_radio_mc1_left_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_left_button(Editor::MC1);
}

void MainWindow::on_radio_mc2_left_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_left_button(Editor::MC2);
}

void MainWindow::on_radio_transparent_right_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_right_button(Editor::TRANSPARENT);
}

void MainWindow::on_radio_sprite_right_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_right_button(Editor::COLOR);
}

void MainWindow::on_radio_mc1_right_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_right_button(Editor::MC1);
}

void MainWindow::on_radio_mc2_right_toggled(bool checked)
{
    if (checked)
        this->ui->label_editor->set_right_button(Editor::MC2);
}

void MainWindow::on_check_lock_multicolors_toggled(bool checked)
{
    this->ui->combo_multicol_1->setEnabled(!checked);
    this->ui->combo_multicol_2->setEnabled(!checked);
}




void MainWindow::on_checkBox_browser_grid_lines_toggled(bool checked)
{
    this->ui->label_viewer->set_gridlines(checked);
}
