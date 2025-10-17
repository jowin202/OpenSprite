#include "scalingdialog.h"
#include "ui_scalingdialog.h"

#include "sprite.h"
#include "QtCore/QtMath"

#define WIDTH 48
#define HEIGHT 48
#define BORDER_SIZE 48
#define UPSCALE_FACTOR 8  // Higher resolution for better scaling quality

ScalingDialog::ScalingDialog(options *opt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScalingDialog)
{
    ui->setupUi(this);
    this->opt = opt;

    this->base_sprite = opt->data.value("sprites").toArray().at(opt->selection_from).toObject();
    if (this->base_sprite.value("overlay_next").toBool() && opt->data.value("sprites").toArray().count() > opt->selection_from+1)
    {
        this->overlay_sprite = opt->data.value("sprites").toArray().at(opt->selection_from+1).toObject();
    }

    connect(this->ui->spin_scale, SIGNAL(valueChanged(double)), this, SLOT(update_scaling()));
    connect(this->ui->spin_steps, SIGNAL(valueChanged(int)), this, SLOT(update_scaling()));

    this->update_scaling();
}

ScalingDialog::~ScalingDialog()
{
    delete ui;
}

void ScalingDialog::update_scaling()
{
    this->result_list.clear();
    this->overlay_list.clear();
    float scale_factor = this->ui->spin_scale->value();
    int steps = this->ui->spin_steps->value();

    // Calculate scale factor per step
    float scale_per_step = scale_factor;
    if (steps > 1)
    {
        // Exponential scaling from 1.0 to scale_factor
        scale_per_step = qPow(scale_factor, 1.0 / (steps - 1));
    }

    for (int i = 0; i < steps; i++)
    {
        float current_scale = qPow(scale_per_step, i);
        this->result_list.append(this->scale_by(this->base_sprite, current_scale));
        if (!this->overlay_sprite.isEmpty())
            this->overlay_list.append(this->scale_by(this->overlay_sprite, current_scale));
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

QJsonObject ScalingDialog::scale_by(QJsonObject sprite, float scale_factor)
{
    bool is_multicolor = sprite.value("mc_mode").toBool();
    
    if (is_multicolor)
    {
        return scale_multicolor_sprite(sprite, scale_factor);
    }
    else
    {
        return scale_hires_sprite(sprite, scale_factor);
    }
}

QJsonObject ScalingDialog::scale_hires_sprite(QJsonObject sprite, float scale_factor)
{
    const int upscale = UPSCALE_FACTOR;
    const int size = (WIDTH + 2 * BORDER_SIZE) * upscale;
    
    // Create high-resolution buffers
    float** source = new float*[size];
    float** scaled = new float*[size];
    
    for (int i = 0; i < size; i++)
    {
        source[i] = new float[size]();
        scaled[i] = new float[size]();
    }

    // Upscale sprite data into high-res buffer
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 21; j++)
        {
            int px_val = sprite.value("sprite_data").toArray().at(j).toArray().at(i).toInt();
            
            // Fill upscaled pixels
            for (int ux = 0; ux < 2 * upscale; ux++)
            {
                for (int uy = 0; uy < 2 * upscale; uy++)
                {
                    source[BORDER_SIZE * upscale + 2 * i * upscale + ux]
                          [BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy] = px_val;
                }
            }
        }
    }

    // Apply bilinear scaling
    int center_x = BORDER_SIZE * upscale + WIDTH * upscale / 2;
    int center_y = BORDER_SIZE * upscale + HEIGHT * upscale / 2;

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            // Calculate source position
            float src_x = center_x + (x - center_x) / scale_factor;
            float src_y = center_y + (y - center_y) / scale_factor;

            // Check bounds
            if (src_x < 0 || src_x >= size - 1 || src_y < 0 || src_y >= size - 1)
            {
                scaled[x][y] = 0;
                continue;
            }

            // Bilinear interpolation
            int x0 = (int)src_x;
            int y0 = (int)src_y;
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            float dx = src_x - x0;
            float dy = src_y - y0;

            float val00 = source[x0][y0];
            float val10 = source[x1][y0];
            float val01 = source[x0][y1];
            float val11 = source[x1][y1];

            float val0 = val00 * (1 - dx) + val10 * dx;
            float val1 = val01 * (1 - dx) + val11 * dx;
            float val = val0 * (1 - dy) + val1 * dy;

            scaled[x][y] = val;
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
                    sum += scaled[BORDER_SIZE * upscale + 2 * x * upscale + ux]
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
        delete[] source[i];
        delete[] scaled[i];
    }
    delete[] source;
    delete[] scaled;

    return sprite;
}

