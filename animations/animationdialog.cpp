#include "animationdialog.h"
#include "ui_animationdialog.h"
#include "../sprite.h"

#include "animationform.h"

AnimationDialog::AnimationDialog(options *opt) : QWidget(),
    ui(new Ui::AnimationDialog)
{
    ui->setupUi(this);
    this->opt = opt;


    int i; //use it later
    for (i = 0; i < opt->data.value("animations").toArray().count(); i++)
    {
        AnimationForm *item = new AnimationForm(opt, i);
        this->ui->verticalLayout->addWidget(item);
    }


}

AnimationDialog::~AnimationDialog()
{
    delete ui;
}

