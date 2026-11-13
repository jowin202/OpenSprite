#include "sprite.h"
#include "spriteview.h"

using SpriteData::Bits;
using SpriteData::HEIGHT;
using SpriteData::MC_WIDTH;
using SpriteData::WIDTH;

Sprite::Sprite(options *opt, int id)
    : id(id)
    , opt(opt)
{}

qreal Sprite::cell_w(const QJsonObject &sprite) const
{
    return sprite.value("exp_y").toBool() ? 5 : 10;
}

qreal Sprite::cell_h(const QJsonObject &sprite) const
{
    return sprite.value("exp_x").toBool() ? 5 : 10;
}

void Sprite::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    const QJsonObject sprite = object();
    const qreal cw = cell_w(sprite);
    const qreal ch = cell_h(sprite);

    QJsonObject overlay;
    const bool has_overlay = sprite.value("overlay_next").toBool() && id + 1 < opt->sprite_count();
    if (has_overlay)
        overlay = SpriteData::sprite_at(opt->data, id + 1);

    SpriteData::render(painter,
                       opt->col_list,
                       opt->data,
                       sprite,
                       has_overlay ? &overlay : nullptr,
                       cw,
                       ch,
                       opt->show_grid_lines);

    if (opt->show_numbers) {
        const int pct = opt->sprite_number_size; // 1..100
        // Font size scales with percentage
        const int fontPx = qMax(6, qRound(qMin(0.8 * WIDTH * cw, 0.8 * HEIGHT * ch) * pct / 100.0));

        QFont font = painter->font();
        font.setPixelSize(fontPx);
        painter->setFont(font);

        if (pct >= 100) {
            // Full size: centered over the whole sprite
            painter->drawText(QRectF(0, 0, WIDTH * cw, HEIGHT * ch),
                              Qt::AlignVCenter | Qt::AlignHCenter,
                              QString::number(id));
        } else {
            // Smaller: top-left, indented half a pixel-cell from left.
            // Use explicit baseline to avoid Qt's internal descent offset.
            const QFontMetrics fm(painter->font());
            const int marginLeft = int(cw / 2);
            const int marginTop = 2;
            painter->drawText(marginLeft, marginTop + fm.ascent(), QString::number(id));
        }
    }

    if (id >= opt->selection_from && id <= opt->selection_to) {
        QPen pen(opt->selection_color);
        pen.setWidth(5);
        painter->setPen(pen);
        painter->setOpacity(1);
        painter->drawRect(this->boundingRect());
    }
}

QRectF Sprite::boundingRect() const
{
    const QJsonObject sprite = object();
    return QRectF(0, 0, WIDTH * cell_w(sprite), HEIGHT * cell_h(sprite));
}

int Sprite::active_button() const
{
    if (left_pressed)
        return opt->left_button;
    if (right_pressed)
        return opt->right_button;
    return -1;
}

void Sprite::update_neighbours()
{
    // the previous sprite may display this one as overlay
    if (id > 0 && id - 1 < opt->sprite_list.size())
        opt->sprite_list.at(id - 1)->update();
    this->update();
}

void Sprite::change_tile(QPointF pos)
{
    const QJsonObject sprite = object();
    const bool mc = sprite.value("mc_mode").toBool();
    const int x = int(pos.x() / cell_w(sprite));
    const int y = int(pos.y() / cell_h(sprite));
    const int cx = x / 2; // multicolor column

    auto set_mc = [&](int value) {
        set_bit(2 * cx, y, value & 2);
        set_bit(2 * cx + 1, y, value & 1);
    };

    switch (active_button()) {
    case TRANSPARENT:
        if (mc)
            set_mc(SpriteData::MC_BACKGROUND);
        else
            set_bit(x, y, false);
        break;
    case COLOR:
        if (mc)
            set_mc(SpriteData::MC_SPRITE);
        else
            set_bit(x, y, true);
        break;
    case MC1:
        if (mc)
            set_mc(SpriteData::MC_MC1);
        break;
    case MC2:
        if (mc)
            set_mc(SpriteData::MC_MC2);
        break;
    case OVERLAY_COLOR:
    case OVERLAY_TRANSPARENT:
        if (id + 1 < opt->sprite_list.size()) {
            Sprite *next = opt->sprite_list.at(id + 1);
            next->set_bit(x, y, active_button() == OVERLAY_COLOR);
            next->update();
        }
        break;
    default:
        return;
    }

    update_neighbours();
}

