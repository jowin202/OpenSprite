#include "mainwindow.h"
#include "fileio.h"
#include "sprite.h"
#include "ui_mainwindow.h"

#include "animations/animationdialog.h"
#include "exportdialog.h"
#include "settingsdialog.h"
#include "rotations/rotationdialog.h"
#include "scale/scalingdialog.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    opt.spriteview = this->ui->graphicsView;

    this->ui->graphicsView->set_opt(&opt);
    this->ui->label_palette->showPalette(&opt);

    // Redo: Ctrl+Y (from the ui file) plus the platform default (e.g. Ctrl+Shift+Z)
    QList<QKeySequence> redo_keys = {this->ui->actionRedo->shortcut()};
    for (const QKeySequence &k : QKeySequence::keyBindings(QKeySequence::Redo))
        if (!redo_keys.contains(k))
            redo_keys.append(k);
    this->ui->actionRedo->setShortcuts(redo_keys);

    // number keys select the tool of the left mouse button
    QAction *num_actions[] = {&num0, &num1, &num2, &num3, &num4, &num5};
    QRadioButton *num_radios[] = {this->ui->radio_transparent_left,
                                  this->ui->radio_sprite_left,
                                  this->ui->radio_mc1_left,
                                  this->ui->radio_mc2_left,
                                  this->ui->radio_overlay_color_left,
                                  this->ui->radio_overlay_transparent_left};
    for (int i = 0; i < 6; i++) {
        num_actions[i]->setShortcut(Qt::Key_0 + i);
        this->addAction(num_actions[i]);
        QRadioButton *radio = num_radios[i];
        connect(num_actions[i], &QAction::triggered, this, [radio]() {
            if (radio->isEnabled())
                radio->setChecked(true);
        });
    }

    connect(this->ui->graphicsView, &SpriteView::zoom_in, this, [=]() {
        this->ui->slider_scale->setValue(this->ui->slider_scale->value() + 10);
    });
    connect(this->ui->graphicsView, &SpriteView::zoom_out, this, [=]() {
        this->ui->slider_scale->setValue(this->ui->slider_scale->value() - 10);
    });

    connect(this->ui->graphicsView, &SpriteView::current_sprite_changed, this, [=](int id) {
        opt.selection_from = id;
        opt.selection_to = id;
        this->sync_ui_from_sprite(id);
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

    //palettes: a click sets the color of the tool on that mouse button
    connect(this->ui->label_palette, &Palette::palette_clicked, this, [=](int button, int id) {
        int tool;
        if (button == Qt::LeftButton)
            tool = leftradio.checkedId();
        else if (button == Qt::RightButton)
            tool = rightradio.checkedId();
        else
            return;

        switch (tool) {
        case BUTTONS::TRANSPARENT:
        case BUTTONS::OVERLAY_TRANSPARENT:
            this->ui->combo_transparent->setCurrentIndex(id);
            break;
        case BUTTONS::COLOR:
            this->ui->combo_sprite_col->setCurrentIndex(id);
            break;
        case BUTTONS::MC1:
            this->ui->combo_multicol_1->setCurrentIndex(id);
            break;
        case BUTTONS::MC2:
            this->ui->combo_multicol_2->setCurrentIndex(id);
            break;
        case BUTTONS::OVERLAY_COLOR:
            this->ui->combo_overlay_color->setCurrentIndex(id);
            break;
        }
    });

    connect(this->ui->graphicsView, &SpriteView::droppedFile, this, &MainWindow::open_file);

    //combos
    QComboBox *combos[] = {this->ui->combo_transparent,
                           this->ui->combo_sprite_col,
                           this->ui->combo_multicol_1,
                           this->ui->combo_multicol_2,
                           this->ui->combo_overlay_color};
    for (int i = 0; i < opt.col_list.size(); i++) {
        const QIcon icon = this->createIconFromColor(opt.col_list.at(i));
        for (QComboBox *combo : combos)
            combo->addItem(icon, opt.col_names.at(i));
    }

    connect(this->ui->combo_transparent, &QComboBox::currentIndexChanged, this, [=](int index) {
        set_project_value("background", index);
    });
    connect(this->ui->combo_multicol_1, &QComboBox::currentIndexChanged, this, [=](int index) {
        set_project_value("mc1", index);
    });
    connect(this->ui->combo_multicol_2, &QComboBox::currentIndexChanged, this, [=](int index) {
        set_project_value("mc2", index);
    });
    connect(this->ui->combo_sprite_col, &QComboBox::currentIndexChanged, this, [=](int index) {
        if (index >= 0 && set_selection_value("sprite_color", index))
            this->ui->graphicsView->scene()->update();
    });
    connect(this->ui->combo_overlay_color, &QComboBox::currentIndexChanged, this, [=](int index) {
        // the overlay color is the color of the next sprite
        if (index >= 0 && set_selection_value("sprite_color", index, 1))
            this->ui->graphicsView->scene()->update();
    });

    connect(this->ui->check_multicolor, &QCheckBox::toggled, this, [=](bool val) {
        set_selection_value("mc_mode", val);
        update_mode_controls();
        this->ui->graphicsView->scene()->update();
    });

    connect(this->ui->check_overlay, &QCheckBox::toggled, this, [=](bool val) {
        opt.push_undo();
        for (int i = opt.selection_from; i <= opt.selection_to && i < opt.sprite_count(); i++) {
            SpriteData::set_sprite_value(opt.data, i, "overlay_next", val);
            if (i + 1 < opt.sprite_count()) {
                //next sprite should be single color
                if (val && this->ui->check_force_single_color->isChecked())
                    SpriteData::set_sprite_value(opt.data, i + 1, "mc_mode", false);

                const QSignalBlocker blocker(this->ui->combo_overlay_color);
                this->ui->combo_overlay_color->setCurrentIndex(
                    SpriteData::sprite_at(opt.data, i + 1).value("sprite_color").toInt());
            }
        }
        update_mode_controls();
        this->ui->graphicsView->scene()->update();
    });

    // expansion changes the size of the sprite items, so the view is rebuilt
    connect(this->ui->check_exp_x, &QCheckBox::toggled, this, [=](bool val) {
        if (set_selection_value("exp_x", val))
            this->ui->graphicsView->redraw();
    });
    connect(this->ui->check_exp_y, &QCheckBox::toggled, this, [=](bool val) {
        if (set_selection_value("exp_y", val))
            this->ui->graphicsView->redraw();
    });

    connect(this->ui->checkBox_editor_grid_lines, &QCheckBox::toggled, this, [=](bool val) {
        this->opt.show_grid_lines = val;
        this->ui->graphicsView->scene()->update();
        quickEditor->update();
    });
    connect(this->ui->check_sprite_numbers, &QCheckBox::toggled, this, [=](bool val) {
        this->opt.show_numbers = val;
        this->ui->graphicsView->scene()->update();
    });

    // ── Quick Editor ──────────────────────────────────────────────────────────
    quickEditor = new QuickEditorWidget(this->ui->groupBox);
    quickEditor->set_opt(&opt);
    quickEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Shift all existing sidebar items down by one row to make room at row 0
    QGridLayout *sideLayout = qobject_cast<QGridLayout*>(this->ui->groupBox->layout());
    for (int i = sideLayout->count() - 1; i >= 0; i--) {
        int r, c, rs, cs;
        sideLayout->getItemPosition(i, &r, &c, &rs, &cs);
        QLayoutItem *item = sideLayout->takeAt(i);
        sideLayout->addItem(item, r + 1, c, rs, cs);
    }
    sideLayout->addWidget(quickEditor, 0, 0, 1, 3);

    // Restore saved visibility state (default: off)
    QSettings qeSettings;
    bool qeEnabled = qeSettings.value("quickeditor_visible", false).toBool();
    this->ui->actionQuick_Editor->setChecked(qeEnabled);
    quickEditor->setVisible(qeEnabled);
    this->ui->groupBox->setTitle(qeEnabled ? tr("Quick Editor and Options") : tr("Options"));

    // Sync Quick Editor and title whenever the scene updates (catches all edits + redraws)
    connect(this->ui->graphicsView->scene(), &QGraphicsScene::changed, this, [=]() {
        if (quickEditor->isVisible()) quickEditor->update();
        update_window_title();
    });

    // After a Quick Editor edit, sync the main scene
    connect(quickEditor, &QuickEditorWidget::spriteEdited, this, [=]() {
        this->ui->graphicsView->scene()->update();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ── save handling ─────────────────────────────────────────────────────────────

bool MainWindow::maybe_save(const QString &question)
{
    opt.end_edit();
    if (!opt.is_modified())
        return true;

    QMessageBox box(this);
    box.setWindowTitle("OpenSprite");
    box.setText(question);
    QPushButton *yes = box.addButton("Yes", QMessageBox::YesRole);
    QPushButton *no = box.addButton("No", QMessageBox::NoRole);
    QPushButton *cancel = box.addButton("Cancel", QMessageBox::RejectRole);
    box.setDefaultButton(yes);
    box.setEscapeButton(cancel);
    box.exec();

    if (box.clickedButton() == no)
        return true;
    if (box.clickedButton() == yes)
        return save_project(); // false if "save as" was cancelled or saving failed
    return false;
}

bool MainWindow::save_project()
{
    if (opt.last_saved_file.isEmpty())
        return save_project_as();

    //auto save
    opt.end_edit();
    if (!FileIO::write_spd(opt.last_saved_file, opt.data, this))
        return false;
    opt.mark_saved();
    current_file = opt.last_saved_file;
    this->statusBar()->showMessage("Saved to: " + opt.last_saved_file);
    update_window_title();
    return true;
}

bool MainWindow::save_project_as()
{
    QString suggestion = opt.last_saved_file;
    if (suggestion.isEmpty()) {
        QFileInfo info(current_file);
        suggestion = current_file.isEmpty()
                         ? QDir::homePath() + QDir::separator() + "Untitled.spd"
                         : info.absolutePath() + QDir::separator() + info.completeBaseName() + ".spd";
    }
    QString path = QFileDialog::getSaveFileName(this, "Save Project As", suggestion, "OpenSprite file (*.spd)");
    if (path.isEmpty())
        return false;
    if (!path.endsWith(".spd", Qt::CaseInsensitive))
        path += ".spd";

    opt.end_edit();
    if (!FileIO::write_spd(path, opt.data, this))
        return false;
    opt.last_saved_file = path;
    opt.mark_saved();
    current_file = path;
    this->statusBar()->showMessage("Saved to: " + opt.last_saved_file);
    update_window_title();
    return true;
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if (!maybe_save("Save changes before closing?")) {
        ev->ignore();
        return;
    }
    ev->accept();
    qApp->quit();
}

// ── ui <-> data ───────────────────────────────────────────────────────────────

void MainWindow::update_mode_controls()
{
    const bool mc = this->ui->check_multicolor->isChecked();
    this->ui->radio_mc1_left->setEnabled(mc);
    this->ui->radio_mc1_right->setEnabled(mc);
    this->ui->radio_mc2_left->setEnabled(mc);
    this->ui->radio_mc2_right->setEnabled(mc);
    this->ui->combo_multicol_1->setEnabled(mc);
    this->ui->combo_multicol_2->setEnabled(mc);
    if (!mc) {
        if (this->ui->radio_mc1_left->isChecked() || this->ui->radio_mc2_left->isChecked())
            this->ui->radio_sprite_left->setChecked(true);
        if (this->ui->radio_mc1_right->isChecked() || this->ui->radio_mc2_right->isChecked())
            this->ui->radio_transparent_right->setChecked(true);
    }

    const bool overlay = this->ui->check_overlay->isChecked();
    this->ui->radio_overlay_color_left->setEnabled(overlay);
    this->ui->radio_overlay_color_right->setEnabled(overlay);
    this->ui->radio_overlay_transparent_left->setEnabled(overlay);
    this->ui->radio_overlay_transparent_right->setEnabled(overlay);
    this->ui->combo_overlay_color->setEnabled(overlay);
    if (!overlay) {
        if (this->ui->radio_overlay_color_left->isChecked())
            this->ui->radio_sprite_left->setChecked(true);
        if (this->ui->radio_overlay_color_right->isChecked())
            this->ui->radio_sprite_right->setChecked(true);
        if (this->ui->radio_overlay_transparent_left->isChecked())
            this->ui->radio_transparent_left->setChecked(true);
        if (this->ui->radio_overlay_transparent_right->isChecked())
            this->ui->radio_transparent_right->setChecked(true);
    }
}

// shows the properties of the sprite without modifying the project
void MainWindow::sync_ui_from_sprite(int id)
{
    if (id < 0 || id >= opt.sprite_count())
        return;
    const QJsonObject sprite = SpriteData::sprite_at(opt.data, id);
    {
        const QSignalBlocker b1(this->ui->combo_sprite_col);
        const QSignalBlocker b2(this->ui->check_multicolor);
        const QSignalBlocker b3(this->ui->check_overlay);
        const QSignalBlocker b4(this->ui->check_exp_x);
        const QSignalBlocker b5(this->ui->check_exp_y);
        const QSignalBlocker b6(this->ui->combo_overlay_color);

        this->ui->combo_sprite_col->setCurrentIndex(sprite.value("sprite_color").toInt());
        this->ui->check_multicolor->setChecked(sprite.value("mc_mode").toBool());
        this->ui->check_overlay->setChecked(sprite.value("overlay_next").toBool());
        this->ui->check_exp_x->setChecked(sprite.value("exp_x").toBool());
        this->ui->check_exp_y->setChecked(sprite.value("exp_y").toBool());
        if (sprite.value("overlay_next").toBool() && id + 1 < opt.sprite_count())
            this->ui->combo_overlay_color->setCurrentIndex(
                SpriteData::sprite_at(opt.data, id + 1).value("sprite_color").toInt());
    }
    update_mode_controls();

    if (leftradio.checkedId() == BUTTONS::OVERLAY_COLOR
        || leftradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
        this->ui->radio_sprite_left->setChecked(true);
    if (rightradio.checkedId() == BUTTONS::OVERLAY_COLOR
        || rightradio.checkedId() == BUTTONS::OVERLAY_TRANSPARENT)
        this->ui->radio_transparent_right->setChecked(true);
    this->ui->statusbar->showMessage(QString("Current Sprite: %1").arg(id));
    if (quickEditor)
        quickEditor->update();
}

void MainWindow::sync_global_ui()
{
    const QSignalBlocker b1(this->ui->combo_transparent);
    const QSignalBlocker b2(this->ui->combo_multicol_1);
    const QSignalBlocker b3(this->ui->combo_multicol_2);
    this->ui->combo_transparent->setCurrentIndex(opt.data.value("background").toInt());
    this->ui->combo_multicol_1->setCurrentIndex(opt.data.value("mc1").toInt());
    this->ui->combo_multicol_2->setCurrentIndex(opt.data.value("mc2").toInt());
}

void MainWindow::update_window_title()
{
    const QString name = current_file.isEmpty() ? QString("Untitled") : QFileInfo(current_file).fileName();
    this->setWindowTitle(name + "[*] - OpenSprite");
    this->setWindowModified(opt.is_modified());
}

void MainWindow::data_replaced()
{
    opt.clamp_selection();
    this->ui->graphicsView->redraw();
    sync_global_ui();
    sync_ui_from_sprite(opt.selection_from);
    if (animation_dialog)
        animation_dialog->fill_list();
    update_window_title();
}

// ── helpers ───────────────────────────────────────────────────────────────────

void MainWindow::apply_to_selection(void (Sprite::*fn)(), bool twice_in_mc_mode)
{
    opt.push_undo();
    for (int i = opt.selection_from; i <= opt.selection_to && i < opt.sprite_list.size(); i++) {
        Sprite *sprite = opt.sprite_list.at(i);
        (sprite->*fn)();
        if (twice_in_mc_mode && sprite->is_mc())
            (sprite->*fn)();
    }
    this->ui->graphicsView->scene()->update();
}

bool MainWindow::set_selection_value(const QString &key, const QJsonValue &value, int offset)
{
    QList<int> targets;
    for (int i = opt.selection_from; i <= opt.selection_to; i++) {
        const int j = i + offset;
        if (j >= 0 && j < opt.sprite_count() && SpriteData::sprite_at(opt.data, j).value(key) != value)
            targets.append(j);
    }
    if (targets.isEmpty())
        return false;

    opt.push_undo();
    for (int j : targets)
        SpriteData::set_sprite_value(opt.data, j, key, value);
    return true;
}

void MainWindow::set_project_value(const QString &key, int value)
{
    if (value < 0 || opt.data.value(key).toInt(-1) == value)
        return;
    opt.push_undo();
    opt.data.insert(key, value);
    this->ui->graphicsView->scene()->update();
}

void MainWindow::show_dialog(QWidget *dialog)
{
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

QJsonArray MainWindow::copied_sprites() const
{
    QJsonParseError error;
    QSettings settings;
    const QJsonDocument doc = QJsonDocument::fromJson(settings.value("copied_sprites").toByteArray(), &error);
    if (error.error != QJsonParseError::NoError)
        return QJsonArray();
    return doc.array();
}

// ── file handling ─────────────────────────────────────────────────────────────

bool MainWindow::import(QString path)
{
    if (path.isEmpty())
        return false;

    QJsonObject data;
    int address = 0;
    const QString suffix = QFileInfo(path).suffix().toLower();
    const bool project_file = (suffix == "spd" || suffix == "spm");

    if (suffix == "spd") {
        data = FileIO::read_spd(path, this);
        if (data.value("version").toInt() > 1)
        {
            QMessageBox::warning(this, "Unsupported format", "Attention! Your file is in a newer spd version format. It will be saved in spd V1 format.", QMessageBox::Ok);
        }
    } else if (suffix == "bin" || suffix == "prg") {
        data = FileIO::read_prg_bin(path, &address, this);
    } else if (suffix == "spm") {
        data = FileIO::read_spm(path, this);
    } else {
        QMessageBox::critical(this, "Unsupported file", "Unsupported file type: " + path);
        return false;
    }

    if (SpriteData::sprite_count(data) == 0)
        return false; // error was already reported, keep the current project

    if (!data.contains("animations"))
        data.insert("animations", QJsonArray());

    if (!project_file) {
        //bin/prg contain no colors: keep the current ones
        data.insert("background", this->ui->combo_transparent->currentIndex());
        data.insert("mc1", this->ui->combo_multicol_1->currentIndex());
        data.insert("mc2", this->ui->combo_multicol_2->currentIndex());
    }

    opt.data = data;
    if (address != 0)
        opt.export_address = address;

    QSettings settings;
    settings.setValue("last_file", path);

    opt.last_saved_file = project_file ? path : ""; //activate autosave on spd/spm files
    opt.last_exported_file = "";                    //reset export
    current_file = path;

    //close animations
    if (this->animation_dialog)
        this->animation_dialog->close();

    opt.clear_history();
    opt.mark_saved();
    opt.selection_from = 0;
    opt.selection_to = 0;
    data_replaced();
    return true;
}

void MainWindow::new_project()
{
    opt.last_exported_file = "";
    opt.last_saved_file = "";
    current_file = "";

    QJsonObject data;
    data.insert("sprites", QJsonArray{SpriteData::new_sprite()});
    //create sample animation (original spritepad wants at least 1 animation)
    data.insert("animations", QJsonArray{SpriteData::new_animation(0, 0, false)});
    data.insert("background", 6);
    data.insert("mc1", qMax(0, this->ui->combo_multicol_1->currentIndex()));
    data.insert("mc2", 1);
    opt.data = data;

    if (this->animation_dialog)
        this->animation_dialog->close();

    opt.clear_history();
    opt.mark_saved();
    opt.selection_from = 0;
    opt.selection_to = 0;
    data_replaced();
}

void MainWindow::open_file(const QString &path)
{
    if (path.isEmpty() || !maybe_save("Save changes?"))
        return;
    this->import(path);
}

void MainWindow::on_actionOpenProject_triggered()
{
    if (!maybe_save("Save changes?"))
        return;

    QSettings settings;
    QString path = QFileDialog::getOpenFileName(this,
                                                "File",
                                                settings.value("last_file", QVariant()).toString(),
                                                "Sprite Files (*.spd *.spm *.prg *.bin);;"
                                                "SpritePad (*.spd);;"
                                                "Spritemate (*.spm);;"
                                                "PRG / BIN (*.prg *.bin);;"
                                                "All Files (*)");
    this->import(path);
}

void MainWindow::on_actionSave_Project_triggered()
{
    save_project();
}

void MainWindow::on_actionSave_Project_As_triggered()
{
    save_project_as();
}

void MainWindow::on_actionExport_as_triggered()
{
    ExportDialog *d = new ExportDialog(&opt, this);
    connect(d, &ExportDialog::statusmsg, this, [=](QString msg){
        this->statusBar()->showMessage(msg);
    });
    show_dialog(d);
}

void MainWindow::on_actionExport_triggered()
{
    if (opt.last_exported_file.isEmpty()) {
        this->on_actionExport_as_triggered();
    } else { //auto export
        if (FileIO::write_prg(opt.last_exported_file,
                              opt.data,
                              opt.export_file_format == 0 ? opt.export_address : -1,
                              opt.export_attribute_format,
                              this))
            this->statusBar()->showMessage("Exported to: " + opt.last_exported_file);
    }
}

void MainWindow::on_actionNew_triggered()
{
    if (maybe_save("Save changes?"))
        this->new_project();
}

// ── clipboard ─────────────────────────────────────────────────────────────────

void MainWindow::on_actionCut_triggered()
{
    this->on_actionCopy_triggered();
    this->on_actionDelete_Sprite_triggered();
}

void MainWindow::on_actionCopy_triggered()
{
    const QJsonArray sprites = this->opt.data.value("sprites").toArray();
    QJsonArray copied;
    for (int i = opt.selection_from; i <= opt.selection_to && i < sprites.count(); i++)
        copied.append(sprites.at(i));

    // stored in the settings, so it can be pasted into another instance
    QSettings settings;
    settings.setValue("copied_sprites", QJsonDocument(copied).toJson(QJsonDocument::Compact));
    this->statusBar()->showMessage(QString("Copied %1 sprite(s)").arg(copied.count()));
}

void MainWindow::on_actionPaste_triggered()
{
    const QJsonArray copied = copied_sprites();
    if (copied.isEmpty())
        return;

    opt.push_undo();
    QJsonArray sprite_array = this->opt.data.take("sprites").toArray();
    const int pos = qBound(0, opt.selection_from, int(sprite_array.count()));
    for (int i = copied.count()-1; i >= 0; i--)
        sprite_array.insert(pos, copied.at(i));
    this->opt.data.insert("sprites", sprite_array);

    // select the pasted sprites
    opt.selection_from = pos;
    opt.selection_to = pos + copied.count() - 1;
    data_replaced();
}

// draws the set pixels of the copied sprites over the selected sprites
void MainWindow::on_actionPaste_Into_triggered()
{
    const QJsonArray copied = copied_sprites();
    if (copied.isEmpty())
        return;

    opt.push_undo();
    for (int i = opt.selection_from; i <= opt.selection_to && i < opt.sprite_count(); i++) {
        const QJsonObject source = copied.at((i - opt.selection_from) % copied.count()).toObject();
        const SpriteData::Bits src = SpriteData::Bits::from_json(source);
        SpriteData::Bits dst = SpriteData::bits_at(opt.data, i);

        if (source.value("mc_mode").toBool()) {
            for (int y = 0; y < SpriteData::HEIGHT; y++)
                for (int cx = 0; cx < SpriteData::MC_WIDTH; cx++)
                    if (src.mc(cx, y) != SpriteData::MC_BACKGROUND)
                        dst.set_mc(cx, y, src.mc(cx, y));
        } else {
            for (int y = 0; y < SpriteData::HEIGHT; y++)
                for (int x = 0; x < SpriteData::WIDTH; x++)
                    if (src.get(x, y))
                        dst.set(x, y, true);
        }
        SpriteData::set_bits(opt.data, i, dst);
    }
    this->ui->graphicsView->scene()->update();
}

// ── sprite operations ─────────────────────────────────────────────────────────

void MainWindow::on_actionClear_triggered()
{
    opt.push_undo();
    for (int i = opt.selection_from; i <= opt.selection_to && i < opt.sprite_count(); i++)
        SpriteData::set_bits(opt.data, i, SpriteData::Bits());
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionSlide_Up_triggered()
{
    apply_to_selection(&Sprite::slide_up);
}

void MainWindow::on_actionSlide_Down_triggered()
{
    apply_to_selection(&Sprite::slide_down);
}

void MainWindow::on_actionSlide_Left_triggered()
{
    // multicolor pixels are two bits wide
    apply_to_selection(&Sprite::slide_left, true);
}

void MainWindow::on_actionSlide_Right_triggered()
{
    apply_to_selection(&Sprite::slide_right, true);
}

void MainWindow::on_actionFlip_Top_to_Bottom_triggered()
{
    apply_to_selection(&Sprite::flip_top);
}

void MainWindow::on_actionFlip_Left_to_Right_triggered()
{
    apply_to_selection(&Sprite::flip_left);
}

void MainWindow::on_actionReflect_Left_To_Right_triggered()
{
    apply_to_selection(&Sprite::reflect_left);
}

void MainWindow::on_actionReflect_Top_to_Bottom_triggered()
{
    apply_to_selection(&Sprite::reflect_top);
}

void MainWindow::on_actionSprite_Color_MC_1_triggered()
{
    apply_to_selection(&Sprite::switch_col_to_mc1);
}

void MainWindow::on_actionSprite_Color_MC_2_triggered()
{
    apply_to_selection(&Sprite::switch_col_to_mc2);
}

void MainWindow::on_actionMC_1_MC_2_triggered()
{
    apply_to_selection(&Sprite::switch_mc1_to_mc2);
}

void MainWindow::on_actionDelete_Sprite_triggered()
{
    opt.push_undo();
    QJsonArray sprites_array = opt.data.take("sprites").toArray();
    for (int i = opt.selection_from; i <= opt.selection_to && opt.selection_from < sprites_array.count(); i++)
        sprites_array.removeAt(opt.selection_from);
    // a project always contains at least one sprite
    if (sprites_array.isEmpty())
        sprites_array.append(SpriteData::new_sprite());
    opt.data.insert("sprites", sprites_array);

    opt.selection_from = qMin(opt.selection_from, int(sprites_array.count()) - 1);
    opt.selection_to = opt.selection_from;
    data_replaced();
}

void MainWindow::on_actionAdd_Sprite_triggered()
{
    this->ui->graphicsView->add_new_sprite();
}

void MainWindow::on_actionUndo_triggered()
{
    if (opt.undo())
        data_replaced();
}

void MainWindow::on_actionRedo_triggered()
{
    if (opt.redo())
        data_replaced();
}

void MainWindow::on_actionSelect_All_triggered()
{
    this->opt.selection_from = 0;
    this->opt.selection_to = qMax(0, opt.sprite_count() - 1);
    this->ui->graphicsView->scene()->update();
}

void MainWindow::on_actionFlood_Fill_triggered()
{
    this->opt.pen = this->ui->actionFlood_Fill->isChecked() ? PEN::FLOOD : PEN::DEFAULT;
}

// ── dialogs ───────────────────────────────────────────────────────────────────

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About OpenSprite");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(
        QString("Version: %1 (%2)").arg(QCoreApplication::applicationVersion(), APP_VERSION_DATE) +
        "<br>Author:<br>Johannes Winkler<br>Wilfried Elmenreich<br>License: GNU GPL License<br><a "
        "href='https://github.com/jowin202/OpenSprite'>https://github.com/jowin202/OpenSprite</a>");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::on_actionAnimations_Editor_triggered()
{
    //doing a singleton.
    if (!animation_dialog) {
        animation_dialog = new AnimationDialog(&opt, this);
    }
    show_dialog(animation_dialog);
}

void MainWindow::on_actionSelection_to_Animation_triggered()
{
    QJsonArray animation_array = opt.data.value("animations").toArray();
    animation_array.append(SpriteData::new_animation(opt.selection_from, opt.selection_to, true));
    opt.data.insert("animations", animation_array);

    // an already open editor has to show the new animation
    if (animation_dialog)
        animation_dialog->fill_list();
    this->on_actionAnimations_Editor_triggered();
}

void MainWindow::on_actionRotate_triggered()
{
    RotationDialog *dialog = new RotationDialog(&opt, this);
    connect(dialog, &RotationDialog::finished, this, [=]() { data_replaced(); });
    show_dialog(dialog);
}

void MainWindow::on_actionScale_Dialog_triggered()
{
    ScalingDialog *dialog = new ScalingDialog(&this->opt, this);
    connect(dialog, &ScalingDialog::finished, this, [=]() { data_replaced(); });
    show_dialog(dialog);
}

void MainWindow::on_actionPreferences_triggered()
{
    SettingsDialog *dialog = new SettingsDialog(&opt, this);
    connect(dialog, &SettingsDialog::finished, this, [=]() { this->ui->graphicsView->redraw(); });
    show_dialog(dialog);
}

// ── view ──────────────────────────────────────────────────────────────────────

void MainWindow::on_slider_scale_valueChanged(int value)
{
    qreal scale = qPow(qreal(2), (value - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);
    this->ui->graphicsView->setTransform(matrix);

    if (QSettings().value("sprites_per_row_auto", false).toBool())
        this->ui->graphicsView->redraw();
}

void MainWindow::on_actionZoomIn_triggered()
{
    this->ui->slider_scale->setValue(this->ui->slider_scale->value()+5);
}

void MainWindow::on_actionZoomOut_triggered()
{
    this->ui->slider_scale->setValue(this->ui->slider_scale->value()-5);
}

void MainWindow::on_actionQuick_Editor_triggered()
{
    QSettings settings;
    bool nowVisible = this->ui->actionQuick_Editor->isChecked();
    quickEditor->setVisible(nowVisible);
    this->ui->groupBox->setTitle(nowVisible ? tr("Quick Editor and Options") : tr("Options"));
    settings.setValue("quickeditor_visible", nowVisible);
}
