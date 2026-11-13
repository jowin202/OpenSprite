#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QColorDialog>
#include "sprite.h"

namespace {
QString color_style(int rgb)
{
    return QString("background-color: #%1").arg(QString::number(rgb & 0xFFFFFF, 16), 6, QChar('0'));
}
int to_int(const QColor &c)
{
    return c.red() << 16 | c.green() << 8 | c.blue();
}
} // namespace

void SettingsDialog::apply_defaults(QSettings &settings, bool force)
{
    const QList<QPair<QString, QVariant>> defaults = {
        {"bgcolor", 0xd9d6c8},
        {"selection_color", 0x00ff00},
        {"sprite_spacing_x", 30},
        {"sprite_spacing_y", 30},
        {"sprites_per_row", 4},
        {"sprites_per_row_auto", false},
        {"sprite_number_size", 100},
    };
    for (const auto &d : defaults)
        if (force || !settings.contains(d.first))
            settings.setValue(d.first, d.second);
}

SettingsDialog::SettingsDialog(options *opt, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    this->opt = opt;

    this->ui->spin_horizontal_spacing->setValue(settings.value("sprite_spacing_x").toInt());
    this->ui->spin_vertical_spacing->setValue(settings.value("sprite_spacing_y").toInt());
    this->ui->spin_sprites_per_row->setValue(settings.value("sprites_per_row").toInt());
    this->ui->spin_sprite_number_size->setValue(settings.value("sprite_number_size", 100).toInt());

    bool auto_mode = settings.value("sprites_per_row_auto", false).toBool();
    this->ui->check_sprites_per_row_auto->setChecked(auto_mode);
    this->ui->spin_sprites_per_row->setEnabled(!auto_mode);

    this->ui->widget_selection_color->setStyleSheet(color_style(settings.value("selection_color").toInt()));
    this->ui->widget_spacing_color->setStyleSheet(color_style(settings.value("bgcolor").toInt()));
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
    settings.setValue("sprites_per_row_auto", this->ui->check_sprites_per_row_auto->isChecked());
    settings.setValue("sprite_number_size", this->ui->spin_sprite_number_size->value());

    if (select_color.isValid())
        settings.setValue("selection_color", to_int(select_color));

    if (bgcolor.isValid())
        settings.setValue("bgcolor", to_int(bgcolor));

    emit finished();
    this->close();
}


void SettingsDialog::on_button_spacing_color_clicked()
{
    QColor c = QColorDialog::getColor(QColor::fromRgb(QRgb(settings.value("bgcolor").toInt())), this, "Spacing Color");
    if (!c.isValid())
        return; // dialog cancelled
    bgcolor = c;
    this->ui->widget_spacing_color->setStyleSheet(color_style(to_int(bgcolor)));
}


void SettingsDialog::on_button_selection_color_clicked()
{
    QColor c = QColorDialog::getColor(QColor::fromRgb(QRgb(settings.value("selection_color").toInt())), this, "Selection Color");
    if (!c.isValid())
        return; // dialog cancelled
    select_color = c;
    this->ui->widget_selection_color->setStyleSheet(color_style(to_int(select_color)));
}


void SettingsDialog::on_check_sprites_per_row_auto_toggled(bool checked)
{
    this->ui->spin_sprites_per_row->setEnabled(!checked);
}

void SettingsDialog::on_button_defaults_clicked()
{
    apply_defaults(settings, true);
    emit finished();
    this->close();
}
