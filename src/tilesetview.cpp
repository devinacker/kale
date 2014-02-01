/*
    Tileset displayer/tile picker for the properties window
    (and eventually tile selection window with mouse tracking + 2x zoom)
*/
#include "tilesetview.h"
#include "graphics.h"
#include <QPainter>
#include <QPaintEvent>

TilesetView::TilesetView(QWidget *parent, const QPixmap *tiles) :
    QWidget(parent),
    pixmap(tiles),
    timer(this)
{
    setFixedSize(this->sizeHint());

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

SpritesView::SpritesView(QWidget *parent):
    QWidget(parent),
    bankNum(0), palNum(0), colorNum(3)
{
    updateBank();
}

void SpritesView::updateBank() {
    this->bank[0] = getCHRBank(bankNum, palNum);
    this->bank[1] = getCHRBank(bankNum + 1, palNum);
    this->update();
}

/*
 * show two consecutive banks in one color
 * (not sure if this should actually use more space to show all four colors at once
 *  instead of using radio buttons)
 */
QSize SpritesView::sizeHint() const {
    return QSize(16 * 16, 16 * 8);
}

void SpritesView::setBank(int num) {
    this->bankNum = num & 0xFE;
    updateBank();
}

void SpritesView::setPalette(int num) {
    this->palNum = num;
    updateBank();
}

void SpritesView::setColor(int num) {
    this->colorNum = num;
    this->update();
}

/*
 * Draw a CHR bank of sprites arranging them into 16x16 blocks
 */
void SpritesView::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect rect = event->rect();

    for (int h = rect.top() / 32; h <= rect.bottom() / 32; h++) {
        for (int w = rect.left() / 16; w <= rect.right() / 16; w++) {
            // rows 0+2 show even numbered tiles, rows 1+3 show odd numbered tiles
            // so that they are arranged correctly into 16x16 sprites
            uint tile = ((h % 2) * 32) + (w * 2);

            QRect destRect(w * 16, h * 32, 16, 16);
            QRect srcRect (tile * 8, this->colorNum * 8, 8, 8);
            painter.drawImage(destRect, this->bank[h / 2], srcRect);
            painter.drawImage(destRect.translated(0, 16), this->bank[h / 2],
                              srcRect.translated(8, 0));
        }
    }
}
