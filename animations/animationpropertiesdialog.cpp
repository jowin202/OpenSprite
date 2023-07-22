#include "animationpropertiesdialog.h"
#include "ui_animationpropertiesdialog.h"

#include "../sprite.h"

AnimationPropertiesDialog::AnimationPropertiesDialog(options *opt, int id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnimationPropertiesDialog)
{
    ui->setupUi(this);


    this->opt = opt;
    this->current_id = id;
    this->ui->spin_from->setMaximum(opt->data.value("sprites").toArray().count()-1);
    this->ui->spin_to->setMaximum(opt->data.value("sprites").toArray().count()-1);

    this->ui->spin_from->setValue(opt->data.value("animations").toArray().at(id).toObject().value("from").toInt());
    this->ui->spin_to->setValue(opt->data.value("animations").toArray().at(id).toObject().value("to").toInt());
    this->ui->spin_timer->setValue(opt->data.value("animations").toArray().at(id).toObject().value("timer").toInt());

    this->ui->check_overlay->setChecked(opt->data.value("animations").toArray().at(id).toObject().value("overlay").toBool());
    this->ui->check_pingpong->setChecked(opt->data.value("animations").toArray().at(id).toObject().value("pingpong").toBool());
}

AnimationPropertiesDialog::~AnimationPropertiesDialog()
{
    delete ui;
}

void AnimationPropertiesDialog::on_button_cancel_clicked()
{
    this->close();
}


void AnimationPropertiesDialog::on_button_ok_clicked()
{
    QJsonArray anim_array = this->opt->data.value("animations").toArray();
    QJsonObject current = anim_array.at(current_id).toObject();

    current.insert("from", this->ui->spin_from->value());
    current.insert("to", this->ui->spin_to->value());
    current.insert("valid", this->ui->spin_from->value() <= this->ui->spin_to->value());
    current.insert("timer", this->ui->spin_timer->value());

    current.insert("overlay", this->ui->check_overlay->isChecked());
    current.insert("pingpong", this->ui->check_pingpong->isChecked());

    anim_array.removeAt(current_id);
    anim_array.insert(current_id, current);
    this->opt->data.insert("animations", anim_array);

    emit finished();
    this->close();
}

