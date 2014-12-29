#ifndef SPRITEEDITWINDOW_H
#define SPRITEEDITWINDOW_H

#include <QDialog>
#include "level.h"

namespace Ui {
class SpriteEditWindow;
}

class SpriteEditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SpriteEditWindow(QWidget *parent, sprite_t *sprite);
    ~SpriteEditWindow();

protected:
    void accept();

private slots:
    void setSpriteInfo(int);
    void setIncludeAll(bool);

private:
    Ui::SpriteEditWindow *ui;
    sprite_t *sprite;
    bool includeAll;
};

#endif // SPRITEEDITWINDOW_H
