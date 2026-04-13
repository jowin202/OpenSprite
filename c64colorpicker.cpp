#include "c64colorpicker.h"

#include <QPainter>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>

// ═══════════════════════════════════════════════════════════════════════════════
//  C64ColorPicker
// ═══════════════════════════════════════════════════════════════════════════════

C64ColorPicker::C64ColorPicker(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
}

void C64ColorPicker::setColors(const QList<QColor> &colors, const QStringList &names)
{
    m_colors = colors;
    m_names  = names;
    update();
}

void C64ColorPicker::setCurrentIndex(int idx)
{
    if (idx < 0 || idx >= m_colors.size()) return;
    if (idx == m_index) return;
    m_index = idx;
    update();
    emit currentIndexChanged(m_index);
}

void C64ColorPicker::setEnabled(bool enabled)
{
    m_enabled = enabled;
    QWidget::setEnabled(enabled);
    update();
}

// ── geometry ──────────────────────────────────────────────────────────────────

int C64ColorPicker::patchH() const
{
    // Patch height = font height — makes each picker row as compact as a text line.
    // patchW = 1.3 * patchH gives a slightly landscape rectangle like a C64 colour chip.
    QFontMetrics fm(font());
    return qRound(fm.height() * 1.155);
}

int C64ColorPicker::patchW() const { return qRound(patchH() * 1.3); }

QSize C64ColorPicker::sizeHint() const
{
    QFontMetrics fm(font());
    int h = patchH() + 2 * kGap;
    int w = nameX() + fm.horizontalAdvance("Multi-Color 2") + 4;
    return QSize(w, h);
}

void C64ColorPicker::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    // Lock height to our computed value so all pickers stay the same height
    // regardless of what the layout tries to assign.
    int needed = patchH() + 2 * kGap;
    if (height() != needed)
        setFixedHeight(needed);
}

// ── paint ─────────────────────────────────────────────────────────────────────

void C64ColorPicker::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    const int ph  = patchH();
    const int pw  = patchW();
    const int px  = patchX();
    const int nx  = nameX();
    const int top = kGap;   // y where patch starts

    // ── number (right-aligned, smaller font) ─────────────────────────────────
    QFont numFont = font();
    numFont.setPointSizeF(font().pointSizeF() * 0.80);
    p.setFont(numFont);
    p.setPen(m_enabled ? palette().color(QPalette::Text)
                       : palette().color(QPalette::Disabled, QPalette::Text));
    p.drawText(QRect(0, top, kNumWidth - 2, ph),
               Qt::AlignRight | Qt::AlignVCenter,
               QString::number(m_index));

    // ── colour patch ─────────────────────────────────────────────────────────
    QRect patch(px, top, pw, ph);
    if (!m_colors.isEmpty() && m_index < m_colors.size()) {
        QColor c = m_colors.at(m_index);
        if (!m_enabled) c = c.darker(150);
        p.fillRect(patch, c);
    }
    p.setPen(QPen(palette().color(QPalette::Mid), 1));
    p.drawRect(patch.adjusted(0, 0, -1, -1));

    // ── colour name (normal font, left-aligned) ───────────────────────────────
    p.setFont(font());
    p.setPen(m_enabled ? palette().color(QPalette::Text)
                       : palette().color(QPalette::Disabled, QPalette::Text));
    QString name = (!m_names.isEmpty() && m_index < m_names.size())
                   ? m_names.at(m_index) : QString();
    p.drawText(QRect(nx, top, width() - nx, ph),
               Qt::AlignLeft | Qt::AlignVCenter, name);

    // ── focus indicator ───────────────────────────────────────────────────────
    if (hasFocus() && m_enabled) {
        p.setPen(QPen(palette().color(QPalette::Highlight), 1, Qt::DotLine));
        p.drawRect(rect().adjusted(1, 1, -2, -2));
    }
}

void C64ColorPicker::mousePressEvent(QMouseEvent *ev)
{
    if (!m_enabled || m_colors.isEmpty()) return;
    if (ev->button() != Qt::LeftButton) return;

    // Only trigger on the colour patch itself
    QRect patch(patchX(), kGap, patchW(), patchH());
    if (!patch.contains(ev->pos())) return;

    auto *popup = new C64ColorPopup(this);
    connect(popup, &C64ColorPopup::colorChosen, this, [=](int idx) {
        setCurrentIndex(idx);
    });
    popup->popup();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  C64ColorPopup
// ═══════════════════════════════════════════════════════════════════════════════

C64ColorPopup::C64ColorPopup(C64ColorPicker *owner)
    : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint)
    , m_owner(owner)
{
    setFrameShape(QFrame::Box);
    setLineWidth(1);
    setFocusPolicy(Qt::StrongFocus);
}

int C64ColorPopup::cellW() const { return m_owner->patchW(); }
int C64ColorPopup::cellH() const { return m_owner->patchH(); }

void C64ColorPopup::popup()
{
    resize(kCols * cellW(), kRows * cellH());

    // Align popup so current colour's patch sits exactly over the picker's patch
    QPoint pickerGlobal = m_owner->mapToGlobal(QPoint(0, 0));
    int cur = m_owner->m_index;
    int col = cur % kCols;
    int row = cur / kCols;

    int x = pickerGlobal.x() + m_owner->patchX() - col * cellW();
    int y = pickerGlobal.y() + m_owner->kGap    - row * cellH();

    // Keep on the screen the picker is actually on
    QPoint pickerCenter = m_owner->mapToGlobal(m_owner->rect().center());
    QScreen *screen = QGuiApplication::screenAt(pickerCenter);
    if (!screen) screen = QGuiApplication::primaryScreen();
    QRect sg = screen->availableGeometry();

    x = qBound(sg.left(), x, sg.right()  - width());
    y = qBound(sg.top(),  y, sg.bottom() - height());

    move(x, y);
    show();
    setFocus();
}

void C64ColorPopup::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const int cw  = cellW();
    const int ch  = cellH();
    const int gap = m_owner->kGap;
    const int ph  = m_owner->patchH();

    for (int i = 0; i < m_owner->m_colors.size() && i < kCols * kRows; i++) {
        int col = i % kCols;
        int row = i / kCols;

        QRect cell(col * cw, row * ch, cw, ch);
        p.fillRect(cell, palette().color(QPalette::Window));

        QRect patch(col * cw, row * ch, cw, ph);
        p.fillRect(patch, m_owner->m_colors.at(i));

        if (i == m_owner->m_index) {
            p.setPen(QPen(Qt::white, 2));
            p.drawRect(patch.adjusted(2, 2, -2, -2));
            p.setPen(QPen(Qt::black, 1));
            p.drawRect(patch.adjusted(1, 1, -1, -1));
        } else {
            p.setPen(QPen(palette().color(QPalette::Mid), 1));
            p.drawRect(patch.adjusted(0, 0, -1, -1));
        }
    }
}

void C64ColorPopup::mousePressEvent(QMouseEvent *ev)
{
    if (!rect().contains(ev->pos())) {
        close();
        return;
    }
    const int cw = cellW();
    const int ch = cellH();
    int col = qBound(0, ev->pos().x() / cw, kCols - 1);
    int row = qBound(0, ev->pos().y() / ch, kRows - 1);
    int idx = row * kCols + col;
    if (idx >= 0 && idx < m_owner->m_colors.size())
        emit colorChosen(idx);
    close();
}

void C64ColorPopup::focusOutEvent(QFocusEvent *) { close(); }

void C64ColorPopup::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Escape) close();
    else QFrame::keyPressEvent(ev);
}
