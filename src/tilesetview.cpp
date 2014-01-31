/*
    Tileset displayer/tile picker for the properties window
    (and eventually tile selection window with mouse tracking + 2x zoom)
*/
#include "tilesetview.h"
#include <QPainter>
#include <QPaintEvent>

TilesetView::TilesetView(QWidget *parent, const QPixmap *tiles, uint speed) :
    QWidget(parent),
    pixmap(tiles),
    timer(this)
{
    // update tileset once per frame (16ms)
    timer.start(16);

    QObject::connect(&timer, SIGNAL(timeout()),
                     this, SLOT(update()));
}

QSize TilesetView::sizeHint() const {
    return QSize(16 * 16, 16 * 16);
}

void TilesetView::paintEvent(QPaintEvent *event) {
    // assign a painter to the widget
    QPainter painter(this);
    QRect rect = event->rect();

    uint tile = 0;
    for (int h = rect.top() / 16; h <= rect.bottom() / 16; h++) {
        for (int w = rect.left() / 16; w <= rect.right() / 16; w++) {
            QRect destRect(w * 16, h * 16, 16, 16);
            QRect srcRect (tile * 16, 0, 16, 16);
            painter.drawPixmap(destRect, *pixmap, srcRect);

            if (++tile >= 256) return;
        }
    }

}