QJsonObject ScalingDialog::scale_multicolor_sprite(QJsonObject sprite, float scale_factor)
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
    int** color_source = new int*[size];
    float** color_scaled = new float*[size];  // Float for interpolation
    
    for (int i = 0; i < size; i++)
    {
        color_source[i] = new int[size]();
        color_scaled[i] = new float[size]();
    }

    // Upscale sprite data - treat as fat pixels (2 bits = 1 color)
    for (int i = 0; i < 12; i++)  // 12 fat pixels wide
    {
        for (int j = 0; j < 21; j++)
        {
            int bit0 = sprite.value("sprite_data").toArray().at(j).toArray().at(2*i).toInt();
            int bit1 = sprite.value("sprite_data").toArray().at(j).toArray().at(2*i + 1).toInt();
            
            int color_index = bit0 * 2 + bit1;
            
            // Fill upscaled pixels with color index
            for (int ux = 0; ux < 4 * upscale; ux++)  // Fat pixel is 2x wide
            {
                for (int uy = 0; uy < 2 * upscale; uy++)
                {
                    color_source[BORDER_SIZE * upscale + 4 * i * upscale + ux]
                               [BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy] = color_index;
                }
            }
        }
    }

    // Apply bilinear scaling for color indices
    int center_x = BORDER_SIZE * upscale + WIDTH * upscale / 2;
    int center_y = BORDER_SIZE * upscale + HEIGHT * upscale / 2;

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            // Calculate source position
            float src_x = center_x + (x - center_x) / scale_factor;
            float src_y = center_y + (y - center_y) / scale_factor;

            // Check bounds
            if (src_x < 0 || src_x >= size - 1 || src_y < 0 || src_y >= size - 1)
            {
                color_scaled[x][y] = 0;
                continue;
            }

            // Nearest neighbor for color indices (to preserve colors better)
            int nearest_x = (int)(src_x + 0.5);
            int nearest_y = (int)(src_y + 0.5);
            
            if (nearest_x >= 0 && nearest_x < size && nearest_y >= 0 && nearest_y < size)
            {
                color_scaled[x][y] = color_source[nearest_x][nearest_y];
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
                    int color_idx = (int)color_scaled[BORDER_SIZE * upscale + 4 * x * upscale + ux]
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
        delete[] color_source[i];
        delete[] color_scaled[i];
    }
    delete[] color_source;
    delete[] color_scaled;
    
    for (int i = 0; i < 12; i++)
        delete[] error_buffer[i];
    delete[] error_buffer;

    return sprite;
}

int ScalingDialog::get_sprite_bit(int sprite_id, int x, int y)
{
    if (x < 0 || y < 0 || x >= 24 || y >= 21) return false;
    if (this->result_list.count() <= sprite_id) return false;
    return this->result_list.at(sprite_id).value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}

int ScalingDialog::get_overlay_bit(int sprite_id, int x, int y)
{
    if (x < 0 || y < 0 || x >= 24 || y >= 21) return false;
    if (this->overlay_list.count() <= sprite_id) return false;
    return this->overlay_list.at(sprite_id).value("sprite_data").toArray().at(y).toArray().at(x).toInt() > 0;
}

QImage ScalingDialog::draw_sprite(int sprite_id)
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

void ScalingDialog::on_button_cancel_clicked()
{
    this->close();
}

void ScalingDialog::on_button_ok_clicked()
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