                                                                                      #include "rotationdialog.h"
#include "ui_rotationdialog.h"

#include "sprite.h"
#include "QtCore/QtMath"

#define WIDTH 48
#define HEIGHT 48
#define BORDER_SIZE 48
#define UPSCALE_FACTOR 4  // Higher resolution for better rotation quality

RotationDialog::RotationDialog(options *opt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RotationDialog)
{
    ui->setupUi(this);
    this->opt = opt;

    this->base_sprite = opt->data.value("sprites").toArray().at(opt->selection_from).toObject();
    if (this->base_sprite.value("overlay_next").toBool() && opt->data.value("sprites").toArray().count() > opt->selection_from+1)
    {
        this->overlay_sprite = opt->data.value("sprites").toArray().at(opt->selection_from+1).toObject();
    }

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
    this->overlay_list.clear();
    int angle = this->ui->spin_angle->value();
    int steps = this->ui->spin_steps->value();

    for (int i = 0; i < steps; i++)
    {
        this->result_list.append(this->rotate_by(this->base_sprite,i*angle));
        if (!this->overlay_sprite.isEmpty())
            this->overlay_list.append(this->rotate_by(this->overlay_sprite,i*angle));
    }

    QImage wallpaper(settings.value("sprite_spacing_x").toInt() + (10*24+settings.value("sprite_spacing_x").toInt())*(settings.value("sprites_per_row").toInt()),settings.value("sprite_spacing_y").toInt()+(10*21+settings.value("sprite_spacing_y").toInt())*(steps/settings.value("sprites_per_row").toInt()+1), QImage::Format_RGB16);
    wallpaper.fill(settings.value("bgcolor").toInt());
    QPainter painter;
    painter.begin(&wallpaper);
    for (int i = 0; i < this->result_list.count(); i++)
    {
        QImage sprite = this->draw_sprite(i);
        painter.drawImage(settings.value("sprite_spacing_x").toInt()+(10*24+settings.value("sprite_spacing_x").toInt())*(i% settings.value("sprites_per_row").toInt()),settings.value("sprite_spacing_y").toInt()+(10*21+settings.value("sprite_spacing_y").toInt())*(i/settings.value("sprites_per_row").toInt()), sprite);
    }
    painter.end();
    wallpaper = wallpaper.scaledToHeight(wallpaper.height()/2, Qt::SmoothTransformation);
    this->ui->preview->setPixmap(QPixmap::fromImage(wallpaper));
}

QJsonObject RotationDialog::rotate_by(QJsonObject sprite, int angle)
{
    bool is_multicolor = sprite.value("mc_mode").toBool();

    if (is_multicolor)
    {
        return rotate_multicolor_sprite(sprite, angle);
    }
    else
    {
        return rotate_hires_sprite(sprite, angle);
    }
}

