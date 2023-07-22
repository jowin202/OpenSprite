#include "animationdialog.h"
#include "ui_animationdialog.h"
#include "../sprite.h"

AnimationDialog::AnimationDialog(options *opt) : QWidget(),
    ui(new Ui::AnimationDialog)
{
    ui->setupUi(this);
    this->opt = opt;
    this->ui->graphicsView->set_opt(opt);
    this->ui->graphicsView->redraw();

}

AnimationDialog::~AnimationDialog()
{
    delete ui;
}

void AnimationDialog::on_button_cancel_clicked()
{
    this->close();
}


void AnimationDialog::on_button_ok_clicked()
{
    //TODO: Save
    this->close();
}

