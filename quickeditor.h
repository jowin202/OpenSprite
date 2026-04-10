#ifndef QUICKEDITOR_H
#define QUICKEDITOR_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include "sprite.h"

class QuickEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QuickEditorWidget(QWidget *parent = nullptr);
    void set_opt(options *opt) { this->opt = opt; }

    QSize sizeHint() const override;
    bool hasHeightForWidth() const override { return true; }
    int  heightForWidth(int w) const override { return w * 21 / 24; }

signals:
    void spriteEdited();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    options *opt = nullptr;

    // Map widget position → Sprite-local coordinate (240×210 space, no expand)
    QPointF toSpriteLocal(QPoint widgetPos) const;
    Sprite *activeSprite() const;
};

#endif // QUICKEDITOR_H
