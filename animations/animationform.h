#ifndef ANIMATIONFORM_H
#define ANIMATIONFORM_H

#include <QWidget>
#include <QTimer>

struct options;

namespace Ui {
class AnimationForm;
}

class AnimationForm : public QWidget
{
    Q_OBJECT

public:
    explicit AnimationForm(options *opt, int animation_id);
    ~AnimationForm();

    QImage draw_sprite(int sprite_id);
    int get_sprite_bit(int sprite_id, int x, int y);

public slots:
    void update_animation();
    void reload_images();


private slots:
    void on_button_delete_clicked();

    void on_button_play_clicked();

private:
    Ui::AnimationForm *ui;
    options *opt;
    int current_id;
    int current_pic = 0;
    QTimer timer;

    QList<QImage> animation_images;
};

#endif // ANIMATIONFORM_H
