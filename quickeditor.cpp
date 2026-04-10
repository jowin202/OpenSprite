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
    QPainter painter(this);
    if (!opt || opt->sprite_list.isEmpty()) return;

    int id = opt->selection_from;
    if (id < 0 || id >= opt->sprite_list.size()) return;

    const QJsonObject spriteObj =
        opt->data.value("sprites").toArray().at(id).toObject();
    const bool mc = spriteObj.value("mc_mode").toBool();

    const double W = width();
    const double H = height();

    // Cell dimensions — expand_x/expand_y always ignored in Quick Editor
    const double cw = mc ? W / 12.0 : W / 24.0;
    const double ch = H / 21.0;

    auto getCol = [&](int colIndex) -> QColor {
        return opt->col_list.at(colIndex);
    };

    Sprite *s = opt->sprite_list.at(id);

    // ── pixels ───────────────────────────────────────────────────────────────
    if (mc) {
        for (int y = 0; y < 21; y++) {
            for (int x = 0; x < 12; x++) {
                bool b0 = s->get_bit(2*x,   y);
                bool b1 = s->get_bit(2*x+1, y);
                QColor c;
                if      (!b0 && !b1) c = getCol(opt->data.value("background").toInt());
                else if ( b0 && !b1) c = getCol(spriteObj.value("sprite_color").toInt());
                else if (!b0 &&  b1) c = getCol(opt->data.value("mc1").toInt());
                else                 c = getCol(opt->data.value("mc2").toInt());
                painter.fillRect(QRectF(x*cw, y*ch, cw+1, ch+1), c);
            }
        }
    } else {
        for (int y = 0; y < 21; y++) {
            for (int x = 0; x < 24; x++) {
                QColor c = s->get_bit(x, y)
                           ? getCol(spriteObj.value("sprite_color").toInt())
                           : getCol(opt->data.value("background").toInt());
                painter.fillRect(QRectF(x*cw, y*ch, cw+1, ch+1), c);
            }
        }
    }

    // ── grid lines (mirrors Sprite::paint logic) ─────────────────────────────
    if (opt->show_grid_lines) {
        QPen pen;
        pen.setColor(Qt::gray);
        pen.setWidthF(0.5);
        painter.setPen(pen);

        // Horizontal lines — one per row
        for (int y = 0; y < 21; y++)
            painter.drawLine(QPointF(0, ch*y), QPointF(W, ch*y));

        // Vertical lines — every two logical columns (mc double-pixel boundary)
        for (int x = 0; x < 24; x += 2)
            painter.drawLine(QPointF(cw * (mc ? x/2 : x), 0),
                             QPointF(cw * (mc ? x/2 : x), H));

        // Single-colour mode also gets odd-column lines
        if (!mc) {
            for (int x = 1; x < 24; x += 2)
                painter.drawLine(QPointF(cw*x, 0), QPointF(cw*x, H));
        }
    }

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

QPointF QuickEditorWidget::toSpriteLocal(QPoint wp) const
{
    // Sprite's own coordinate system: 240 wide × 210 tall (w=10, h=10, no expand).
    return QPointF(wp.x() * 240.0 / width(),
                   wp.y() * 210.0 / height());
}

// ── mouse ─────────────────────────────────────────────────────────────────────

void QuickEditorWidget::mousePressEvent(QMouseEvent *ev)
{
    Sprite *s = activeSprite();
    if (!s) return;

    if (ev->button() == Qt::LeftButton)  s->set_left_pressed(true);
    if (ev->button() == Qt::RightButton) s->set_right_pressed(true);

    opt->undoDB.append(opt->data);

    QPointF sp = toSpriteLocal(ev->pos());
    if (opt->pen == PEN::DEFAULT)
        s->change_tile(sp);
    else
        s->flood_fill(sp);

    s->update();
    update();
    emit spriteEdited();
}

void QuickEditorWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (!(ev->buttons() & (Qt::LeftButton | Qt::RightButton))) return;
    Sprite *s = activeSprite();
    if (!s) return;

    if (opt->pen == PEN::DEFAULT) {
        QPointF sp = toSpriteLocal(ev->pos());
        if (QRectF(0, 0, width(), height()).contains(ev->pos()))
            s->change_tile(sp);
    }

    s->update();
    update();
    emit spriteEdited();
}

void QuickEditorWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    Sprite *s = activeSprite();
    if (!s) return;

    if (ev->button() == Qt::LeftButton)  s->set_left_pressed(false);
    if (ev->button() == Qt::RightButton) s->set_right_pressed(false);
}
