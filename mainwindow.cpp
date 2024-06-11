#include "mainwindow.h"
#include "fileio.h"
#include "sprite.h"
#include "ui_mainwindow.h"

#include "animations/animationdialog.h"
#include "exportdialog.h"
#include "settingsdialog.h"
#include "rotations/rotationdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    opt.spriteview = this->ui->graphicsView;

    this->ui->graphicsView->set_opt(&opt);
    this->ui->label_palette->showPalette(&opt);

    num0.setShortcut(Qt::Key_0);
    num1.setShortcut(Qt::Key_1);
    num2.setShortcut(Qt::Key_2);
    num3.setShortcut(Qt::Key_3);
    num4.setShortcut(Qt::Key_4);
    num5.setShortcut(Qt::Key_5);
    this->addAction(&num0);
    this->addAction(&num1);
    this->addAction(&num2);
    this->addAction(&num3);
    this->addAction(&num4);
    this->addAction(&num5);
    connect(&num0, &QAction::triggered, [=]() {
        this->ui->radio_transparent_left->setChecked(true);
    });
    connect(&num1, &QAction::triggered, [=]() { this->ui->radio_sprite_left->setChecked(true); });
    connect(&num2, &QAction::triggered, [=]() { this->ui->radio_mc1_left->setChecked(true); });
    connect(&num3, &QAction::triggered, [=]() { this->ui->radio_mc2_left->setChecked(true); });
    connect(&num4, &QAction::triggered, [=]() {
        if (this->ui->radio_overlay_color_left->isEnabled())
            this->ui->radio_overlay_color_left->setChecked(true);
    });
    connect(&num5, &QAction::triggered, [=]() {
        if (this->ui->radio_overlay_transparent_left->isEnabled())
            this->ui->radio_overlay_transparent_left->setChecked(true);
    });

    connect(this->ui->graphicsView, &SpriteView::zoom_in, this, [=]() {
        this->ui->slider_scale->setValue(this->ui->slider_scale->value() + 10);
    });
    connect(this->ui->graphicsView, &SpriteView::zoom_out, this, [=]() {
        this->ui->slider_scale->setValue(this->ui->slider_scale->value() - 10);
    });

    connect(this->ui->graphicsView, &SpriteView::current_sprite_changed, this, [=](int id) {
        opt.selection_from = id;
        opt.selection_to = id;
        this->ui->combo_sprite_col->setCurrentIndex(
            opt.data.value("sprites").toArray().at(id).toObject().value("sprite_color").toInt());
        this->ui->check_multicolor->setChecked(
            opt.data.value("sprites").toArray().at(id).toObject().value("mc_mode").toBool());
        this->ui->check_overlay->setChecked(
            opt.data.value("sprites").toArray().at(id).toObject().value("overlay_next").toBool());
        this->ui->check_exp_x->setChecked(
            opt.data.value("sprites").toArray().at(id).toObject().value("exp_x").toBool());
        this->ui->check_exp_y->setChecked(
            opt.data.value("sprites").toArray().at(id).toObject().value("exp_y").toBool());

        if (opt.data.value("sprites").toArray().at(id).toObject().value("overlay_next").toBool()
            && this->opt.data.value("sprites").toArray().count() > id + 1)
            this->ui->combo_overlay_color->setCurrentIndex(opt.data.value("sprites")
                                                               .toArray()
                                                               .at(id + 1)
                                                               .toObject()
                                                               .value("sprite_color")
                                                               .toInt());

        if (leftradio.checkedId() == BUTTONS::OVERLAY_COLOR
            || leftradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
            this->ui->radio_sprite_left->setChecked(true);
        if (rightradio.checkedId() == BUTTONS::OVERLAY_COLOR
            || rightradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
            this->ui->radio_transparent_right->setChecked(true);
        this->ui->statusbar->showMessage(QString("Current Sprite: %1").arg(id));
    });

    leftradio.addButton(this->ui->radio_transparent_left, BUTTONS::TRANSPARENT);
    leftradio.addButton(this->ui->radio_sprite_left, BUTTONS::COLOR);
    leftradio.addButton(this->ui->radio_mc1_left, BUTTONS::MC1);
    leftradio.addButton(this->ui->radio_mc2_left, BUTTONS::MC2);
    leftradio.addButton(this->ui->radio_overlay_color_left, BUTTONS::OVERLAY_COLOR);
    leftradio.addButton(this->ui->radio_overlay_transparent_left, BUTTONS::OVERLAY_TRANSPARENT);

    rightradio.addButton(this->ui->radio_transparent_right, BUTTONS::TRANSPARENT);
    rightradio.addButton(this->ui->radio_sprite_right, BUTTONS::COLOR);
    rightradio.addButton(this->ui->radio_mc1_right, BUTTONS::MC1);
    rightradio.addButton(this->ui->radio_mc2_right, BUTTONS::MC2);
    rightradio.addButton(this->ui->radio_overlay_color_right, BUTTONS::OVERLAY_COLOR);
    rightradio.addButton(this->ui->radio_overlay_transparent_right, BUTTONS::OVERLAY_TRANSPARENT);
    this->ui->radio_transparent_right->setChecked(true);

    connect(&leftradio, &QButtonGroup::idToggled, this, [=](int id, bool checked) {
        if (checked)
            this->opt.left_button = id;
    });
    connect(&rightradio, &QButtonGroup::idToggled, this, [=](int id, bool checked) {
        if (checked)
            this->opt.right_button = id;
    });

    //palettes
    connect(this->ui->label_palette, &Palette::palette_clicked, this, [=](int button, int id) {
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

    connect(this->ui->graphicsView, &SpriteView::droppedFile, [=](QString file) {
        //ask question
        QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
        hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
        QByteArray current_hash = hash1.result().toHex();

        if (opt.last_hash != current_hash) {
            QMessageBox box;
            box.setWindowTitle("OpenSprite");
            box.setText("Save changes?");
            QPushButton *yes = box.addButton("Yes", QMessageBox::YesRole);
            QPushButton *no = box.addButton("No", QMessageBox::NoRole);
            QPushButton *cancel = box.addButton("Cancel", QMessageBox::RejectRole);
            box.exec();

            if (box.clickedButton() == cancel)
                return; //dont create new project
            else if (box.clickedButton() == no) {
            } // go on
            else if (box.clickedButton() == yes) {
                this->on_actionSave_Project_triggered();
            }
        }

        this->import(file);
    });

    //combos
    for (int i = 0; i < 16; i++) {
        this->ui->combo_transparent->addItem(opt.col_names.at(i));
        this->ui->combo_transparent->setItemIcon(i, this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_sprite_col->addItem(opt.col_names.at(i));
        this->ui->combo_sprite_col->setItemIcon(i, this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_multicol_1->addItem(opt.col_names.at(i));
        this->ui->combo_multicol_1->setItemIcon(i, this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_multicol_2->addItem(opt.col_names.at(i));
        this->ui->combo_multicol_2->setItemIcon(i, this->createIconFromColor(opt.col_list.at(i)));
        this->ui->combo_overlay_color->addItem(opt.col_names.at(i));
        this->ui->combo_overlay_color->setItemIcon(i, this->createIconFromColor(opt.col_list.at(i)));
    }
    connect(this->ui->combo_transparent, &QComboBox::currentIndexChanged, this, [=](int index) {
        opt.data.insert("background", index);
        this->ui->graphicsView->redraw();
    });
    connect(this->ui->combo_sprite_col, &QComboBox::currentIndexChanged, this, [=](int index) {

        for (int i = opt.selection_from; i <= opt.selection_to; i++)
        {
            QJsonObject current_sprite_obj
                = opt.data.value("sprites").toArray().at(i).toObject();
            current_sprite_obj.insert("sprite_color", index);
            QJsonArray sprites_array = opt.data.value("sprites").toArray();
            sprites_array.removeAt(i);
            sprites_array.insert(i, current_sprite_obj);
            opt.data.insert("sprites", sprites_array);
        }

        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->combo_multicol_1, &QComboBox::currentIndexChanged, this, [=](int index) {
        opt.data.insert("mc1", index);
        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->combo_multicol_2, &QComboBox::currentIndexChanged, this, [=](int index) {
        opt.data.insert("mc2", index);
        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->combo_overlay_color, &QComboBox::currentIndexChanged, this, [=](int index) {

        for (int i = opt.selection_from; i <= opt.selection_to; i++)
        {
            if (this->opt.data.value("sprites").toArray().count() > i + 1) {
                QJsonObject current_sprite_obj
                    = opt.data.value("sprites").toArray().at(i + 1).toObject();
                current_sprite_obj.insert("sprite_color", index);
                QJsonArray sprites_array = opt.data.value("sprites").toArray();
                sprites_array.removeAt(i + 1);
                sprites_array.insert(i + 1, current_sprite_obj);
                opt.data.insert("sprites", sprites_array);
            }
        }

        this->ui->graphicsView->scene()->update();
    });

    connect(this->ui->check_multicolor, &QCheckBox::toggled, this, [=](bool val) {
        for (int i = opt.selection_from; i <= opt.selection_to; i++)
        {
            QJsonObject current_sprite_obj
                = opt.data.value("sprites").toArray().at(i).toObject();
            current_sprite_obj.insert("mc_mode", val);
            QJsonArray sprites_array = opt.data.value("sprites").toArray();
            sprites_array.removeAt(i);
            sprites_array.insert(i, current_sprite_obj);
            opt.data.insert("sprites", sprites_array);
        }

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


        for (int i = opt.selection_from; i <= opt.selection_to; i++)
        {
            QJsonObject current_sprite_obj
                = opt.data.value("sprites").toArray().at(i).toObject();
            current_sprite_obj.insert("overlay_next", val);
            QJsonArray sprites_array = opt.data.value("sprites").toArray();
            sprites_array.removeAt(i);
            sprites_array.insert(i, current_sprite_obj);
            //next sprite should be single color
            if (val && this->ui->check_force_single_color->isChecked()
                && i + 1 < opt.data.value("sprites").toArray().count()) {
                QJsonObject next_sprite_obj
                    = opt.data.value("sprites").toArray().at(i + 1).toObject();
                next_sprite_obj.insert("mc_mode", false);
                sprites_array.removeAt(i + 1);
                sprites_array.insert(i + 1, next_sprite_obj);
            }
            opt.data.insert("sprites", sprites_array);

            this->ui->combo_overlay_color->setCurrentIndex(opt.data.value("sprites")
                                                               .toArray()
                                                               .at(i + 1)
                                                               .toObject()
                                                               .value("sprite_color")
                                                               .toInt());
        }

        this->ui->graphicsView->scene()->update();
    });

    connect(this->ui->check_exp_x, &QCheckBox::toggled, this, [=](bool val) {
        for (int i = opt.selection_from; i <= opt.selection_to; i++)
        {
            QJsonObject current_sprite_obj
                = opt.data.value("sprites").toArray().at(i).toObject();
            current_sprite_obj.insert("exp_x", val);
            QJsonArray sprites_array = opt.data.value("sprites").toArray();
            sprites_array.removeAt(i);
            sprites_array.insert(i, current_sprite_obj);
            opt.data.insert("sprites", sprites_array);
        }
        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->check_exp_y, &QCheckBox::toggled, this, [=](bool val) {

        for (int i = opt.selection_from; i <= opt.selection_to; i++)
        {
            QJsonObject current_sprite_obj
                = opt.data.value("sprites").toArray().at(i).toObject();
            current_sprite_obj.insert("exp_y", val);
            QJsonArray sprites_array = opt.data.value("sprites").toArray();
            sprites_array.removeAt(i);
            sprites_array.insert(i, current_sprite_obj);
            opt.data.insert("sprites", sprites_array);
        }
        this->ui->graphicsView->scene()->update();
    });

    connect(this->ui->checkBox_editor_grid_lines, &QCheckBox::toggled, [=](bool val) {
        this->opt.show_grid_lines = val;
        this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->check_sprite_numbers, &QCheckBox::toggled, [=](bool val) {
        this->opt.show_numbers = val;
        this->ui->graphicsView->scene()->update();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
    hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
    QByteArray current_hash = hash1.result().toHex();

    if (opt.last_hash != current_hash) {
        QMessageBox box;
        box.setWindowTitle("OpenSprite");
        box.setText("Save changes before closing?");
        QPushButton *yes = box.addButton("Yes", QMessageBox::YesRole);
        QPushButton *no = box.addButton("No", QMessageBox::NoRole);
        QPushButton *cancel = box.addButton("Cancel", QMessageBox::RejectRole);
        box.exec();

        if (box.clickedButton() == cancel) {
            ev->ignore();
            return;
        } else if (box.clickedButton() == no) {
            ev->accept();
            qApp->quit();
        } else if (box.clickedButton() == yes) {
            this->on_actionSave_Project_triggered();
            if (opt.last_hash == current_hash) {
                ev->accept();
                qApp->quit();
            } else
                ev->ignore();
        }
    } else
        qApp->quit();
}

void MainWindow::import(QString path)
{
    QSettings settings;
    if (path != "") {
        int address = 0;
        if (path.endsWith(".spd", Qt::CaseInsensitive))
            opt.data = FileIO().read_spd(path);
        else if (path.endsWith(".bin", Qt::CaseInsensitive)
                 || path.endsWith(".prg", Qt::CaseInsensitive))
            opt.data = FileIO().read_prg_bin(path, &address);
        else if (path.endsWith(".spm"))
        {
            opt.data = FileIO().read_spm(path);
            //qDebug() << opt.data;
        }

        if (address != 0)
            opt.export_address = address;

        //detect changes
        QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
        hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
        opt.last_hash = hash1.result().toHex();

        settings.setValue("last_file", path);

        if (path.endsWith(".spd", Qt::CaseInsensitive) || path.endsWith(".spm", Qt::CaseInsensitive)) //only changes preset when spd or spm
        {
            this->ui->combo_multicol_1->setCurrentIndex(opt.data.value("mc1").toInt());
            this->ui->combo_multicol_2->setCurrentIndex(opt.data.value("mc2").toInt());
            this->ui->combo_transparent->setCurrentIndex(opt.data.value("background").toInt());

            opt.last_saved_file = path;  //activate autosave on spd files
            opt.last_exported_file = ""; //reset export
        } else {
            opt.data.insert("background", this->ui->combo_transparent->currentIndex());
            opt.data.insert("mc1", this->ui->combo_multicol_1->currentIndex());
            opt.data.insert("mc2", this->ui->combo_multicol_2->currentIndex());

            opt.last_saved_file = "";    //reset save
            opt.last_exported_file = ""; //reset export
        }

        //close animations
        if (this->animation_dialog != 0)
            this->animation_dialog->close();

        opt.undoDB.clear();
        this->ui->graphicsView->change_current_sprite(0);
        this->ui->graphicsView->redraw();
    }
}

void MainWindow::new_project()
{
    opt.last_exported_file = "";
    opt.last_saved_file = "";
    opt.undoDB.clear();
    opt.data.insert("sprites", QJsonArray());
    this->on_actionAdd_Sprite_triggered();
    this->ui->combo_transparent->setCurrentIndex(6);
    this->ui->combo_multicol_2->setCurrentIndex(1);

    if (this->animation_dialog != 0)
        this->animation_dialog->close();

    //detect changes
    QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
    hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
    opt.last_hash = hash1.result().toHex();
}

void MainWindow::on_actionOpenProject_triggered()
{
    //ask question
    QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
    hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
    QByteArray current_hash = hash1.result().toHex();

    if (opt.last_hash != current_hash) {
        QMessageBox box;
        box.setWindowTitle("OpenSprite");
        box.setText("Save changes?");
        QPushButton *yes = box.addButton("Yes", QMessageBox::YesRole);
        QPushButton *no = box.addButton("No", QMessageBox::NoRole);
        QPushButton *cancel = box.addButton("Cancel", QMessageBox::RejectRole);
        box.exec();

        if (box.clickedButton() == cancel)
            return; //dont create new project
        else if (box.clickedButton() == no) {
        } // go on
        else if (box.clickedButton() == yes) {
            this->on_actionSave_Project_triggered();
        }
    }

    QSettings settings;
    QString path = QFileDialog::getOpenFileName(this,
                                                "File",
                                                settings.value("last_file", QVariant()).toString(),
                                                "Sprite Files(*.spd *.prg);;All Files (*)");
    this->import(path);
}

void MainWindow::on_actionCut_triggered()
{
    this->on_actionCopy_triggered();
    this->on_actionDelete_Sprite_triggered();
}

void MainWindow::on_actionCopy_triggered()
{
    //there is no sub-array function
    QJsonArray copied_sprites;
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        copied_sprites.append(this->opt.data.value("sprites").toArray().at(i).toObject());

    QSettings settings;
    settings.setValue("copied_sprites",
                      QJsonDocument(copied_sprites)
                          .toJson(QJsonDocument::Compact));
}

void MainWindow::on_actionPaste_triggered()
{
    opt.undoDB.append(opt.data);
    QJsonParseError error;
    QSettings settings;
    QJsonArray copied_sprites
        = QJsonDocument::fromJson(settings.value("copied_sprites").toString().toUtf8(), &error)
              .array();
    if (error.error != QJsonParseError::NoError)
        return;

    QJsonArray sprite_array = this->opt.data.value("sprites").toArray();
    //this->ui->combo_sprite_col->setCurrentIndex(copied_sprites.value("sprite_color").toInt());
    //sprite_array.removeAt(current_sprite);
    for (int i = copied_sprites.count()-1; i >= 0; i--)
        sprite_array.insert(opt.selection_from, copied_sprites[i]);
    this->opt.data.insert("sprites", sprite_array);
    this->ui->graphicsView->redraw();
}

void MainWindow::on_actionPaste_Into_triggered()
{
    QMessageBox::information(this, "Not implemented yet", "Not implemented yet");
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
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
    {
        QJsonObject current_sprite_obj
            = opt.data.value("sprites").toArray().at(i).toObject();
        QJsonArray array_y;
        for (int y = 0; y < 21; y++) {
            QJsonArray array_x;
            for (int x = 0; x < 24; x++) {
                array_x.append(0);
            }
            array_y.append(array_x);
        }
        current_sprite_obj.insert("sprite_data", array_y);

        QJsonArray sprites_array = opt.data.value("sprites").toArray();
        sprites_array.removeAt(i);
        sprites_array.insert(i, current_sprite_obj);
        opt.data.insert("sprites", sprites_array);
    }
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionSlide_Up_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->slide_up();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionSlide_Down_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->slide_down();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionSlide_Left_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
    {
        this->opt.sprite_list.at(i)->slide_left();
        if (this->opt.data.value("sprites")
                .toArray()
                .at(i)
                .toObject()
                .value("mc_mode")
                .toBool())
            this->opt.sprite_list.at(i)->slide_left();
    }
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionSlide_Right_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
    {
        this->opt.sprite_list.at(i)->slide_right();
        if (this->opt.data.value("sprites")
                .toArray()
                .at(i)
                .toObject()
                .value("mc_mode")
                .toBool())
            this->opt.sprite_list.at(i)->slide_right();
    }
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_slider_scale_valueChanged(int value)
{
    qreal scale = qPow(qreal(2), (value - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);
    this->ui->graphicsView->setTransform(matrix);
}

void MainWindow::on_actionFlip_Top_to_Bottom_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->flip_top();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionFlip_Left_to_Right_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->flip_left();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionSave_Project_triggered()
{
    if (opt.last_saved_file == "") {
        this->on_actionSave_Project_As_triggered();
    } else {
        //auto save
        //TODO REMOVE THIS
        if (opt.data.value("version").toInt() > 0)
        {
            QMessageBox::critical(this, "Unsupported format", "Only spd files v1 can be saved", QMessageBox::Ok);
            return;
        }
        //TODO REMOVE THIS
        FileIO().write_spd(opt.last_saved_file, opt.data);

        //detect changes
        QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
        hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
        opt.last_hash = hash1.result().toHex();
    }
}

void MainWindow::on_actionSave_Project_As_triggered()
{
    QString path = QFileDialog::getSaveFileName(this,
                                                "Save Project As",
                                                opt.last_saved_file == ""
                                                    ? QDir::homePath() + QDir::separator()
                                                          + "Untitled.spd"
                                                    : opt.last_saved_file,
                                                "OpenSprite file (*.spd)");
    if (path != "") {
        //TODO REMOVE THIS
        if (opt.data.value("version").toInt() > 0)
        {
            QMessageBox::critical(this, "Unsupported format", "Only spd files v1 can be saved", QMessageBox::Ok);
            return;
        }
        //TODO REMOVE THIS
        FileIO().write_spd(path, opt.data);
        opt.last_saved_file = path;

        QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
        hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
        opt.last_hash = hash1.result().toHex();
    }
}

void MainWindow::on_actionDelete_Sprite_triggered()
{
    opt.undoDB.append(opt.data);
    QJsonArray sprites_array = opt.data.value("sprites").toArray();
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        sprites_array.removeAt(opt.selection_from);
    opt.selection_from = qMin(opt.selection_from, sprites_array.count() - 1);
    opt.selection_to = qMin(opt.selection_to, sprites_array.count() - 1);
    opt.data.insert("sprites", sprites_array);
    this->ui->graphicsView->redraw();
}

void MainWindow::on_actionAdd_Sprite_triggered()
{
    this->ui->graphicsView->add_new_sprite();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox(this);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
        "Version: 1.42 (06 / 2024)<br>Author: Johannes Winkler<br>License: GNU GPL License<br><a "
        "href='https://github.com/jowin202/OpenSprite'>https://github.com/jowin202/OpenSprite</a>");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::on_actionExport_as_triggered()
{
    ExportDialog *d = new ExportDialog(&opt);
    d->show();
}

void MainWindow::on_actionExport_triggered()
{
    if (opt.last_exported_file == "") {
        ExportDialog *d = new ExportDialog(&opt);
        d->show();
    } else { //auto export
        FileIO().write_prg(opt.last_exported_file,
                           opt.data,
                           opt.export_address,
                           opt.export_attribute_format);
    }
}

void MainWindow::on_actionReflect_Left_To_Right_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->reflect_left();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionReflect_Top_to_Bottom_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->reflect_top();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionNew_triggered()
{
    //ask question
    QCryptographicHash hash1(QCryptographicHash::Algorithm::Sha256);
    hash1.addData(QJsonDocument(opt.data).toJson(QJsonDocument::Compact));
    QByteArray current_hash = hash1.result().toHex();

    if (opt.last_hash != current_hash) {
        QMessageBox box;
        box.setWindowTitle("OpenSprite");
        box.setText("Save changes?");
        QPushButton *yes = box.addButton("Yes", QMessageBox::YesRole);
        QPushButton *no = box.addButton("No", QMessageBox::NoRole);
        QPushButton *cancel = box.addButton("Cancel", QMessageBox::RejectRole);
        box.exec();

        if (box.clickedButton() == cancel)
            return; //dont create new project
        else if (box.clickedButton() == no) {
        } // go on
        else if (box.clickedButton() == yes) {
            this->on_actionSave_Project_triggered();
        }
    }
    this->new_project();
}

void MainWindow::on_actionUndo_triggered()
{
    if (!opt.undoDB.isEmpty()) {
        opt.data = opt.undoDB.last();
        opt.undoDB.removeLast();
        this->ui->graphicsView->redraw();
    }
}

void MainWindow::on_actionAnimations_Editor_triggered()
{
    //doing a singleton.
    if (animation_dialog == 0) {
        animation_dialog = new AnimationDialog(&opt);
        connect(animation_dialog, &AnimationDialog::dialog_closed, [=]() {
            this->animation_dialog = 0;
        });
        animation_dialog->show();
    } else {
        animation_dialog->raise();
    }
}

void MainWindow::on_actionSprite_Color_MC_1_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->switch_col_to_mc1();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionSprite_Color_MC_2_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->switch_col_to_mc2();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionMC_1_MC_2_triggered()
{
    opt.undoDB.append(opt.data);
    for (int i = opt.selection_from; i <= opt.selection_to; i++)
        this->opt.sprite_list.at(i)->switch_mc1_to_mc2();
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionRotate_triggered()
{
    RotationDialog *dialog = new RotationDialog(&opt);
    dialog->show();
    connect(dialog, &RotationDialog::finished, [=]() { this->ui->graphicsView->redraw(); });
}

void MainWindow::on_actionPreferences_triggered()
{
    SettingsDialog *dialog = new SettingsDialog(&opt);
    dialog->show();
    connect(dialog, &SettingsDialog::finished, [=]() { this->ui->graphicsView->redraw(); });
}


void MainWindow::on_actionSelection_to_Animation_triggered()
{
    QJsonArray animation_array = opt.data.value("animations").toArray();
    QJsonObject new_animation;
    new_animation.insert("from", opt.selection_from);
    new_animation.insert("to", opt.selection_to);
    new_animation.insert("overlay", false);
    new_animation.insert("pingpong", false);
    new_animation.insert("valid", true);
    new_animation.insert("timer", 16);
    animation_array.append(new_animation);
    opt.data.insert("animations", animation_array);

    this->on_actionAnimations_Editor_triggered();
}


void MainWindow::on_actionSelect_All_triggered()
{
    this->opt.selection_from = 0;
    this->opt.selection_to = this->opt.sprite_list.length()-1;
    this->ui->graphicsView->redraw();
}

