#include "animationdialog.h"
#include "ui_animationdialog.h"
#include "../sprite.h"

#include "animationform.h"

AnimationDialog::AnimationDialog(options *opt, QWidget *parent) : QWidget(parent),
    ui(new Ui::AnimationDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    this->opt = opt;

    this->fill_list();
}

AnimationDialog::~AnimationDialog()
{
    delete ui;
}

AnimationForm *AnimationDialog::add_form(int animation_id)
{
    AnimationForm *item = new AnimationForm(opt, animation_id);
    connect(item, &AnimationForm::changed, this, &AnimationDialog::save_data);
    // queued: the list is rebuilt after the emitting form has returned (it gets deleted)
    connect(item, &AnimationForm::rewrite_list, this, &AnimationDialog::fill_list, Qt::QueuedConnection);
    this->ui->verticalLayout->addWidget(item);
    return item;
}

void AnimationDialog::fill_list()
{
    QLayoutItem *item;
    while ( (item = this->ui->verticalLayout->takeAt(0)) != nullptr)
    {
        if (QWidget *w = item->widget())
        {
            w->hide();
            w->deleteLater();
        }
        delete item;
    }

    for (int i = 0; i < opt->data.value("animations").toArray().count(); i++)
        add_form(i);
}

void AnimationDialog::save_data()
{
    QJsonArray new_animations;

    for (int i = 0; i < this->ui->verticalLayout->count(); i++)
    {
        AnimationForm *form = qobject_cast<AnimationForm*>(this->ui->verticalLayout->itemAt(i)->widget());
        if (form == nullptr || form->delete_this)
            continue; //do not store this if marked for deletion

        new_animations.append(form->get_animation());
    }
    opt->data.insert("animations", new_animations);
}


void AnimationDialog::on_button_add_animation_clicked()
{
    add_form(opt->data.value("animations").toArray().count());
    this->save_data();
}
