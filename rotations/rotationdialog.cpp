#include "rotationdialog.h"
#include "ui_rotationdialog.h"

#include "sprite.h"
#include "QtCore/QtMath"

#define WIDTH 48
#define HEIGHT 48
#define BORDER_SIZE 48

RotationDialog::RotationDialog(options *opt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RotationDialog)
{
    ui->setupUi(this);
    this->opt = opt;

    this->base_sprite = opt->data.value("sprites").toArray().at(opt->selection_from).toObject();

    connect(this->ui->spin_angle, SIGNAL(valueChanged(int)), this, SLOT(update_rotation()));
    connect(this->ui->spin_steps, SIGNAL(valueChanged(int)), this, SLOT(update_rotation()));

    this->update_rotation();
}

RotationDialog::~RotationDialog()
{
    delete ui;
}

void RotationDialog::update_rotation()
{
    this->result_list.clear();
    int angle = this->ui->spin_angle->value();
    int steps = this->ui->spin_steps->value();

    for (int i = 0; i < steps; i++)
    {
        this->result_list.append(this->rotate_by(this->base_sprite,i*angle));
    }


    QImage wallpaper(opt->sprite_spacing_x+(10*24+opt->sprite_spacing_x)*(opt->sprites_per_row),opt->sprite_spacing_y+(10*21+opt->sprite_spacing_y)*(steps/opt->sprites_per_row+1), QImage::Format_RGB16);
    wallpaper.fill(opt->background);
    QPainter painter;
    painter.begin(&wallpaper);
    for (int i = 0; i < this->result_list.count(); i++)
    {
        QImage sprite = this->draw_sprite(i);
        painter.drawImage(opt->sprite_spacing_x+(10*24+opt->sprite_spacing_x)*(i% opt->sprites_per_row),opt->sprite_spacing_y+(10*21+opt->sprite_spacing_y)*(i/opt->sprites_per_row), sprite);
    }
    painter.end();
    wallpaper = wallpaper.scaledToHeight(wallpaper.height()/2, Qt::SmoothTransformation);
    this->ui->preview->setPixmap(QPixmap::fromImage(wallpaper));
}

QJsonObject RotationDialog::rotate_by(QJsonObject sprite, int angle)
{
    int rotated[WIDTH + 2 * BORDER_SIZE][HEIGHT + 2 * BORDER_SIZE];
    float temp1[WIDTH + 2 * BORDER_SIZE][HEIGHT + 2 * BORDER_SIZE];
    float temp2[WIDTH + 2 * BORDER_SIZE][HEIGHT + 2 * BORDER_SIZE];
    float temp3[WIDTH + 2 * BORDER_SIZE][HEIGHT + 2 * BORDER_SIZE];

    for (int i = 0; i < 144; i++)
        for (int j = 0; j < 144; j++)
            temp1[i][j] = temp2[i][j] = temp3[i][j] = rotated[i][j] = 0;


   //
   if (angle <= 90 || angle >= 270)
   {
        for (int i = 0; i < 24; i++)
        {
            for (int j = 0; j < 21; j++)
            {
                int px_val = sprite.value("sprite_data").toArray().at(j).toArray().at(i).toInt();
                rotated[BORDER_SIZE + 2*i+0][BORDER_SIZE + 2*j+0+3] = px_val;
                rotated[BORDER_SIZE + 2*i+1][BORDER_SIZE + 2*j+0+3] = px_val;
                rotated[BORDER_SIZE + 2*i+0][BORDER_SIZE + 2*j+1+3] = px_val;
                rotated[BORDER_SIZE + 2*i+1][BORDER_SIZE + 2*j+1+3] = px_val;
            }
        }
   }
   else //angle near 180
   {
       angle -= 180;
       for (int i = 0; i < 24; i++)
       {
           for (int j = 0; j < 21; j++)
           {
               int px_val = sprite.value("sprite_data").toArray().at(20-j).toArray().at(23-i).toInt();
               rotated[BORDER_SIZE + 2*i+0][BORDER_SIZE + 2*j+0+3] = px_val;
               rotated[BORDER_SIZE + 2*i+1][BORDER_SIZE + 2*j+0+3] = px_val;
               rotated[BORDER_SIZE + 2*i+0][BORDER_SIZE + 2*j+1+3] = px_val;
               rotated[BORDER_SIZE + 2*i+1][BORDER_SIZE + 2*j+1+3] = px_val;
           }
       }
   }


    qreal shear_factor_x = -qTan(angle/180.0 * 3.14159265358979323846 * 0.5);
    qreal shear_factor_y = qSin(angle/180.0 * 3.14159265358979323846);


    int i, j;

    // Shear x
    for (i = 0; i < HEIGHT + 2 * BORDER_SIZE; i++) {
        for (j = 0; j < WIDTH + 2 * BORDER_SIZE; j++) {
            int newX = j + shear_factor_x * (i-BORDER_SIZE-WIDTH/2);
            if (newX < 0 || newX >= WIDTH + 2 * BORDER_SIZE) {
                // Handle out of bounds by skipping
                continue;
            }
            temp1[newX][i] = rotated[j][i];
        }
    }

    // Shear y
    for (i = 0; i < HEIGHT + 2 * BORDER_SIZE; i++) {
        for (j = 0; j < WIDTH + 2 * BORDER_SIZE; j++) {
            int newY = i + shear_factor_y *  (j-BORDER_SIZE-HEIGHT/2);
            if (newY < 0 || newY >= HEIGHT + 2 * BORDER_SIZE) {
                // Handle out of bounds by skipping
                continue;
            }
            temp2[j][newY] = temp1[j][i];
        }
    }

    // Shear x with -shear_factor_x
    for (i = 0; i < HEIGHT + 2 * BORDER_SIZE; i++) {
        for (j = 0; j < WIDTH + 2 * BORDER_SIZE; j++) {
            int newX = j + shear_factor_x * (i-BORDER_SIZE-WIDTH/2);
            if (newX < 0 || newX >= WIDTH + 2 * BORDER_SIZE) {
                // Handle out of bounds by skipping
                continue;
            }
            temp3[newX][i] = temp2[j][i];
        }
    }



    QJsonArray sprite_data = sprite.value("sprite_data").toArray();
    for (int y = 0; y < 21; y++)
    {
        QJsonArray row = sprite_data.at(y).toArray();
        for (int x = 0; x < 24; x++)
        {
            int sum =
                    temp3[BORDER_SIZE + 2*x+0][BORDER_SIZE + 2*y+0+3] +
                    temp3[BORDER_SIZE + 2*x+0][BORDER_SIZE + 2*y+1+3] +
                    temp3[BORDER_SIZE + 2*x+1][BORDER_SIZE + 2*y+0+3] +
                    temp3[BORDER_SIZE + 2*x+1][BORDER_SIZE + 2*y+1+3];

            row.removeAt(x);
            row.insert(x,(sum < 2 ? 0 : 1));
        }
        sprite_data.removeAt(y);
        sprite_data.insert(y, row);
    }
    sprite.insert("sprite_data", sprite_data);

    return sprite;
}


