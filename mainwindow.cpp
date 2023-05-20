#include "fileio.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sprite.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    opt.spriteview = this->ui->graphicsView;

    this->ui->graphicsView->set_opt(&opt);
    this->ui->label_palette->showPalette(&opt);

    connect(this->ui->graphicsView, &SpriteView::zoom_in, this, [=](){ this->ui->slider_scale->setValue(this->ui->slider_scale->value()+10);});
    connect(this->ui->graphicsView, &SpriteView::zoom_out, this, [=](){ this->ui->slider_scale->setValue(this->ui->slider_scale->value()-10);});


    connect(this->ui->graphicsView, &SpriteView::current_sprite_changed, this, [=](int id){
        this->current_sprite = id;
        this->ui->combo_sprite_col->setCurrentIndex(opt.data.value("sprites").toArray().at(id).toObject().value("sprite_color").toInt());
        this->ui->check_multicolor->setChecked(opt.data.value("sprites").toArray().at(id).toObject().value("mc_mode").toBool());
        this->ui->check_overlay->setChecked(opt.data.value("sprites").toArray().at(id).toObject().value("overlay_next").toBool());

        if (opt.data.value("sprites").toArray().at(id).toObject().value("overlay_next").toBool() &&
                this->opt.data.value("sprites").toArray().count() > id+1)
            this->ui->combo_overlay_color->setCurrentIndex(opt.data.value("sprites").toArray().at(id+1).toObject().value("sprite_color").toInt());

        if (leftradio.checkedId() == BUTTONS::OVERLAY_COLOR || leftradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
            this->ui->radio_sprite_left->setChecked(true);
        if (rightradio.checkedId() == BUTTONS::OVERLAY_COLOR || rightradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
            this->ui->radio_transparent_right->setChecked(true);
    });



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

    //palettes
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



    connect(this->ui->graphicsView, &SpriteView::droppedFile, [=](QString file) { this->import(file); });

    //combos
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
    connect(this->ui->combo_transparent, &QComboBox::currentIndexChanged, this, [=](int index){
        opt.data.insert("background", index);
        this->ui->graphicsView->redraw();
    });
    connect(this->ui->combo_sprite_col, &QComboBox::currentIndexChanged, this, [=](int index){
        QJsonObject current_sprite_obj = opt.data.value("sprites").toArray().at(current_sprite).toObject();
        current_sprite_obj.insert("sprite_color", index);
        QJsonArray sprites_array = opt.data.value("sprites").toArray();
        sprites_array.removeAt(current_sprite);
        sprites_array.insert(current_sprite, current_sprite_obj);
        opt.data.insert("sprites", sprites_array);

        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->combo_multicol_1, &QComboBox::currentIndexChanged, this, [=](int index){
        opt.data.insert("mc1", index);
        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->combo_multicol_2, &QComboBox::currentIndexChanged, this, [=](int index){
        opt.data.insert("mc2", index);
        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->combo_overlay_color, &QComboBox::currentIndexChanged, this, [=](int index){
        if (this->opt.data.value("sprites").toArray().count() > current_sprite+1)
        {
            QJsonObject current_sprite_obj = opt.data.value("sprites").toArray().at(current_sprite+1).toObject();
            current_sprite_obj.insert("sprite_color", index);
            QJsonArray sprites_array = opt.data.value("sprites").toArray();
            sprites_array.removeAt(current_sprite+1);
            sprites_array.insert(current_sprite+1, current_sprite_obj);
            opt.data.insert("sprites", sprites_array);
        }
        this->ui->graphicsView->scene()->update();
    });


    connect(this->ui->check_multicolor, &QCheckBox::toggled, this, [=](bool val){
        QJsonObject current_sprite_obj = opt.data.value("sprites").toArray().at(current_sprite).toObject();
        current_sprite_obj.insert("mc_mode", val);
        QJsonArray sprites_array = opt.data.value("sprites").toArray();
        sprites_array.removeAt(current_sprite);
        sprites_array.insert(current_sprite, current_sprite_obj);
        opt.data.insert("sprites", sprites_array);

        this->ui->radio_mc1_left->setEnabled(val);
        this->ui->radio_mc1_right->setEnabled(val);
        this->ui->radio_mc2_left->setEnabled(val);
        this->ui->radio_mc2_right->setEnabled(val);
        this->ui->combo_multicol_1->setEnabled(val);
        this->ui->combo_multicol_2->setEnabled(val);

        if (!val && this->ui->radio_mc1_left->isChecked())
            this->ui->radio_sprite_left->setChecked(true);
        if (!val && this->ui->radio_mc1_right->isChecked())
            this->ui->radio_transparent_right->setChecked(true);

        if (!val && this->ui->radio_mc2_left->isChecked())
            this->ui->radio_sprite_left->setChecked(true);
        if (!val && this->ui->radio_mc2_right->isChecked())
            this->ui->radio_transparent_right->setChecked(true);

        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->check_overlay, &QCheckBox::toggled, this, [=](bool val) {
        this->ui->radio_overlay_color_left->setEnabled(val);
        this->ui->radio_overlay_color_right->setEnabled(val);
        this->ui->radio_overlay_transparent_left->setEnabled(val);
        this->ui->radio_overlay_transparent_right->setEnabled(val);
        this->ui->combo_overlay_color->setEnabled(val);



        if (!val && this->ui->radio_overlay_color_left->isChecked())
            this->ui->radio_sprite_left->setChecked(true);
        if (!val && this->ui->radio_overlay_color_right->isChecked())
            this->ui->radio_sprite_right->setChecked(true);

        if (!val && this->ui->radio_overlay_transparent_left->isChecked())
            this->ui->radio_transparent_left->setChecked(true);
        if (!val && this->ui->radio_overlay_transparent_right->isChecked())
            this->ui->radio_transparent_right->setChecked(true);



        QJsonObject current_sprite_obj = opt.data.value("sprites").toArray().at(current_sprite).toObject();
        current_sprite_obj.insert("overlay_next", val);
        QJsonArray sprites_array = opt.data.value("sprites").toArray();
        sprites_array.removeAt(current_sprite);
        sprites_array.insert(current_sprite, current_sprite_obj);
        //next sprite should be single color
        if (val && this->ui->check_force_single_color->isChecked() && current_sprite+1 < opt.data.value("sprites").toArray().count())
        {
            QJsonObject next_sprite_obj = opt.data.value("sprites").toArray().at(current_sprite+1).toObject();
            next_sprite_obj.insert("mc_mode", false);
            sprites_array.removeAt(current_sprite+1);
            sprites_array.insert(current_sprite+1, next_sprite_obj);
        }
        opt.data.insert("sprites", sprites_array);

        this->ui->combo_overlay_color->setCurrentIndex(opt.data.value("sprites").toArray().at(current_sprite+1).toObject().value("sprite_color").toInt());


        this->ui->graphicsView->scene()->update();});

    connect(this->ui->checkBox_editor_grid_lines, &QCheckBox::toggled, [=](bool val){ this->opt.show_grid_lines = val; this->ui->graphicsView->scene()->update();});
    connect(this->ui->spin_horizontal_spacing, &QSpinBox::valueChanged, [=](int val){ this->opt.sprite_spacing_x = val; this->ui->graphicsView->redraw();});
    connect(this->ui->spin_vertical_spacing, &QSpinBox::valueChanged, [=](int val){ this->opt.sprite_spacing_y = val; this->ui->graphicsView->redraw();});
    connect(this->ui->spin_sprites_per_line, &QSpinBox::valueChanged, [=](int val){ this->opt.sprites_per_row = val; this->ui->graphicsView->redraw();});
    connect(this->ui->button_spacing_color, &QPushButton::clicked, [=](){ QColor col = QColorDialog::getColor(opt.background, 0, "Background Color"); if (col.isValid()) this->opt.background = col; this->ui->graphicsView->redraw();});
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
        opt.data = FileIO().read_spd(path);
        //FileIO().write_spd("/tmp/test.bin", opt.data);

        QFile f1(path);
        f1.open(QIODevice::ReadOnly);
        //QFile f2("/tmp/test.bin");
        //f2.open(QIODevice::ReadOnly);
        QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
        hash1.addData(f1.readAll());
        //QCryptographicHash hash2(QCryptographicHash::Algorithm::Sha256);
        //hash2.addData(f2.readAll());
        f1.close();
        //f2.close();
        //if (hash1.result() == hash2.result())
        //    qDebug() << "hash ok" << hash1.result().toHex();
        //else qDebug() << "hash_fail";



        settings.setValue("last_file", path);
        this->ui->combo_multicol_1->setCurrentIndex(opt.data.value("mc1").toInt());
        this->ui->combo_multicol_2->setCurrentIndex(opt.data.value("mc2").toInt());
        this->ui->combo_transparent->setCurrentIndex(opt.data.value("background").toInt());
        this->ui->graphicsView->change_current_sprite(0);
        this->ui->graphicsView->redraw();
    }
}



void MainWindow::on_actionOpenProject_triggered()
{
    QSettings settings;
    QString path = QFileDialog::getOpenFileName(this, "File", settings.value("last_file", QVariant()).toString(), "All Files (*);;Sprite Files(*.spd *.prg)");
    this->import(path);
}


void MainWindow::on_actionCut_triggered()
{
    this->on_actionCopy_triggered();
    this->on_actionDelete_Sprite_triggered();
}


void MainWindow::on_actionCopy_triggered()
{
    QSettings settings;
    settings.setValue("copied_sprite",QJsonDocument(this->opt.data.value("sprites").toArray().at(current_sprite).toObject()).toJson(QJsonDocument::Compact));
}


void MainWindow::on_actionPaste_triggered()
{
    QJsonParseError error;
    QSettings settings;
    QJsonObject copied_sprite = QJsonDocument::fromJson(settings.value("copied_sprite").toString().toUtf8(), &error).object();
    if (error.error != QJsonParseError::NoError) return;

    QJsonArray sprite_array = this->opt.data.value("sprites").toArray();
    //sprite_array.removeAt(current_sprite);
    sprite_array.insert(current_sprite, copied_sprite);
    this->opt.data.insert("sprites", sprite_array);
    this->ui->graphicsView->redraw();
}


void MainWindow::on_actionPaste_Into_triggered()
{
    //TODO
    /*
    for (int i = 0; i < 64; i++)
    {
        if (this->copied_sprite_data[i] != 0)
            this->opt.sprite_list.at(current_sprite)->sprite_data[i] = this->copied_sprite_data[i];
    }
    */
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionClear_triggered()
{
    QJsonObject current_sprite_obj = opt.data.value("sprites").toArray().at(current_sprite).toObject();
    QJsonArray array_y;
    for (int y = 0; y < 21; y++)
    {
        QJsonArray array_x;
        for (int x = 0; x < 24; x++)
        {
            array_x.append(0);
        }
        array_y.append(array_x);
    }
    current_sprite_obj.insert("sprite_data", array_y);

    QJsonArray sprites_array = opt.data.value("sprites").toArray();
    sprites_array.removeAt(current_sprite);
    sprites_array.insert(current_sprite, current_sprite_obj);
    opt.data.insert("sprites", sprites_array);
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
    if (this->opt.data.value("sprites").toArray().at(current_sprite).toObject().value("mc_mode").toBool())
        this->opt.sprite_list.at(current_sprite)->slide_left();
    this->ui->graphicsView->scene()->update();
}


void MainWindow::on_actionSlide_Right_triggered()
{
    this->opt.sprite_list.at(current_sprite)->slide_right();
    if (this->opt.data.value("sprites").toArray().at(current_sprite).toObject().value("mc_mode").toBool())
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



void MainWindow::on_actionSave_Project_triggered()
{
    QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
    FileIO().write_spd(QSettings().value("last_file").toString(), opt.data);

    QFile f(QSettings().value("last_file").toString());
    f.open(QIODevice::ReadOnly);
    hash.addData(f.readAll());
    qDebug() << hash.result().toHex();
}


void MainWindow::on_actionSave_Project_As_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, "Save Project As", QSettings().value("last_file").toString(), "OpenSprite file (*.spd)");
    if (path != "")
        FileIO().write_spd(path, opt.data);
}

void MainWindow::on_actionDelete_Sprite_triggered()
{
    QJsonArray sprites_array = opt.data.value("sprites").toArray();
    sprites_array.removeAt(current_sprite);
    opt.data.insert("sprites", sprites_array);
    this->ui->graphicsView->redraw();
}


void MainWindow::on_actionAdd_Sprite_triggered()
{
    QJsonArray sprites_array = opt.data.value("sprites").toArray();

    QJsonObject sprite;
    QJsonArray array_rows;
    for (int y = 0; y < 21; y++)
    {
        QJsonArray array_row;
        for (int x = 0; x < 24; x++)
        {
            array_row.append( 0 );
        }
        array_rows.append(array_row);
    }
    sprite.insert("sprite_data", array_rows);
    sprite.insert("mc_mode", true);
    sprite.insert("overlay_next", false);
    sprite.insert("sprite_color", 5);
    sprites_array.insert(current_sprite, sprite);
    opt.data.insert("sprites", sprites_array);
    this->ui->graphicsView->redraw();
}


void MainWindow::on_actionAbout_triggered()
{

    QMessageBox msgBox(this);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText("Author: Johannes Winkler<br>License: GNU GPL License<br><a href='https://github.com/jowin202/OpenSprite'>https://github.com/jowin202/OpenSprite</a>");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}


void MainWindow::on_actionExport_as_triggered()
{
    QString last_file = QSettings().value("last_file").toString();
    last_file.chop(4);
    int address = QInputDialog::getInt(this, "Address", "Load Address:", 0x3000, 0, 0xFFFF,1);
    QString path = QFileDialog::getSaveFileName(this, "Export As", last_file + ".prg", "PRG File (*.prg)");
    if (path != "")
        FileIO().write_prg(path, opt.data, address);
}