void Sprite::flood_fill(QPointF pos)
{
    const int button = active_button();
    if (button < 0)
        return;

    const QJsonObject sprite = object();
    const int hx = int(pos.x() / cell_w(sprite));
    const int y = int(pos.y() / cell_h(sprite));
    if (!Bits::inside(hx, y))
        return;

    // overlay buttons fill the next sprite (which is displayed on top of this one)
    const bool overlay = (button == OVERLAY_COLOR || button == OVERLAY_TRANSPARENT);
    const int target_id = overlay ? id + 1 : id;
    if (target_id >= opt->sprite_list.size())
        return;
    Sprite *target = opt->sprite_list.at(target_id);
    const bool mc = target->is_mc();

    int to;
    switch (button) {
    case TRANSPARENT:
    case OVERLAY_TRANSPARENT:
        to = SpriteData::MC_BACKGROUND;
        break;
    case MC1:
        to = mc ? SpriteData::MC_MC1 : 1;
        break;
    case MC2:
        to = mc ? SpriteData::MC_MC2 : 1;
        break;
    default: // COLOR, OVERLAY_COLOR
        to = mc ? SpriteData::MC_SPRITE : 1;
        break;
    }

    Bits b = target->bits();
    const int w = mc ? MC_WIDTH : WIDTH;
    auto value = [&](int px, int py) { return mc ? b.mc(px, py) : int(b.get(px, py)); };
    auto put = [&](int px, int py) {
        if (mc)
            b.set_mc(px, py, to);
        else
            b.set(px, py, to != 0);
    };

    const int start_x = mc ? hx / 2 : hx;
    const int from = value(start_x, y);
    if (from == to)
        return; // nothing to do

    // breadth first fill; cells are recolored when queued, so they are never queued twice
    QList<QPoint> queue;
    queue.reserve(w * HEIGHT);
    put(start_x, y);
    queue.append(QPoint(start_x, y));
    for (int i = 0; i < queue.size(); i++) {
        const QPoint p = queue.at(i);
        const QPoint neighbours[4] = {QPoint(p.x() - 1, p.y()),
                                      QPoint(p.x() + 1, p.y()),
                                      QPoint(p.x(), p.y() - 1),
                                      QPoint(p.x(), p.y() + 1)};
        for (const QPoint &n : neighbours) {
            if (n.x() < 0 || n.x() >= w || n.y() < 0 || n.y() >= HEIGHT)
                continue;
            if (value(n.x(), n.y()) == from) {
                put(n.x(), n.y());
                queue.append(n);
            }
        }
    }

    target->set_bits(b);
    target->update();
    update_neighbours();
}

void Sprite::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    const bool shift_pressed = ev->modifiers().testFlag(Qt::ShiftModifier);
    const bool inside_selection = id >= opt->selection_from && id <= opt->selection_to;
    QGraphicsScene *s = this->scene();

    //click outside of selection and shift not pressed
    if (!inside_selection && !shift_pressed) {
        //change selection to current
        opt->selection_from = id;
        opt->selection_to = id;
        if (s)
            s->update();
        opt->spriteview->change_current_sprite(id);
        return; //dont do anything else when switching sprite
    }
    //click outside of selection and shift pressed
    else if (!inside_selection && shift_pressed) {
        //extend selection
        opt->selection_from = qMin(opt->selection_from, id);
        opt->selection_to = qMax(opt->selection_to, id);
        if (s)
            s->update();
        return; //dont do anything else when switching sprite
    }
    //click inside of selection and not shift pressed AND MULTIPLE SELECTED
    else if (inside_selection && !shift_pressed && opt->selection_to != opt->selection_from) {
        opt->selection_from = id;
        opt->selection_to = id;
        if (s)
            s->update();
        opt->spriteview->change_current_sprite(id);
        return; //dont do anything else when switching sprite
    }

    if (ev->button() == Qt::LeftButton)
        this->left_pressed = true;
    else if (ev->button() == Qt::RightButton)
        this->right_pressed = true;
    else
        return;

    opt->begin_edit();
    if (opt->pen == PEN::FLOOD)
        this->flood_fill(ev->pos());
    else
        this->change_tile(ev->pos());
}

