#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QColorDialog>
#include "sprite.h"

SettingsDialog::SettingsDialog(options *opt, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    this->opt = opt;

    this->ui->spin_horizontal_spacing->setValue(settings.value("sprite_spacing_x").toInt());
    this->ui->spin_vertical_spacing->setValue(settings.value("sprite_spacing_y").toInt());
    this->ui->spin_sprites_per_row->setValue(settings.value("sprites_per_row").toInt());

    this->ui->widget_selection_color->setStyleSheet(QString("background-color: #%1").arg(QString::number(settings.value("selection_color").toInt(),16),6,QChar('0')));
    this->ui->widget_spacing_color->setStyleSheet(QString("background-color: #%1").arg(QString::number(settings.value("bgcolor").toInt(),16),6,QChar('0')));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_button_cancel_clicked()
{
    this->close();
}


void SettingsDialog::on_button_ok_clicked()
{
    settings.setValue("sprite_spacing_x", this->ui->spin_horizontal_spacing->value());
    settings.setValue("sprite_spacing_y", this->ui->spin_vertical_spacing->value());
    settings.setValue("sprites_per_row", this->ui->spin_sprites_per_row->value());

    if (select_color.isValid())
        settings.setValue("selection_color", select_color.red() << 16 | select_color.green() << 8 | select_color.blue());

    if (bgcolor.isValid())
        settings.setValue("bgcolor", bgcolor.red() << 16 | bgcolor.green() << 8 | bgcolor.blue());

    emit finished();
    this->close();
}


void SettingsDialog::on_button_spacing_color_clicked()
{
    bgcolor = QColorDialog::getColor(settings.value("bgcolor").toInt(), 0, "Selection Color");
    this->ui->widget_spacing_color->setStyleSheet(QString("background-color: #%1").arg(QString::number(bgcolor.red() << 16 | bgcolor.green() << 8 | bgcolor.blue(),16),6,QChar('0')));

}


void SettingsDialog::on_button_selection_color_clicked()
{
    select_color = QColorDialog::getColor(settings.value("selection_color").toInt(), 0, "Selection Color");
    this->ui->widget_selection_color->setStyleSheet(QString("background-color: #%1").arg(QString::number(select_color.red() << 16 | select_color.green() << 8 | select_color.blue(),16),6,QChar('0')));
}


void SettingsDialog::on_button_defaults_clicked()
{
    settings.setValue("bgcolor", 0xd9d6c8);
    settings.setValue("selection_color", 0x00ff00);
    settings.setValue("sprite_spacing_x", 30);
    settings.setValue("sprite_spacing_y", 30);
    settings.setValue("sprites_per_row", 4);
    emit finished();
    this->close();
}

