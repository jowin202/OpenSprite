#ifndef C64COLORPICKER_H
#define C64COLORPICKER_H

#include <QWidget>
#include <QFrame>
#include <QList>
#include <QColor>
#include <QStringList>

// ── C64ColorPicker ────────────────────────────────────────────────────────────
// Displays:  "N"  ████  Color Name
//   N        = colour index 0-15, right-aligned, slightly smaller font
//   ████     = colour patch, height = row height - 2*kGap, width = 1.3 * height
//   Color Name = left-aligned text
//
// Click anywhere on the widget opens a floating 4×4 colour grid popup.
// Row height is font-metrics-based so patches never touch between pickers.

class C64ColorPopup;

class C64ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit C64ColorPicker(QWidget *parent = nullptr);

    void setColors(const QList<QColor> &colors, const QStringList &names = {});
    int  currentIndex() const { return m_index; }
    void setCurrentIndex(int idx);
    void setEnabled(bool enabled);

signals:
    void currentIndexChanged(int index);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    QSize sizeHint() const override;

private:
    QList<QColor> m_colors;
    QStringList   m_names;
    int           m_index   = 0;
    bool          m_enabled = true;

    static constexpr int kGap      = 4;   // px gap above/below patch
    static constexpr int kNumWidth = 24;  // fixed px for number column
    static constexpr int kNameGap  = 6;   // gap between patch and name

    int patchH() const;
    int patchW() const;
    int patchX() const { return kNumWidth; }
    int nameX()  const { return patchX() + patchW() + kNameGap; }

    friend class C64ColorPopup;
};

// ── C64ColorPopup ─────────────────────────────────────────────────────────────
class C64ColorPopup : public QFrame
{
    Q_OBJECT
public:
    explicit C64ColorPopup(C64ColorPicker *owner);
    void popup();

signals:
    void colorChosen(int index);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    C64ColorPicker *m_owner;
    static constexpr int kCols = 4;
    static constexpr int kRows = 4;
    int cellW() const;
    int cellH() const;
};

#endif // C64COLORPICKER_H
