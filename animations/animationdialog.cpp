#include "animationdialog.h"
#include "ui_animationdialog.h"
#include "../sprite.h"

#include "animationform.h"

AnimationDialog::AnimationDialog(options *opt) : QWidget(),
    ui(new Ui::AnimationDialog)
{
    ui->setupUi(this);
    this->opt = opt;

    this->fill_list();
}

AnimationDialog::~AnimationDialog()
{
    delete ui;
}

void AnimationDialog::fill_list()
{
    QLayoutItem *item;
    while ( (item = this->ui->verticalLayout->takeAt(0)) != 0)
        delete item->widget();

    for (int i = 0; i < opt->data.value("animations").toArray().count(); i++)
    {
        AnimationForm *item = new AnimationForm(opt, i);
        connect(item, &AnimationForm::changed, [=](){
            this->save_data();
        });

        connect(item, &AnimationForm::rewrite_list, [=](){
            this->fill_list();
        });
        this->ui->verticalLayout->addWidget(item);
    }
}

void AnimationDialog::save_data()
{
    QJsonArray new_animations;

    for (int i = 0; i < this->ui->verticalLayout->count(); i++)
    {
        if (((AnimationForm*)this->ui->verticalLayout->itemAt(i)->widget())->delete_this)
            continue; //do not store this if marked for deletion

        QJsonObject o = ((AnimationForm*)this->ui->verticalLayout->itemAt(i)->widget())->get_animation();
        new_animations.append(o);
    }
    opt->data.insert("animations", new_animations);
}


void AnimationDialog::on_button_add_animation_clicked()
{
    AnimationForm *item = new AnimationForm(opt, opt->data.value("animations").toArray().count());
    this->ui->verticalLayout->addWidget(item);
    this->save_data();
}

