#include "quickeditor.h"

QuickEditorWidget::QuickEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

QSize QuickEditorWidget::sizeHint() const
{
    int w = parentWidget() ? parentWidget()->width() : 240;
    return QSize(w, heightForWidth(w));
}

void QuickEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Enforce 24:21 aspect ratio every time the width changes.
    // Using resizeEvent (not hasHeightForWidth alone) avoids the startup
    // squeeze bug where the layout commits height before width is finalised.
    int needed = width() * 21 / 24;
    if (needed != height())
        setFixedHeight(needed);
}

// ── paint ────────────────────────────────────────────────────────────────────

void QuickEditorWidget::paintEvent(QPaintEvent *)
{
    Sprite *s = activeSprite();
    if (!s) return;

    QPainter painter(this);
    const int id = s->get_id();
    const QJsonObject spriteObj = s->object();

    // expand_x/expand_y are always ignored in the Quick Editor
    const double W = width();
    const double H = height();

    QJsonObject overlay;
    const bool has_overlay = spriteObj.value("overlay_next").toBool() && id + 1 < opt->sprite_count();
    if (has_overlay)
        overlay = SpriteData::sprite_at(opt->data, id + 1);

    SpriteData::render(&painter,
                       opt->col_list,
                       opt->data,
                       spriteObj,
                       has_overlay ? &overlay : nullptr,
                       W / SpriteData::WIDTH,
                       H / SpriteData::HEIGHT,
                       opt->show_grid_lines);

    // ── border ───────────────────────────────────────────────────────────────
    painter.setPen(QPen(Qt::darkGray, 1));
    painter.drawRect(0, 0, (int)W-1, (int)H-1);
}

// ── helpers ───────────────────────────────────────────────────────────────────

Sprite *QuickEditorWidget::activeSprite() const
{
    if (!opt || opt->sprite_list.isEmpty()) return nullptr;
    int id = opt->selection_from;
    if (id < 0 || id >= opt->sprite_list.size()) return nullptr;
    return opt->sprite_list.at(id);
}

QPointF QuickEditorWidget::toSpriteLocal(Sprite *s, QPoint wp) const
{
    // Map to the sprite item's own coordinate system, which depends on the
    // expansion flags (240×210, 120×210, 240×105 or 120×105).
    const QRectF r = s->boundingRect();
    return QPointF(wp.x() * r.width() / width(),
                   wp.y() * r.height() / height());
}

// ── mouse ─────────────────────────────────────────────────────────────────────

void QuickEditorWidget::mousePressEvent(QMouseEvent *ev)
{
    Sprite *s = activeSprite();
    if (!s) return;

    if (ev->button() == Qt::LeftButton)       s->set_left_pressed(true);
    else if (ev->button() == Qt::RightButton) s->set_right_pressed(true);
    else return;

    opt->begin_edit();

    QPointF sp = toSpriteLocal(s, ev->pos());
    if (opt->pen == PEN::DEFAULT)
        s->change_tile(sp);
    else
        s->flood_fill(sp);

    update();
    emit spriteEdited();
}

void QuickEditorWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (!(ev->buttons() & (Qt::LeftButton | Qt::RightButton))) return;
    if (opt == nullptr || opt->pen != PEN::DEFAULT) return;
    Sprite *s = activeSprite();
    if (!s) return;

    if (rect().contains(ev->pos()))
        s->change_tile(toSpriteLocal(s, ev->pos()));

    update();
    emit spriteEdited();
}

void QuickEditorWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    Sprite *s = activeSprite();
    if (s) {
        if (ev->button() == Qt::LeftButton)  s->set_left_pressed(false);
        if (ev->button() == Qt::RightButton) s->set_right_pressed(false);
    }
    if (opt && !(ev->buttons() & (Qt::LeftButton | Qt::RightButton)))
        opt->end_edit();
}
