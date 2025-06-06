#include "exportdialog.h"
#include "ui_exportdialog.h"

#include "mainwindow.h"
#include "fileio.h"

ExportDialog::ExportDialog(options *opt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    this->opt = opt;
    this->ui->address->setValue(opt->export_address);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::on_button_export_clicked()
{
    QString path;
    QString last_file = opt->last_saved_file == "" ? QDir::homePath() + QDir::separator() + "Untitled...." : opt->last_saved_file; //last 4 removed
    last_file.chop(4);


    int attribute = 0; //0=empty
    if (this->ui->radio_seuck->isChecked())
        attribute = 2; //2=seuck
    else if (this->ui->radio_spritepad->isChecked())
        attribute = 1; //1=spritepad


    if (this->ui->radio_prg->isChecked())
    {
        path = QFileDialog::getSaveFileName(this, "Export As", last_file + ".prg", "PRG File (*.prg)");
        if (path != "")
        {
            FileIO().write_prg(path, opt->data, this->ui->address->value(), attribute);
            emit statusmsg("Exported to: " + path);
        }
    }
    else if (this->ui->radio_bin->isChecked())
    {
        path = QFileDialog::getSaveFileName(this, "Export As", last_file + ".bin", "BIN File (*.bin)");
        if (path != "")
        {
            FileIO().write_prg(path, opt->data, -1, attribute);
            emit statusmsg("Exported to: " + path);
        }
    }

    if (path != "")
    {
        opt->last_exported_file = path;
        opt->export_address = (this->ui->radio_prg->isChecked() ?
                                 this->ui->address->value() : -1);
        opt->export_file_format = (this->ui->radio_prg->isChecked() ? 0 : 1);
        opt->export_attribute_format = attribute;
    }
    this->on_button_close_clicked();
}


void ExportDialog::on_button_close_clicked()
{
    this->close();
}