QJsonObject RotationDialog::rotate_hires_sprite(QJsonObject sprite, int angle)
{
    const int upscale = UPSCALE_FACTOR;
    const int size = (WIDTH + 2 * BORDER_SIZE) * upscale;

    // Create high-resolution buffers
    float** rotated = new float*[size];
    float** temp1 = new float*[size];
    float** temp2 = new float*[size];
    float** temp3 = new float*[size];

    for (int i = 0; i < size; i++)
    {
        rotated[i] = new float[size]();
        temp1[i] = new float[size]();
        temp2[i] = new float[size]();
        temp3[i] = new float[size]();
    }

    // Handle angle wrapping for 180-degree optimization
    bool flip_180 = false;
    if (angle > 90 && angle < 270)
    {
        flip_180 = true;
        angle -= 180;
    }

    // Upscale sprite data into high-res buffer
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 21; j++)
        {
            int px_val = flip_180 ?
                             sprite.value("sprite_data").toArray().at(20-j).toArray().at(23-i).toInt() :
                             sprite.value("sprite_data").toArray().at(j).toArray().at(i).toInt();

            // Fill upscaled pixels
            for (int ux = 0; ux < 2 * upscale; ux++)
            {
                for (int uy = 0; uy < 2 * upscale; uy++)
                {
                    rotated[BORDER_SIZE * upscale + 2 * i * upscale + ux]
                           [BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy] = px_val;
                }
            }
        }
    }

    // Apply rotation using shear method
    qreal shear_factor_x = -qTan(angle / 180.0 * M_PI * 0.5);
    qreal shear_factor_y = qSin(angle / 180.0 * M_PI);

    int center_x = BORDER_SIZE * upscale + WIDTH * upscale / 2;
    int center_y = BORDER_SIZE * upscale + WIDTH * upscale / 2;

    // Shear X
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            float offset = shear_factor_x * (i - center_y);
            int newX = j + (int)qRound(offset);
            if (newX >= 0 && newX < size)
            {
                temp1[newX][i] = rotated[j][i];
            }
        }
    }

    // Shear Y
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            float offset = shear_factor_y * (j - center_x);
            int newY = i + (int)qRound(offset);
            if (newY >= 0 && newY < size)
            {
                temp2[j][newY] = temp1[j][i];
            }
        }
    }

    // Shear X again
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            float offset = shear_factor_x * (i - center_y);
            int newX = j + (int)qRound(offset);
            if (newX >= 0 && newX < size)
            {
                temp3[newX][i] = temp2[j][i];
            }
        }
    }

    // Downscale with simple averaging (no dithering for hires)
    QJsonArray sprite_data = sprite.value("sprite_data").toArray();

    for (int y = 0; y < 21; y++)
    {
        QJsonArray row = sprite_data.at(y).toArray();
        for (int x = 0; x < 24; x++)
        {
            // Average the upscaled pixels
            float sum = 0;
            for (int ux = 0; ux < 2 * upscale; ux++)
            {
                for (int uy = 0; uy < 2 * upscale; uy++)
                {
                    sum += temp3[BORDER_SIZE * upscale + 2 * x * upscale + ux]
                                [BORDER_SIZE * upscale + (2 * y + 3) * upscale + uy];
                }
            }
            float avg = sum / (4.0 * upscale * upscale);

            // Simple threshold without dithering
            int new_val = (avg >= 0.5) ? 1 : 0;

            row.removeAt(x);
            row.insert(x, new_val);
        }
        sprite_data.removeAt(y);
        sprite_data.insert(y, row);
    }

    sprite.insert("sprite_data", sprite_data);

    // Cleanup
    for (int i = 0; i < size; i++)
    {
        delete[] rotated[i];
        delete[] temp1[i];
        delete[] temp2[i];
        delete[] temp3[i];
    }
    delete[] rotated;
    delete[] temp1;
    delete[] temp2;
    delete[] temp3;

    return sprite;
}

