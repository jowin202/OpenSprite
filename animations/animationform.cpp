#include "animationform.h"
#include "ui_animationform.h"

#include "../sprite.h"

AnimationForm::AnimationForm(options *opt, int animation_id) :
    QWidget(),
    ui(new Ui::AnimationForm)
{
    ui->setupUi(this);

    this->opt = opt;
    const int max_sprite = qMax(0, opt->sprite_count()-1);
    this->ui->spin_from->setMaximum(max_sprite);
    this->ui->spin_to->setMaximum(max_sprite);

    const QJsonObject animation = opt->data.value("animations").toArray().at(animation_id).toObject();
    this->ui->spin_from->setValue(animation.value("from").toInt());
    this->ui->spin_to->setValue(animation.value("to").toInt());

    int delay = animation.value("timer").toInt();
    this->ui->spin_timer->setValue( delay == 0 ? 16 : delay);

    this->ui->check_overlay->setChecked(animation.value("overlay").toBool());
    this->ui->check_pingpong->setChecked(animation.value("pingpong").toBool());


    this->timer.setInterval(this->ui->spin_timer->value() * 20);
    connect(&this->timer, &QTimer::timeout, this, [=](){
        if (this->animation_images.isEmpty())
            return;
        // forward frames are followed by the backward frames; without pingpong only the forward part is used
        const int frames = this->ui->check_pingpong->isChecked() ? this->animation_images.count() : 1+( this->animation_images.count()>>1);
        current_pic %= frames;
        this->ui->label_img->setPixmap(QPixmap::fromImage(this->animation_images.at(current_pic)));
        current_pic = (current_pic+1) % frames;
    });

    this->reload_images();

    connect(this->ui->spin_from, &QSpinBox::valueChanged, this, [=](){ this->update_animation(); });
    connect(this->ui->spin_to, &QSpinBox::valueChanged, this, [=](){ this->update_animation(); });
    connect(this->ui->spin_timer, &QSpinBox::valueChanged, this, [=](){ this->update_animation(); });


    connect(this->ui->check_overlay, &QCheckBox::toggled, this, [=](){ this->update_animation(); });
    connect(this->ui->check_pingpong, &QCheckBox::toggled, this, [=](){ this->update_animation(); });
}

AnimationForm::~AnimationForm()
{
    delete ui;
}

void AnimationForm::update_animation()
{
    timer.setInterval(this->ui->spin_timer->value()*20);
    emit changed();
}

void AnimationForm::reload_images()
{
    this->animation_images.clear();
    this->current_pic = 0;

    const QJsonArray sprites = this->opt->data.value("sprites").toArray();
    // sprites that are an overlay of the previous sprite are drawn together with it
    auto is_overlay = [&](int i) {
        return i > 0 && sprites.at(i-1).toObject().value("overlay_next").toBool();
    };

    for (int i = this->ui->spin_from->value(); i <= this->ui->spin_to->value(); i++)
    {
        if (is_overlay(i))
            continue;
        this->animation_images.append(this->draw_sprite(i));
    }

    for (int i = this->ui->spin_to->value()-1; i >= this->ui->spin_from->value(); i--)
    {
        if (is_overlay(i))
            continue;
        this->animation_images.append(this->draw_sprite(i));
    }

    if (this->animation_images.isEmpty())
        this->ui->label_img->clear();
    else
        this->ui->label_img->setPixmap(QPixmap::fromImage(this->animation_images.first()));
}

QImage AnimationForm::draw_sprite(int sprite_id)
{
    QImage img(10*24, 10*21, QImage::Format_RGB32);
    const QJsonObject sprite = SpriteData::sprite_at(opt->data, sprite_id);
    img.fill(SpriteData::color(opt->col_list, opt->data.value("background").toInt()));

    // expansion changes the aspect ratio inside the fixed 240x210 frame
    const qreal cw = sprite.value("exp_y").toBool() ? 5 : 10;
    const qreal ch = sprite.value("exp_x").toBool() ? 5 : 10;

    QJsonObject overlay;
    const bool has_overlay = sprite.value("overlay_next").toBool() && sprite_id + 1 < opt->sprite_count();
    if (has_overlay)
        overlay = SpriteData::sprite_at(opt->data, sprite_id + 1);

    QPainter painter(&img);
    SpriteData::render(&painter, opt->col_list, opt->data, sprite,
                       has_overlay ? &overlay : nullptr, cw, ch, opt->show_grid_lines);
    painter.end();
    return img;
}

QJsonObject AnimationForm::get_animation()
{
    QJsonObject obj;

    obj.insert("from", this->ui->spin_from->value());
    obj.insert("to", this->ui->spin_to->value());
    obj.insert("valid", this->ui->spin_from->value() <= this->ui->spin_to->value());
    obj.insert("timer", this->ui->spin_timer->value());

    obj.insert("overlay", this->ui->check_overlay->isChecked());
    obj.insert("pingpong", this->ui->check_pingpong->isChecked());
    return obj;
}

void AnimationForm::on_button_delete_clicked()
{
    this->timer.stop();
    this->delete_this = true;
    emit changed();
    emit rewrite_list();
}


void AnimationForm::on_button_play_clicked()
{
    if (this->timer.isActive())
    {
        this->timer.stop();
        this->ui->button_play->setText("Play");
    }
    else
    {
        this->reload_images();
        this->timer.start();
        this->ui->button_play->setText("Stop");
    }
}




void AnimationForm::on_spin_from_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    if (this->ui->spin_to->value() <= this->ui->spin_from->value())
        this->ui->spin_to->setValue(this->ui->spin_from->value()+1);
}


void AnimationForm::on_spin_to_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    if (this->ui->spin_to->value() <= this->ui->spin_from->value())
        this->ui->spin_from->setValue(this->ui->spin_to->value()-1);
}
