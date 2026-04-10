#include "quickeditor.h"

QuickEditorWidget::QuickEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(10);
}

QSize QuickEditorWidget::sizeHint() const
{
    int w = parentWidget() ? parentWidget()->width() : 240;
    return QSize(w, heightForWidth(w));
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

    const int W = width();
    const int H = height();

    // Cell dimensions in widget space — always treat expand_x/expand_y as off
    // Sprite pixel grid: 24 wide × 21 tall  (multicolor: 12 double-wide cols)
    const double cw = mc ? (double)W / 12.0 : (double)W / 24.0;
    const double ch = (double)H / 21.0;

    auto getCol = [&](int colIndex) -> QColor {
        return opt->col_list.at(colIndex);
    };

    Sprite *s = opt->sprite_list.at(id);

    if (mc) {
        for (int y = 0; y < 21; y++) {
            for (int x = 0; x < 12; x++) {
                QColor c;
                bool b0 = s->get_bit(2*x,   y);
                bool b1 = s->get_bit(2*x+1, y);
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

    // Subtle border so it reads as an editor area
    painter.setPen(QPen(Qt::darkGray, 1));
    painter.drawRect(0, 0, W-1, H-1);
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
    // change_tile / flood_fill both use those dimensions, so we just scale.
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

    // Only DEFAULT pen tracks drag (flood fill is one-shot on press)
    if (opt->pen == PEN::DEFAULT) {
        QPointF sp = toSpriteLocal(ev->pos());
        if (QRectF(0,0,width(),height()).contains(ev->pos()))
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
