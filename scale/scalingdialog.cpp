#include "scalingdialog.h"
#include "ui_scalingdialog.h"

#include "sprite.h"
#include "seriesutil.h"

#include <QtMath>
#include <vector>

namespace {
constexpr int WIDTH = 48;
constexpr int HEIGHT = 48;
constexpr int BORDER_SIZE = 48;
constexpr int UPSCALE_FACTOR = 8; // Higher resolution for better scaling quality
constexpr int SIZE = (WIDTH + 2 * BORDER_SIZE) * UPSCALE_FACTOR;

// Only this area of the scaled buffer is sampled when downscaling to the
// 24x21 sprite, so nothing outside of it needs to be computed.
constexpr int SAMPLE_X0 = BORDER_SIZE * UPSCALE_FACTOR;
constexpr int SAMPLE_X1 = SAMPLE_X0 + 48 * UPSCALE_FACTOR;      // 24 px * 2
constexpr int SAMPLE_Y0 = BORDER_SIZE * UPSCALE_FACTOR + 3 * UPSCALE_FACTOR;
constexpr int SAMPLE_Y1 = SAMPLE_Y0 + 42 * UPSCALE_FACTOR;      // 21 px * 2

// square buffer, addressed as [x][y]
template<typename T>
struct Grid
{
    std::vector<T> v = std::vector<T>(size_t(SIZE) * SIZE, T(0));
    T &operator()(int x, int y) { return v[size_t(x) * SIZE + y]; }
    T operator()(int x, int y) const { return v[size_t(x) * SIZE + y]; }
};
} // namespace

ScalingDialog::ScalingDialog(options *opt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScalingDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    this->opt = opt;

    this->base_index = opt->selection_from;
    this->base_sprite = SpriteData::sprite_at(opt->data, base_index);
    if (this->base_sprite.value("overlay_next").toBool() && opt->sprite_count() > base_index+1)
    {
        this->overlay_sprite = SpriteData::sprite_at(opt->data, base_index+1);
    }

    connect(this->ui->spin_scale, &QDoubleSpinBox::valueChanged, this, &ScalingDialog::update_scaling);
    connect(this->ui->spin_steps, &QSpinBox::valueChanged, this, &ScalingDialog::update_scaling);

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

    this->ui->preview->setPixmap(SeriesUtil::preview(opt, result_list, overlay_list));
}

QJsonObject ScalingDialog::scale_by(QJsonObject sprite, float scale_factor) const
{
    if (sprite.value("mc_mode").toBool())
        return scale_multicolor_sprite(sprite, scale_factor);
    else
        return scale_hires_sprite(sprite, scale_factor);
}

QJsonObject ScalingDialog::scale_hires_sprite(QJsonObject sprite, float scale_factor) const
{
    const int upscale = UPSCALE_FACTOR;
    const int size = SIZE;
    const SpriteData::Bits bits = SpriteData::Bits::from_json(sprite);

    // Upscale sprite data into high-res buffer
    Grid<float> source;
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 21; j++)
        {
            const float px_val = bits.get(i, j);
            for (int ux = 0; ux < 2 * upscale; ux++)
                for (int uy = 0; uy < 2 * upscale; uy++)
                    source(BORDER_SIZE * upscale + 2 * i * upscale + ux,
                           BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy) = px_val;
        }
    }

    // Apply bilinear scaling (only in the area that is sampled afterwards)
    const int center_x = BORDER_SIZE * upscale + WIDTH * upscale / 2;
    const int center_y = BORDER_SIZE * upscale + HEIGHT * upscale / 2;

    Grid<float> scaled;
    for (int y = SAMPLE_Y0; y < SAMPLE_Y1; y++)
    {
        for (int x = SAMPLE_X0; x < SAMPLE_X1; x++)
        {
            // Calculate source position
            float src_x = center_x + (x - center_x) / scale_factor;
            float src_y = center_y + (y - center_y) / scale_factor;

            if (src_x < 0 || src_x >= size - 1 || src_y < 0 || src_y >= size - 1)
                continue; // stays 0

            // Bilinear interpolation
            int x0 = (int)src_x;
            int y0 = (int)src_y;
            float dx = src_x - x0;
            float dy = src_y - y0;

            float val0 = source(x0, y0) * (1 - dx) + source(x0 + 1, y0) * dx;
            float val1 = source(x0, y0 + 1) * (1 - dx) + source(x0 + 1, y0 + 1) * dx;
            scaled(x, y) = val0 * (1 - dy) + val1 * dy;
        }
    }

    // Downscale with simple averaging (no dithering for hires)
    SpriteData::Bits result;
    for (int y = 0; y < 21; y++)
    {
        for (int x = 0; x < 24; x++)
        {
            float sum = 0;
            for (int ux = 0; ux < 2 * upscale; ux++)
                for (int uy = 0; uy < 2 * upscale; uy++)
                    sum += scaled(BORDER_SIZE * upscale + 2 * x * upscale + ux,
                                  BORDER_SIZE * upscale + (2 * y + 3) * upscale + uy);
            const float avg = sum / (4.0 * upscale * upscale);
            result.set(x, y, avg >= 0.5);
        }
    }

    sprite.insert("sprite_data", result.to_json());
    return sprite;
}

