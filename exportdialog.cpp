#include "exportdialog.h"
#include "ui_exportdialog.h"

#include "sprite.h"
#include "fileio.h"

#include <QDir>
#include <QFileInfo>

ExportDialog::ExportDialog(options *opt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    this->opt = opt;
    this->ui->address->setValue(opt->export_address);

    // restore the previous export choices
    if (!opt->last_exported_file.isEmpty())
    {
        if (opt->export_file_format == 1)
            this->ui->radio_bin->setChecked(true);
        else
            this->ui->radio_prg->setChecked(true);

        if (opt->export_attribute_format == 1)
            this->ui->radio_spritepad->setChecked(true);
        else if (opt->export_attribute_format == 2)
            this->ui->radio_seuck->setChecked(true);
        else
            this->ui->radio_empty->setChecked(true);
    }
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::on_button_export_clicked()
{
    // suggest a file name based on the project (without extension)
    QString base;
    const QString reference = !opt->last_exported_file.isEmpty() ? opt->last_exported_file : opt->last_saved_file;
    if (reference.isEmpty())
        base = QDir::homePath() + QDir::separator() + "Untitled";
    else
    {
        QFileInfo info(reference);
        base = info.absolutePath() + QDir::separator() + info.completeBaseName();
    }

    int attribute = 0; //0=empty
    if (this->ui->radio_seuck->isChecked())
        attribute = 2; //2=seuck
    else if (this->ui->radio_spritepad->isChecked())
        attribute = 1; //1=spritepad

    const bool prg = this->ui->radio_prg->isChecked();
    const bool bin = this->ui->radio_bin->isChecked();
    if (!prg && !bin)
        return;

    QString path = prg ? QFileDialog::getSaveFileName(this, "Export As", base + ".prg", "PRG File (*.prg)")
                       : QFileDialog::getSaveFileName(this, "Export As", base + ".bin", "BIN File (*.bin)");
    if (path.isEmpty())
        return; // keep the dialog open

    const int address = prg ? this->ui->address->value() : -1;
    if (!FileIO::write_prg(path, opt->data, address, attribute, this))
        return;

    emit statusmsg("Exported to: " + path);
    opt->last_exported_file = path;
    if (prg)
        opt->export_address = address;
    opt->export_file_format = prg ? 0 : 1;
    opt->export_attribute_format = attribute;

    this->on_button_close_clicked();
}


void ExportDialog::on_button_close_clicked()
{
    this->close();
}