int RotationDialog::get_sprite_bit(int sprite_id, int x, int y)
{
    if (x < 0 || y < 0 || x >= 24 || y >= 21) return false;
    if (this->result_list.count() <= sprite_id) return false;
    return this->result_list.at(sprite_id).value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}


QImage RotationDialog::draw_sprite(int sprite_id)
{
    QPainter painter;
    QImage img(10*24, 10*21, QImage::Format_RGB16);
    painter.begin(&img);
    bool expand_x = false; //ignore
    bool expand_y = false; //ignore


    if (this->result_list.at(sprite_id).value("mc_mode").toBool())
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
                    painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->result_list.at(sprite_id).value("sprite_color").toInt()));
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
                    painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(this->result_list.at(sprite_id).value("sprite_color").toInt()));
                }
                else
                {
                    painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(opt->data.value("background").toInt()));
                }
            }
        }
    }


    /*
    if (this->result_list.at(sprite_id).value("overlay_next").toBool() && opt->sprite_list.length() > sprite_id+1)
    {
        if (this->result_list.at(sprite_id+1).value("mc_mode").toBool())
        {
            int w = 20 * (expand_y ? 0.5 : 1);
            int h = 10 * (expand_x ? 0.5 : 1);
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 12; x++)
                {
                    if (opt->sprite_list.at(sprite_id+1)->get_bit(2*x,y)== 1 && opt->sprite_list.at(sprite_id+1)->get_bit(2*x+1,y) == 0)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->result_list.at(sprite_id+1).value("sprite_color").toInt()));
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
    */



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

        if (!this->result_list.at(sprite_id).value("mc_mode").toBool())
            for (int x = 1; x < 24; x+=2)
                painter.drawLine(w*x, 0, w*x, h*21);
    }
    return img;
}





void RotationDialog::on_button_cancel_clicked()
{
    this->close();
}


void RotationDialog::on_button_ok_clicked()
{
    opt->undoDB.append(opt->data);

    QJsonArray sprites = opt->data.value("sprites").toArray();
    for (int i = 1; i < result_list.count(); i++)
    {
        sprites.insert(opt->selection_from+i, result_list.at(i));
    }

    opt->data.insert("sprites", sprites);

    emit finished();
    this->close();
}