QJsonObject ScalingDialog::scale_multicolor_sprite(QJsonObject sprite, float scale_factor) const
{
    const int upscale = UPSCALE_FACTOR;
    const int size = SIZE;
    const SpriteData::Bits bits = SpriteData::Bits::from_json(sprite);

    // Upscale sprite data - treat as fat pixels (2 bits = 1 color index)
    Grid<int> source;
    for (int i = 0; i < 12; i++)  // 12 fat pixels wide
    {
        for (int j = 0; j < 21; j++)
        {
            const int color_index = bits.mc(i, j);
            for (int ux = 0; ux < 4 * upscale; ux++)  // Fat pixel is 2x wide
                for (int uy = 0; uy < 2 * upscale; uy++)
                    source(BORDER_SIZE * upscale + 4 * i * upscale + ux,
                           BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy) = color_index;
        }
    }

    // Nearest neighbor scaling for color indices (to preserve colors)
    const int center_x = BORDER_SIZE * upscale + WIDTH * upscale / 2;
    const int center_y = BORDER_SIZE * upscale + HEIGHT * upscale / 2;

    Grid<int> scaled;
    for (int y = SAMPLE_Y0; y < SAMPLE_Y1; y++)
    {
        for (int x = SAMPLE_X0; x < SAMPLE_X1; x++)
        {
            float src_x = center_x + (x - center_x) / scale_factor;
            float src_y = center_y + (y - center_y) / scale_factor;

            if (src_x < 0 || src_x >= size - 1 || src_y < 0 || src_y >= size - 1)
                continue; // stays 0

            int nearest_x = (int)(src_x + 0.5);
            int nearest_y = (int)(src_y + 0.5);
            if (nearest_x >= 0 && nearest_x < size && nearest_y >= 0 && nearest_y < size)
                scaled(x, y) = source(nearest_x, nearest_y);
        }
    }

    // Downscale: use the most frequent color in each fat pixel
    SpriteData::Bits result;
    for (int y = 0; y < 21; y++)
    {
        for (int x = 0; x < 12; x++)
        {
            int color_count[4] = {0, 0, 0, 0};
            for (int ux = 0; ux < 4 * upscale; ux++)
            {
                for (int uy = 0; uy < 2 * upscale; uy++)
                {
                    const int color_idx = scaled(BORDER_SIZE * upscale + 4 * x * upscale + ux,
                                                 BORDER_SIZE * upscale + (2 * y + 3) * upscale + uy);
                    if (color_idx >= 0 && color_idx < 4)
                        color_count[color_idx]++;
                }
            }
            result.set_mc(x, y, SeriesUtil::most_frequent(color_count));
        }
    }

    sprite.insert("sprite_data", result.to_json());
    return sprite;
}

void ScalingDialog::on_button_cancel_clicked()
{
    this->close();
}

void ScalingDialog::on_button_ok_clicked()
{
    SeriesUtil::insert(opt, base_index, result_list, overlay_list);
    emit finished();
    this->close();
}
