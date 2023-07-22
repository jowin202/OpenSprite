#include "animationform.h"
#include "ui_animationform.h"

#include "../sprite.h"

AnimationForm::AnimationForm(options *opt, int animation_id) :
    QWidget(),
    ui(new Ui::AnimationForm)
{
    ui->setupUi(this);

    this->opt = opt;
    this->current_id = animation_id;
    this->ui->spin_from->setMaximum(opt->data.value("sprites").toArray().count()-1);
    this->ui->spin_to->setMaximum(opt->data.value("sprites").toArray().count()-1);

    this->ui->spin_from->setValue(opt->data.value("animations").toArray().at(animation_id).toObject().value("from").toInt());
    this->ui->spin_to->setValue(opt->data.value("animations").toArray().at(animation_id).toObject().value("to").toInt());
    this->ui->spin_timer->setValue(opt->data.value("animations").toArray().at(animation_id).toObject().value("timer").toInt());

    this->ui->check_overlay->setChecked(opt->data.value("animations").toArray().at(animation_id).toObject().value("overlay").toBool());
    this->ui->check_pingpong->setChecked(opt->data.value("animations").toArray().at(animation_id).toObject().value("pingpong").toBool());


    this->timer.setInterval(opt->data.value("animations").toArray().at(animation_id).toObject().value("timer").toInt() * 20);
    connect(&this->timer, &QTimer::timeout, [=](){
        this->ui->label_img->setPixmap(QPixmap::fromImage(this->animation_images.at(current_pic)));
        current_pic = (current_pic+1) % (this->ui->check_pingpong->isChecked() ? this->animation_images.count() : 1+( this->animation_images.count()>>1));
    });

    this->reload_images();

    connect(this->ui->spin_from, &QSpinBox::valueChanged, [=](){ this->update_animation(); });
    connect(this->ui->spin_to, &QSpinBox::valueChanged, [=](){ this->update_animation(); });
    connect(this->ui->spin_timer, &QSpinBox::valueChanged, [=](){ this->update_animation(); });


    connect(this->ui->check_overlay, &QCheckBox::toggled, [=](){ this->update_animation(); });
    connect(this->ui->check_pingpong, &QCheckBox::toggled, [=](){ this->update_animation(); });
}

AnimationForm::~AnimationForm()
{
    delete ui;
}

void AnimationForm::update_animation()
{
    QJsonArray anim_array = this->opt->data.value("animations").toArray();
    QJsonObject current = anim_array.at(current_id).toObject();

    current.insert("from", this->ui->spin_from->value());
    current.insert("to", this->ui->spin_to->value());
    current.insert("valid", this->ui->spin_from->value() <= this->ui->spin_to->value());
    current.insert("timer", this->ui->spin_timer->value());

    current.insert("overlay", this->ui->check_overlay->isChecked());
    current.insert("pingpong", this->ui->check_pingpong->isChecked());

    timer.setInterval(this->ui->spin_timer->value()*20);

    anim_array.removeAt(current_id);
    anim_array.insert(current_id, current);
    this->opt->data.insert("animations", anim_array);
}

void AnimationForm::reload_images()
{
    this->animation_images.clear();
    for (int i = this->ui->spin_from->value(); i <= this->ui->spin_to->value(); i++)
    {
        this->animation_images.append(this->draw_sprite(i));
    }

    for (int i = this->ui->spin_to->value()-1; i >= this->ui->spin_from->value(); i--)
    {
        this->animation_images.append(this->draw_sprite(i));
    }
    this->ui->label_img->setPixmap(QPixmap::fromImage(this->animation_images.first()));
}

