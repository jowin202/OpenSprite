#include "rotationdialog.h"
#include "ui_rotationdialog.h"

#include "sprite.h"
#include "seriesutil.h"

#include <QtMath>
#include <vector>

namespace {
constexpr int WIDTH = 48;
constexpr int BORDER_SIZE = 48;
constexpr int UPSCALE_FACTOR = 4; // Higher resolution for better rotation quality
constexpr int SIZE = (WIDTH + 2 * BORDER_SIZE) * UPSCALE_FACTOR;

// square buffer, addressed as [x][y]
template<typename T>
struct Grid
{
    std::vector<T> v = std::vector<T>(size_t(SIZE) * SIZE, T(0));
    T &operator()(int x, int y) { return v[size_t(x) * SIZE + y]; }
    T operator()(int x, int y) const { return v[size_t(x) * SIZE + y]; }
};

// rotation by three shears (x, y, x); same algorithm for both pixel types
template<typename T>
void shear_rotate(const Grid<T> &src, Grid<T> &dst, int angle)
{
    const qreal shear_factor_x = -qTan(angle / 180.0 * M_PI * 0.5);
    const qreal shear_factor_y = qSin(angle / 180.0 * M_PI);
    const int center_x = BORDER_SIZE * UPSCALE_FACTOR + WIDTH * UPSCALE_FACTOR / 2;
    const int center_y = center_x;

    Grid<T> temp1, temp2;

    // Shear X
    for (int i = 0; i < SIZE; i++) {
        const int offset = qRound(float(shear_factor_x * (i - center_y)));
        for (int j = 0; j < SIZE; j++) {
            const int newX = j + offset;
            if (newX >= 0 && newX < SIZE)
                temp1(newX, i) = src(j, i);
        }
    }
    // Shear Y
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            const int newY = i + qRound(float(shear_factor_y * (j - center_x)));
            if (newY >= 0 && newY < SIZE)
                temp2(j, newY) = temp1(j, i);
        }
    }
    // Shear X again
    for (int i = 0; i < SIZE; i++) {
        const int offset = qRound(float(shear_factor_x * (i - center_y)));
        for (int j = 0; j < SIZE; j++) {
            const int newX = j + offset;
            if (newX >= 0 && newX < SIZE)
                dst(newX, i) = temp2(j, i);
        }
    }
}
} // namespace

RotationDialog::RotationDialog(options *opt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RotationDialog)
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

    connect(this->ui->spin_angle, &QSpinBox::valueChanged, this, &RotationDialog::update_rotation);
    connect(this->ui->spin_steps, &QSpinBox::valueChanged, this, &RotationDialog::update_rotation);

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

    this->ui->preview->setPixmap(SeriesUtil::preview(opt, result_list, overlay_list));
}

QJsonObject RotationDialog::rotate_by(QJsonObject sprite, int angle) const
{
    if (sprite.value("mc_mode").toBool())
        return rotate_multicolor_sprite(sprite, angle);
    else
        return rotate_hires_sprite(sprite, angle);
}

QJsonObject RotationDialog::rotate_hires_sprite(QJsonObject sprite, int angle) const
{
    const int upscale = UPSCALE_FACTOR;
    const SpriteData::Bits bits = SpriteData::Bits::from_json(sprite);

    // Handle angle wrapping for 180-degree optimization
    bool flip_180 = false;
    if (angle > 90 && angle < 270)
    {
        flip_180 = true;
        angle -= 180;
    }

    // Upscale sprite data into high-res buffer
    Grid<float> source;
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 21; j++)
        {
            const float px_val = flip_180 ? bits.get(23-i, 20-j) : bits.get(i, j);
            for (int ux = 0; ux < 2 * upscale; ux++)
                for (int uy = 0; uy < 2 * upscale; uy++)
                    source(BORDER_SIZE * upscale + 2 * i * upscale + ux,
                           BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy) = px_val;
        }
    }

    Grid<float> rotated;
    shear_rotate(source, rotated, angle);

    // Downscale with simple averaging (no dithering for hires)
    SpriteData::Bits result;
    for (int y = 0; y < 21; y++)
    {
        for (int x = 0; x < 24; x++)
        {
            float sum = 0;
            for (int ux = 0; ux < 2 * upscale; ux++)
                for (int uy = 0; uy < 2 * upscale; uy++)
                    sum += rotated(BORDER_SIZE * upscale + 2 * x * upscale + ux,
                                   BORDER_SIZE * upscale + (2 * y + 3) * upscale + uy);
            const float avg = sum / (4.0 * upscale * upscale);
            result.set(x, y, avg >= 0.5);
        }
    }

    sprite.insert("sprite_data", result.to_json());
    return sprite;
}

QJsonObject RotationDialog::rotate_multicolor_sprite(QJsonObject sprite, int angle) const
{
    const int upscale = UPSCALE_FACTOR;
    const SpriteData::Bits bits = SpriteData::Bits::from_json(sprite);

    // Handle angle wrapping
    bool flip_180 = false;
    if (angle > 90 && angle < 270)
    {
        flip_180 = true;
        angle -= 180;
    }

    // Upscale sprite data - treat as fat pixels (2 bits = 1 color index)
    Grid<int> source;
    for (int i = 0; i < 12; i++)  // 12 fat pixels wide
    {
        for (int j = 0; j < 21; j++)
        {
            const int color_index = flip_180 ? bits.mc(11 - i, 20 - j) : bits.mc(i, j);
            for (int ux = 0; ux < 4 * upscale; ux++)  // Fat pixel is 2x wide
                for (int uy = 0; uy < 2 * upscale; uy++)
                    source(BORDER_SIZE * upscale + 4 * i * upscale + ux,
                           BORDER_SIZE * upscale + (2 * j + 3) * upscale + uy) = color_index;
        }
    }

    Grid<int> rotated;
    shear_rotate(source, rotated, angle);

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
                    const int color_idx = rotated(BORDER_SIZE * upscale + 4 * x * upscale + ux,
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

void RotationDialog::on_button_cancel_clicked()
{
    this->close();
}

void RotationDialog::on_button_ok_clicked()
{
    SeriesUtil::insert(opt, base_index, result_list, overlay_list);
    emit finished();
    this->close();
}
