#include "fileimport.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sprite.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    opt.show_grid_lines = true;
    opt.spriteview = this->ui->graphicsView;

    this->ui->graphicsView->set_opt(&opt);
    this->ui->label_palette->showPalette(&opt);

    connect(this->ui->graphicsView, &SpriteView::zoom_in, this, [=](){ this->ui->slider_scale->setValue(this->ui->slider_scale->value()+10);});
    connect(this->ui->graphicsView, &SpriteView::zoom_out, this, [=](){ this->ui->slider_scale->setValue(this->ui->slider_scale->value()-10);});


    leftradio.addButton(this->ui->radio_transparent_left,BUTTONS::TRANSPARENT);
    leftradio.addButton(this->ui->radio_sprite_left,BUTTONS::COLOR);
    leftradio.addButton(this->ui->radio_mc1_left,BUTTONS::MC1);
    leftradio.addButton(this->ui->radio_mc2_left,BUTTONS::MC2);
    leftradio.addButton(this->ui->radio_overlay_color_left,BUTTONS::OVERLAY_COLOR);
    leftradio.addButton(this->ui->radio_overlay_transparent_left,BUTTONS::OVERLAY_TRANSPARENT);

    rightradio.addButton(this->ui->radio_transparent_right,BUTTONS::TRANSPARENT);
    rightradio.addButton(this->ui->radio_sprite_right,BUTTONS::COLOR);
    rightradio.addButton(this->ui->radio_mc1_right,BUTTONS::MC1);
    rightradio.addButton(this->ui->radio_mc2_right,BUTTONS::MC2);
    rightradio.addButton(this->ui->radio_overlay_color_right,BUTTONS::OVERLAY_COLOR);
    rightradio.addButton(this->ui->radio_overlay_transparent_right,BUTTONS::OVERLAY_TRANSPARENT);
    this->ui->radio_transparent_right->setChecked(true);

    connect(&leftradio,&QButtonGroup::idToggled, this, [=](int id, bool checked){if (checked) this->opt.left_button = id;});
    connect(&rightradio,&QButtonGroup::idToggled, this, [=](int id, bool checked){if (checked) this->opt.right_button = id;});

    connect(this->ui->label_palette, &Palette::palette_clicked, this, [=](int button, int id){
        if ((button == Qt::LeftButton && leftradio.checkedId() == BUTTONS::TRANSPARENT)
                || (button == Qt::RightButton && rightradio.checkedId() == BUTTONS::TRANSPARENT))
            this->ui->combo_transparent->setCurrentIndex(id);

        if ((button == Qt::LeftButton && leftradio.checkedId() == BUTTONS::COLOR)
                || (button == Qt::RightButton && rightradio.checkedId() == BUTTONS::COLOR))
            this->ui->combo_sprite_col->setCurrentIndex(id);

        if ((button == Qt::LeftButton && leftradio.checkedId() == BUTTONS::MC1)
                || (button == Qt::RightButton && rightradio.checkedId() == BUTTONS::MC1))
            this->ui->combo_multicol_1->setCurrentIndex(id);

        if ((button == Qt::LeftButton && leftradio.checkedId() == BUTTONS::MC2)
                || (button == Qt::RightButton && rightradio.checkedId() == BUTTONS::MC2))
            this->ui->combo_multicol_2->setCurrentIndex(id);

        if ((button == Qt::LeftButton && leftradio.checkedId() == BUTTONS::OVERLAY_COLOR)
                || (button == Qt::RightButton && rightradio.checkedId() == BUTTONS::OVERLAY_COLOR))
            this->ui->combo_overlay_color->setCurrentIndex(id);

        if ((button == Qt::LeftButton && leftradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
                || (button == Qt::RightButton && rightradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT))
            this->ui->combo_transparent->setCurrentIndex(id);
    });

    for (int i = 0; i < 16; i++)
    {
        this->ui->combo_transparent->addItem(opt.col_names.at(i));
        this->ui->combo_transparent->setItemIcon(i,this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_sprite_col->addItem(opt.col_names.at(i));
        this->ui->combo_sprite_col->setItemIcon(i,this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_multicol_1->addItem(opt.col_names.at(i));
        this->ui->combo_multicol_1->setItemIcon(i,this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_multicol_2->addItem(opt.col_names.at(i));
        this->ui->combo_multicol_2->setItemIcon(i,this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_overlay_color->addItem(opt.col_names.at(i));
        this->ui->combo_overlay_color->setItemIcon(i,this->createIconFromColor(opt.col_list.at(i)));
    }
    connect(this->ui->combo_transparent, &QComboBox::currentIndexChanged, this, [=](int index){opt.background = index; this->ui->graphicsView->setBackgroundBrush(opt.col_list.at(index));});
    connect(this->ui->combo_sprite_col, &QComboBox::currentIndexChanged, this, [=](int index){opt.sprite_list.at(current_sprite)->sprite_color = index; this->ui->graphicsView->scene()->update();});
    connect(this->ui->combo_multicol_1, &QComboBox::currentIndexChanged, this, [=](int index){opt.mc1 = index; this->ui->graphicsView->scene()->update();});
    connect(this->ui->combo_multicol_2, &QComboBox::currentIndexChanged, this, [=](int index){opt.mc2 = index; this->ui->graphicsView->scene()->update();});
    connect(this->ui->combo_overlay_color, &QComboBox::currentIndexChanged, this, [=](int index){
        if (this->opt.sprite_list.length() > current_sprite+1)
            this->opt.sprite_list.at(current_sprite+1)->sprite_color = index;
        this->ui->graphicsView->scene()->update();
    });


    connect(this->ui->graphicsView, &SpriteView::current_sprite_changed, this, [=](int id){
        this->current_sprite = id;
        this->ui->combo_sprite_col->setCurrentIndex(opt.sprite_list.at(id)->sprite_color);
        this->ui->check_multicolor->setChecked(opt.sprite_list.at(id)->multi_color_mode);
        this->ui->check_overlay->setChecked(opt.sprite_list.at(id)->overlay_next);

        if (opt.sprite_list.at(id)->overlay_next && this->opt.sprite_list.length() > id+1)
            this->ui->combo_overlay_color->setCurrentIndex(opt.sprite_list.at(id+1)->sprite_color);


        if (leftradio.checkedId() == BUTTONS::OVERLAY_COLOR || leftradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
            this->ui->radio_sprite_left->setChecked(true);
        if (rightradio.checkedId() == BUTTONS::OVERLAY_COLOR || rightradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
            this->ui->radio_transparent_right->setChecked(true);
    });


    connect(this->ui->check_multicolor, &QCheckBox::toggled, this, [=](bool val){opt.sprite_list.at(this->current_sprite)->multi_color_mode = val; this->ui->graphicsView->scene()->update();});
    connect(this->ui->check_overlay, &QCheckBox::toggled, this, [=](bool val) {
        this->ui->radio_overlay_color_left->setEnabled(val);
        this->ui->radio_overlay_color_right->setEnabled(val);
        this->ui->radio_overlay_transparent_left->setEnabled(val);
        this->ui->radio_overlay_transparent_right->setEnabled(val);
        this->ui->combo_overlay_color->setEnabled(val);
        opt.sprite_list.at(this->current_sprite)->overlay_next = val;
        this->ui->graphicsView->scene()->update();});

    connect(this->ui->checkBox_editor_grid_lines, &QCheckBox::toggled, this, [=](bool val){ opt.show_grid_lines = val; this->ui->graphicsView->scene()->update();});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::import(QString path)
{
    QSettings settings;
    if (path != "")
    {
        FileImport(path, &opt);
        settings.setValue("last_file", path);
        this->ui->combo_multicol_1->setCurrentIndex(opt.mc1);
        this->ui->combo_multicol_2->setCurrentIndex(opt.mc2);
        this->ui->combo_transparent->setCurrentIndex(opt.background);
        this->ui->graphicsView->change_current_sprite(0);
        this->ui->graphicsView->redraw();
    }
}


void MainWindow::on_actionImport_triggered()
{
    QSettings settings;
    QString path = QFileDialog::getOpenFileName(this, "File", settings.value("last_file", QVariant()).toString());//, "Sprite Files(*.spd, *.prg)");
    this->import(path);
}


void MainWindow::on_actionCut_triggered()
{
    this->on_actionCopy_triggered();
    this->on_actionClear_triggered();
}


void MainWindow::on_actionCopy_triggered()
{
    for (int i = 0; i < 64; i++)
        this->copied_sprite_data[i] = this->opt.sprite_list.at(current_sprite)->sprite_data[i];
}


void MainWindow::on_actionPaste_triggered()
{
    for (int i = 0; i < 64; i++)
        this->opt.sprite_list.at(current_sprite)->sprite_data[i] = this->copied_sprite_data[i];
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionPaste_Into_triggered()
{
    for (int i = 0; i < 64; i++)
    {
        if (this->copied_sprite_data[i] != 0)
            this->opt.sprite_list.at(current_sprite)->sprite_data[i] = this->copied_sprite_data[i];
    }
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionClear_triggered()
{
    for (int i = 0; i < 64; i++)
        this->opt.sprite_list.at(current_sprite)->sprite_data[i] = 0;
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionSlide_Up_triggered()
{
    this->opt.sprite_list.at(current_sprite)->slide_up();
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionSlide_Down_triggered()
{
    this->opt.sprite_list.at(current_sprite)->slide_down();
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionSlide_Left_triggered()
{
    this->opt.sprite_list.at(current_sprite)->slide_left();
    if (this->opt.sprite_list.at(current_sprite)->multi_color_mode)
        this->opt.sprite_list.at(current_sprite)->slide_left();
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionSlide_Right_triggered()
{
    this->opt.sprite_list.at(current_sprite)->slide_right();
    if (this->opt.sprite_list.at(current_sprite)->multi_color_mode)
        this->opt.sprite_list.at(current_sprite)->slide_right();
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_slider_scale_valueChanged(int value)
{
    qreal scale = qPow(qreal(2), (value - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale,scale);
    this->ui->graphicsView->setTransform(matrix);
}


void MainWindow::on_actionFlip_Top_to_Bottom_triggered()
{
    this->opt.sprite_list.at(current_sprite)->flip_top();
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionFlip_Left_to_Right_triggered()
{
    this->opt.sprite_list.at(current_sprite)->flip_left();
    this->ui->graphicsView->scene()->update();
}