QJsonObject RotationDialog::rotate_multicolor_sprite(QJsonObject sprite, int angle)
{
    const int upscale = UPSCALE_FACTOR;
    const int size = (WIDTH + 2 * BORDER_SIZE) * upscale;

    // Get color palette
    QVector<int> palette(4);
    palette[0] = opt->data.value("background").toInt();
    palette[1] = sprite.value("sprite_color").toInt();
    palette[2] = opt->data.value("mc1").toInt();
    palette[3] = opt->data.value("mc2").toInt();

    // Create high-resolution color buffer (stores palette index)
    int** color_buffer = new int*[size];
    int** temp1 = new int*[size];
    int** temp2 = new int*[size];
    int** temp3 = new int*[size];

    for (int i = 0; i < size; i++)
    {
        color_buffer[i] = new int[size]();
        temp1[i] = new int[size]();
        temp2[i] = new int[size]();
        temp3[i] = new int[size]();
    }

    // Handle angle wrapping
    bool flip_180 = false;
    if (angle > 90 && angle < 270)
    {
        flip_180 = true;
        angle -= 180;
    }

    // Upscale sprite data - treat as fat pixels (2 bits = 1 color)
    for (int i = 0; i < 12; i++)  // 12 fat pixels wide
    {
        for (int j = 0; j < 21; j++)
        {
            int bit0, bit1;
            if (flip_180)
            {
                bit0 = sprite.value("sprite_data").toArray().at(20-j).toArray().at(23 - 2*i - 1).toInt();
                bit1 = sprite.value("sprite_data").toArray().at(20-j).toArray().at(23 - 2*i).toInt();
            }
            else
            {
                bit0 = sprite.value("sprite_data").toArray().at(j).toArray().at(2*i).toInt();
                bit1 = sprite.value("sprite_data").toArray().at(j).toArray().at(2*i + 1).toInt();
            }

            int color_index = bit0 * 2 + bit1;

            // Fill upscaled pixels with color index
            for (int ux = 0; ux < 4 * upscale; ux++)  // Fat pixel is 2x wide
            {
                for (int uy = 0; uy < 2 * upscale; uy++)
                {
                    color_buffer[BORDER_SIZE * upscale + 4 * i * upscale + ux]
                                [BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy] = color_index;
                }
            }
        }
    }

    // Apply rotation using shear method
    qreal shear_factor_x = -qTan(angle / 180.0 * M_PI * 0.5);
    qreal shear_factor_y = qSin(angle / 180.0 * M_PI);

    int center_x = BORDER_SIZE * upscale + WIDTH * upscale / 2;
    int center_y = BORDER_SIZE * upscale + WIDTH * upscale / 2;

    // Shear X
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            float offset = shear_factor_x * (i - center_y);
            int newX = j + (int)qRound(offset);
            if (newX >= 0 && newX < size)
            {
                temp1[newX][i] = color_buffer[j][i];
            }
        }
    }

    // Shear Y
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            float offset = shear_factor_y * (j - center_x);
            int newY = i + (int)qRound(offset);
            if (newY >= 0 && newY < size)
            {
                temp2[j][newY] = temp1[j][i];
            }
        }
    }

    // Shear X again
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            float offset = shear_factor_x * (i - center_y);
            int newX = j + (int)qRound(offset);
            if (newX >= 0 && newX < size)
            {
                temp3[newX][i] = temp2[j][i];
            }
        }
    }

    // Downscale with color-aware dithering
    QJsonArray sprite_data = sprite.value("sprite_data").toArray();

    // Error diffusion buffer for each color component
    float** error_buffer = new float*[12];
    for (int i = 0; i < 12; i++)
    {
        error_buffer[i] = new float[21]();
    }

    for (int y = 0; y < 21; y++)
    {
        QJsonArray row = sprite_data.at(y).toArray();
        for (int x = 0; x < 12; x++)  // 12 fat pixels
        {
            // Count color occurrences in the downscale area
            int color_count[4] = {0, 0, 0, 0};
            for (int ux = 0; ux < 4 * upscale; ux++)
            {
                for (int uy = 0; uy < 2 * upscale; uy++)
                {
                    int color_idx = temp3[BORDER_SIZE * upscale + 4 * x * upscale + ux]
                                         [BORDER_SIZE * upscale + (2 * y + 3) * upscale + uy];
                    if (color_idx >= 0 && color_idx < 4)
                        color_count[color_idx]++;
                }
            }

            // Find most common color, adjusted by error
            int best_color = 0;
            int max_count = color_count[0];
            float adjusted_value = color_count[0] / (float)(8 * upscale * upscale) + error_buffer[x][y];

            for (int c = 1; c < 4; c++)
            {
                if (color_count[c] > max_count)
                {
                    max_count = color_count[c];
                    best_color = c;
                }
            }

            // Calculate error
            float actual_value = color_count[best_color] / (float)(8 * upscale * upscale);
            float error = adjusted_value - actual_value;

            // Distribute error
            if (x + 1 < 12)
                error_buffer[x + 1][y] += error * 7.0 / 16.0;
            if (y + 1 < 21)
            {
                if (x > 0)
                    error_buffer[x - 1][y + 1] += error * 3.0 / 16.0;
                error_buffer[x][y + 1] += error * 5.0 / 16.0;
                if (x + 1 < 12)
                    error_buffer[x + 1][y + 1] += error * 1.0 / 16.0;
            }

            // Convert color index back to bits
            int bit0 = (best_color >> 1) & 1;
            int bit1 = best_color & 1;

            row.removeAt(2 * x);
            row.insert(2 * x, bit0);
            row.removeAt(2 * x + 1);
            row.insert(2 * x + 1, bit1);
        }
        sprite_data.removeAt(y);
        sprite_data.insert(y, row);
    }

    sprite.insert("sprite_data", sprite_data);

    // Cleanup
    for (int i = 0; i < size; i++)
    {
        delete[] color_buffer[i];
        delete[] temp1[i];
        delete[] temp2[i];
        delete[] temp3[i];
    }
    delete[] color_buffer;
    delete[] temp1;
    delete[] temp2;
    delete[] temp3;

    for (int i = 0; i < 12; i++)
        delete[] error_buffer[i];
    delete[] error_buffer;

    return sprite;
}