QImage AnimationForm::draw_sprite(int sprite_id)
{
    QPainter painter;
    QImage img(10*24, 10*21, QImage::Format_RGB16);
    painter.begin(&img);
    bool expand_x = this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("exp_x").toBool();
    bool expand_y = this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("exp_y").toBool();


    if (this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("mc_mode").toBool())
    {
        int w = 20 * (expand_y ? 0.5 : 1);
        int h = 10 * (expand_x ? 0.5 : 1);
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 12; x++)
            {
                if (this->get_sprite_bit(sprite_id,2*x,y)== 0 && this->get_sprite_bit(sprite_id,2*x+1,y) == 0)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(opt->data.value("background").toInt()));
                }
                else if (this->get_sprite_bit(sprite_id,2*x,y)== 1 && this->get_sprite_bit(sprite_id,2*x+1,y) == 0)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("sprite_color").toInt()));
                }
                else if (this->get_sprite_bit(sprite_id,2*x,y) == 0 && this->get_sprite_bit(sprite_id,2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc1").toInt()));
                }
                else if (this->get_sprite_bit(sprite_id,2*x,y) == 1 && this->get_sprite_bit(sprite_id,2*x+1,y) == 1)
                {
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc2").toInt()));
                }

            }
        }
    }
    else //not multicol
    {
        int w = 10 * (expand_y ? 0.5 : 1);
        int h = 10 * (expand_x ? 0.5 : 1);
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 24; x++)
            {
                if (this->get_sprite_bit(sprite_id,x,y) == 1)
                {
                    painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("sprite_color").toInt()));
                }
                else
                {
                    painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(opt->data.value("background").toInt()));
                }
            }
        }
    }


    if (this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("overlay_next").toBool() && opt->sprite_list.length() > sprite_id+1)
    {
        if (this->opt->data.value("sprites").toArray().at(sprite_id+1).toObject().value("mc_mode").toBool())
        {
            int w = 20 * (expand_y ? 0.5 : 1);
            int h = 10 * (expand_x ? 0.5 : 1);
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 12; x++)
                {
                    if (opt->sprite_list.at(sprite_id+1)->get_bit(2*x,y)== 1 && opt->sprite_list.at(sprite_id+1)->get_bit(2*x+1,y) == 0)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id+1).toObject().value("sprite_color").toInt()));
                    }
                    else if (opt->sprite_list.at(sprite_id+1)->get_bit(2*x,y) == 0 && opt->sprite_list.at(sprite_id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc1").toInt()));
                    }
                    else if (opt->sprite_list.at(sprite_id+1)->get_bit(2*x,y) == 1 && opt->sprite_list.at(sprite_id+1)->get_bit(2*x+1,y) == 1)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc2").toInt()));
                    }

                }
            }
        }
        else
        {
            int w = 10 * (expand_y ? 0.5 : 1);
            int h = 10 * (expand_x ? 0.5 : 1);
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 24; x++)
                {
                    if (opt->sprite_list.at(sprite_id+1)->get_bit(x,y) == 1)
                    {
                        painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(this->opt->data.value("sprites").toArray().at(sprite_id+1).toObject().value("sprite_color").toInt()));
                    }
                }
            }
        }
    }



    if (opt->show_grid_lines)
    {

        int w = 10 * (expand_y ? 0.5 : 1);
        int h = 10 * (expand_x ? 0.5 : 1);

        QPen pen;
        pen.setColor(Qt::gray);
        pen.setWidthF(0.5);
        painter.setPen(pen);
        for (int y = 0; y < 21; y++)
            painter.drawLine(0, h*y, 24*w, h*y);
        for (int x = 0; x < 24; x+=2)
            painter.drawLine(w*x, 0, w*x, h*21);

        if (!this->opt->data.value("sprites").toArray().at(sprite_id).toObject().value("mc_mode").toBool())
            for (int x = 1; x < 24; x+=2)
                painter.drawLine(w*x, 0, w*x, h*21);
    }
    return img;
}

int AnimationForm::get_sprite_bit(int sprite_id, int x, int y)
{
    if (x < 0 || y < 0 || x >= 24 || y >= 21) return false;
    if (opt->data.value("sprites").toArray().count() <= sprite_id) return false;
    return opt->data.value("sprites").toArray().at(sprite_id).toObject().value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}

void AnimationForm::on_button_delete_clicked()
{
    QJsonArray array = opt->data.value("animations").toArray();
    array.removeAt(current_id);
    opt->data.insert("animations", array);

    delete this;
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
        this->timer.start();
        this->ui->button_play->setText("Stop");
    }
}



