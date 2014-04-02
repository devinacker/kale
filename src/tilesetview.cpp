/*
    Tileset displayer/tile picker for the properties window
    (and eventually tile selection window with mouse tracking + 2x zoom)
*/
#include "tilesetview.h"
#include "graphics.h"
#include <QPainter>
#include <QPaintEvent>

const QColor TilesetView::infoColor(255, 192, 192, 192);

TilesetView::TilesetView(QWidget *parent, const QPixmap *tiles) :
    QWidget(parent),
    pixmap(tiles),
    timer(this),
    currTile(-1)
{
    this->setFixedSize(sizeHint());
    this->setMouseEnabled(false);

    // update tileset once per frame (16ms)
    timer.start(16);

    QObject::connect(&timer, SIGNAL(timeout()),
                     this, SLOT(update()));
}

QSize TilesetView::sizeHint() const {
    return QSize(16 * TILE_SIZE, 16 * TILE_SIZE);
}

void TilesetView::paintEvent(QPaintEvent *event) {
    // assign a painter to the widget
    QPainter painter(this);
    QRect rect = event->rect();

    uint tile = 0;
    for (int h = rect.top() / TILE_SIZE; tile < 256 && h <= rect.bottom() / TILE_SIZE; h++) {
        for (int w = rect.left() / TILE_SIZE; tile < 256 && w <= rect.right() / TILE_SIZE; w++) {
            QRect destRect(w * TILE_SIZE, h * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            QRect srcRect (tile * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE);
            painter.drawPixmap(destRect, *pixmap, srcRect);

            tile++;
        }
    }

    if (currTile >= 0)
        painter.fillRect((currTile % 16) * TILE_SIZE, (currTile / 16) * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                         TilesetView::infoColor);
}

void TilesetView::setMouseEnabled(bool on) {
    this->mouseEnabled = on;
    this->setMouseTracking(on);
}

void TilesetView::mouseMoveEvent(QMouseEvent *event) {
    if (this->mouseEnabled) {
        QPoint pos = event->pos();
        int newTile;
        if (this->rect().contains(pos)) {
            newTile = (pos.y() / TILE_SIZE * 16) + (pos.x() / TILE_SIZE);
        } else {
            newTile = -1;
        }

        if (newTile != currTile) {
            currTile = newTile;
            emit tileHovered(currTile);
        }
    }
}

void TilesetView::mousePressEvent(QMouseEvent *event) {
    if (this->mouseEnabled && currTile >= 0) {
        event->accept();
        emit tileSelected(currTile);
    }
}

SpritesView::SpritesView(QWidget *parent):
    QWidget(parent),
    bankNum(0), palNum(0)
{
    this->setFixedSize(sizeHint());
    updateBank();
}

void SpritesView::updateBank() {
    this->bank[0] = getCHRSpriteBank(bankNum, palNum);
    this->bank[1] = getCHRSpriteBank(bankNum + 1, palNum);
    this->update();
}

/*
 * show two consecutive banks in one color
 * (not sure if this should actually use more space to show all four colors at once
 *  instead of using radio buttons)
 */
QSize SpritesView::sizeHint() const {
    return QSize(16 * 16, 16 * 16);
}

void SpritesView::setBank(int num) {
    this->bankNum = num & 0xFE;
    updateBank();
}

void SpritesView::setPalette(int num) {
    this->palNum = num;
    updateBank();
}

/*
 * Draw a CHR bank of sprites arranging them into 16x16 blocks
 */
void SpritesView::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect rect = event->rect();

    for (int h = rect.top() / (TILE_SIZE * 2); h <= rect.bottom() / (TILE_SIZE * 2) / 2; h++) {
        for (int w = rect.left() / (TILE_SIZE * 2); w <= rect.right() / (TILE_SIZE * 2); w++) {
            // get the first tile of each sprite
            uint tile = ((h % 2) * 32) + (w * 2);

            QRect destRect(w * TILE_SIZE * 2, h * TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
            QRect srcRect (tile * 8, 0, 8, 8);
            // first color
            painter.drawImage(destRect, this->bank[h / 2], srcRect);
            painter.drawImage(destRect.translated(0, 16), this->bank[h / 2],
                              srcRect.translated(8, 0));
            painter.drawImage(destRect.translated(16, 0), this->bank[h / 2],
                              srcRect.translated(8 * 16, 0));
            painter.drawImage(destRect.translated(16, 16), this->bank[h / 2],
                              srcRect.translated(8 * 17, 0));
            // second color
            painter.drawImage(destRect.translated(0, 16 * 8), this->bank[h / 2],
                              srcRect.translated(0, 8));
            painter.drawImage(destRect.translated(0, 16 * 9), this->bank[h / 2],
                              srcRect.translated(8, 8));
            painter.drawImage(destRect.translated(16, 16 * 8), this->bank[h / 2],
                              srcRect.translated(8 * 16, 8));
            painter.drawImage(destRect.translated(16, 16 * 9), this->bank[h / 2],
                              srcRect.translated(8 * 17, 8));
        }
    }
}