int RotationDialog::get_sprite_bit(int sprite_id, int x, int y)
{
    if (x < 0 || y < 0 || x >= 24 || y >= 21) return false;
    if (this->result_list.count() <= sprite_id) return false;
    return this->result_list.at(sprite_id).value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}

int RotationDialog::get_overlay_bit(int sprite_id, int x, int y)
{
    if (x < 0 || y < 0 || x >= 24 || y >= 21) return false;
    if (this->overlay_list.count() <= sprite_id) return false;
    return this->overlay_list.at(sprite_id).value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}

QImage RotationDialog::draw_sprite(int sprite_id)
{
    QPainter painter;
    QImage img(10*24, 10*21, QImage::Format_RGB16);
    painter.begin(&img);
    bool expand_x = false;
    bool expand_y = false;

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
    else
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

    if (this->result_list.at(sprite_id).value("overlay_next").toBool() && !overlay_sprite.isEmpty())
    {
        if (this->overlay_sprite.value("mc_mode").toBool())
        {
            int w = 20 * (expand_y ? 0.5 : 1);
            int h = 10 * (expand_x ? 0.5 : 1);
            for (int y = 0; y < 21; y++)
            {
                for (int x = 0; x < 12; x++)
                {
                    if (this->get_overlay_bit(sprite_id,2*x,y)== 1 && this->get_overlay_bit(sprite_id,2*x+1,y) == 0)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->overlay_list.at(sprite_id).value("sprite_color").toInt()));
                    }
                    else if (this->get_overlay_bit(sprite_id,2*x,y) == 0 && this->get_overlay_bit(sprite_id,2*x+1,y) == 1)
                    {
                        painter.fillRect(x*w,y*h,w+1,h+1,this->opt->col_list.at(this->opt->data.value("mc1").toInt()));
                    }
                    else if (this->get_overlay_bit(sprite_id,2*x,y) == 1 && this->get_overlay_bit(sprite_id,2*x+1,y) == 1)
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
                    if (this->get_overlay_bit(sprite_id,x,y) == 1)
                    {
                        painter.fillRect(w*x,h*y,w+1,h+1,this->opt->col_list.at(this->overlay_list.at(sprite_id).value("sprite_color").toInt()));
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
    if (this->base_sprite.value("overlay_next").toBool())
    {
        for (int i = 1; i < result_list.count(); i++)
        {
            sprites.insert(opt->selection_from+2*i, result_list.at(i));
            sprites.insert(opt->selection_from+2*i+1, overlay_list.at(i));
        }
    }
    else
    {
        for (int i = 1; i < result_list.count(); i++)
        {
            sprites.insert(opt->selection_from+i, result_list.at(i));
        }
    }

    opt->data.insert("sprites", sprites);

    emit finished();
    this->close();
}
