#ifndef SPRITEVIEW_H
#define SPRITEVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QWheelEvent>
#include <QPainter>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>

#include <QMimeData>

struct options;

class SpriteView : public QGraphicsView
{
    Q_OBJECT
public:
    SpriteView(QWidget *parent);
    void set_opt(options *opt) { this->opt = opt; }
    void redraw();


    void change_current_sprite(int id) { emit current_sprite_changed(id); }

    void wheelEvent(QWheelEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override {
            event->acceptProposedAction();
    }
    void dragMoveEvent(QDragMoveEvent *event) override {
        event->acceptProposedAction();
    }
    void dragLeaveEvent(QDragLeaveEvent *event) override {
        event->accept();
    }
    void dropEvent(QDropEvent *event) override {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls() && mimeData->urls().length() == 1)
            emit droppedFile(mimeData->urls().at(0).toLocalFile());
    }

public slots:
    void add_new_sprite();

signals:
    void current_sprite_changed(int id);
    void zoom_in();
    void zoom_out();
    void droppedFile(QString file);

private:
    options *opt;
    QSettings settings;

};

#endif // SPRITEVIEW_H
