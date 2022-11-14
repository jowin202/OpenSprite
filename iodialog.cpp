#include "iodialog.h"
#include "ui_iodialog.h"

IODialog::IODialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IODialog)
{
    ui->setupUi(this);
}

IODialog::~IODialog()
{
    delete ui;
}