void Sprite::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    // flood fill only acts on the click, not while dragging
    if (opt->pen == PEN::DEFAULT && this->boundingRect().contains(ev->pos()))
        this->change_tile(ev->pos());
}

void Sprite::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        this->left_pressed = false;
    else if (ev->button() == Qt::RightButton)
        this->right_pressed = false;

    if (!left_pressed && !right_pressed)
        opt->end_edit();
}

bool Sprite::get_bit(int x, int y) const
{
    if (!Bits::inside(x, y) || id >= opt->sprite_count())
        return false;
    return SpriteData::sprite_at(opt->data, id)
               .value("sprite_data")
               .toArray()
               .at(y)
               .toArray()
               .at(x)
               .toInt()
           > 0;
}

void Sprite::set_bit(int x, int y, bool value)
{
    SpriteData::set_pixel(opt->data, id, x, y, value);
}

// --- whole sprite operations ------------------------------------------------

void Sprite::slide_up()
{
    const Bits b = bits();
    Bits r;
    for (int y = 0; y < HEIGHT; y++)
        r.px[y] = b.px[(y + 1) % HEIGHT];
    set_bits(r);
}

void Sprite::slide_down()
{
    const Bits b = bits();
    Bits r;
    for (int y = 0; y < HEIGHT; y++)
        r.px[y] = b.px[(y + HEIGHT - 1) % HEIGHT];
    set_bits(r);
}

void Sprite::slide_left()
{
    const Bits b = bits();
    Bits r;
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            r.px[y][x] = b.px[y][(x + 1) % WIDTH];
    set_bits(r);
}

void Sprite::slide_right()
{
    const Bits b = bits();
    Bits r;
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            r.px[y][x] = b.px[y][(x + WIDTH - 1) % WIDTH];
    set_bits(r);
}

void Sprite::flip_left()
{
    const Bits b = bits();
    Bits r;
    if (is_mc()) {
        for (int y = 0; y < HEIGHT; y++)
            for (int cx = 0; cx < MC_WIDTH; cx++)
                r.set_mc(cx, y, b.mc(MC_WIDTH - 1 - cx, y));
    } else {
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                r.px[y][x] = b.px[y][WIDTH - 1 - x];
    }
    set_bits(r);
}

void Sprite::flip_top()
{
    const Bits b = bits();
    Bits r;
    for (int y = 0; y < HEIGHT; y++)
        r.px[y] = b.px[HEIGHT - 1 - y];
    set_bits(r);
}

void Sprite::reflect_top()
{
    Bits b = bits();
    for (int y = HEIGHT / 2 + 1; y < HEIGHT; y++)
        b.px[y] = b.px[HEIGHT - 1 - y];
    set_bits(b);
}

void Sprite::reflect_left()
{
    Bits b = bits();
    if (is_mc()) {
        for (int y = 0; y < HEIGHT; y++)
            for (int cx = MC_WIDTH / 2; cx < MC_WIDTH; cx++)
                b.set_mc(cx, y, b.mc(MC_WIDTH - 1 - cx, y));
    } else {
        for (int y = 0; y < HEIGHT; y++)
            for (int x = WIDTH / 2; x < WIDTH; x++)
                b.px[y][x] = b.px[y][WIDTH - 1 - x];
    }
    set_bits(b);
}

void Sprite::swap_mc_colors(int a, int c)
{
    if (!is_mc())
        return;
    Bits b = bits();
    for (int y = 0; y < HEIGHT; y++) {
        for (int cx = 0; cx < MC_WIDTH; cx++) {
            const int v = b.mc(cx, y);
            if (v == a)
                b.set_mc(cx, y, c);
            else if (v == c)
                b.set_mc(cx, y, a);
        }
    }
    set_bits(b);
}

void Sprite::switch_col_to_mc1()
{
    swap_mc_colors(SpriteData::MC_SPRITE, SpriteData::MC_MC1);
}

void Sprite::switch_col_to_mc2()
{
    swap_mc_colors(SpriteData::MC_SPRITE, SpriteData::MC_MC2);
}

void Sprite::switch_mc1_to_mc2()
{
    swap_mc_colors(SpriteData::MC_MC1, SpriteData::MC_MC2);
}
