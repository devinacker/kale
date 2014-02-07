#ifndef TILEEDITWINDOW_H
#define TILEEDITWINDOW_H

#include <QDialog>
#include "tilesetview.h"
#include "level.h"

namespace Ui {
class TileEditWindow;
}

class TileEditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TileEditWindow(QWidget *parent, leveldata_t *level, QRect rect, const QPixmap *tiles);
    ~TileEditWindow();

public slots:
    void setTileLabel(int);
    void accept(int tile = -1);

private:
    Ui::TileEditWindow *ui;
    leveldata_t *level;
    QRect area;
    TilesetView *view;
};

#endif // TILEEDITWINDOW_H
