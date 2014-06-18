/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QFontMetrics>
#include <QGraphicsView>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <list>
#include "level.h"
#include "mainwindow.h"
#include "mapscene.h"
#include "mapchange.h"
#include "graphics.h"
#include "tileset.h"
#include "sceneitem.h"
#include "stuff.h"
#include "tileeditwindow.h"

#define MAP_TEXT_PAD_H 4
#define MAP_TEXT_PAD_V 0

const QFont MapScene::infoFont("Segoe UI", 10, QFont::Bold);
const QFontMetrics MapScene::infoFontMetrics(MapScene::infoFont);

const QColor MapScene::infoColor(255, 192, 192, 192);
const QColor MapScene::infoBackColor(255, 192, 192, 128);

const QColor MapScene::selectionColor(255, 192, 192, 192);
const QColor MapScene::selectionBorder(255, 192, 192, 255);

const QColor MapScene::layerColor(0, 192, 224, 192);

/*
  Overridden constructor which inits some scene info
 */
MapScene::MapScene(QObject *parent, leveldata_t *currentLevel)
    : QGraphicsScene(parent),

      tileX(-1), tileY(-1),
      selLength(0), selWidth(0), selecting(false),
      selectTiles(false), selectSprites(false), selectExits(false),
      sprites(), exits(),
      copyWidth(0), copyLength(0),
      stack(this),
      level(currentLevel),
      tilesetPixmap(256*TILE_SIZE, TILE_SIZE),
      animFrame(0), animTimer(this),
      showBounds(true), seeThrough(true),
      clearRects(NULL)
{
    /*
    QObject::connect(this, SIGNAL(edited()),
                     this, SLOT(refresh()));
    */
    QObject::connect(this, SIGNAL(edited()),
                     this, SLOT(update()));
    QObject::connect(&animTimer, SIGNAL(timeout()),
                     this, SLOT(animate()));
}

/*
  Edit the currently selected tiles (if any)
*/
void MapScene::editTiles() {
    if (selWidth == 0 || selLength == 0)
        return;

    MapChange *edit = new MapChange(level, selX, selY, selWidth, selLength);

    // send the level and selection info to a new tile edit window instance
    TileEditWindow win(NULL, level, QRect(selX, selY, selWidth, selLength), &tilesetPixmap);
    if (win.exec())
        stack.push(edit);
    else delete edit;

    // redraw the map scene with the new properties
    emit edited();

}

/*
  Redraw the scene
*/
void MapScene::refresh() {
    tileX = -1;
    tileY = -1;
    updateSelection();

    // reset the scene
    clear();
    this->sprites.clear();
    this->exits.clear();

    // if level is null , minimize the scene and return
    if (!level) {
        setSceneRect(0, 0, 0, 0);
        return;
    }

    uint width = level->header.screensH * SCREEN_WIDTH;
    uint height = level->header.screensV * SCREEN_HEIGHT;
    setSceneRect(0, 0, width * TILE_SIZE, height * TILE_SIZE);

    // no width/height = don't draw anything
    if (width * height == 0) {
        update();
        return;
    }

    setAnimSpeed(level->header.animSpeed);
    refreshPixmap();

    // add sprites
    for (std::list<sprite_t*>::iterator i = level->sprites.begin(); i != level->sprites.end(); i++) {
        SpriteItem *spr = new SpriteItem(*i);
        spr->setFlag(QGraphicsItem::ItemIsSelectable, selectSprites);
        spr->setFlag(QGraphicsItem::ItemIsMovable, selectSprites);
        addItem(spr);
        this->sprites.push_back(spr);
    }

    // add exits
    for (std::list<exit_t*>::iterator i = level->exits.begin(); i != level->exits.end(); i++) {
        ExitItem *exit = new ExitItem(*i);
        exit->setFlag(QGraphicsItem::ItemIsSelectable, selectExits);
        exit->setFlag(QGraphicsItem::ItemIsMovable, selectExits);
        addItem(exit);
        this->exits.push_back(exit);
    }

    update();
}

void MapScene::setAnimSpeed(int speed) {
    // set up tile animation
    // frame length (NTSC frames -> msec)
    uint timeout = speed * 16;
    if (timeout) {
        animTimer.start(timeout);
    } else {
        animFrame = 0;
        animTimer.stop();
        refreshPixmap();
    }
}

const QPixmap* MapScene::getPixmap() const {
    return &this->tilesetPixmap;
}

void MapScene::refreshPixmap() {
    // update CHR banks
    uint chr = level->header.tileIndex;
    uint pal = level->header.tilePal;
    QImage gfxBanks[4] = {
        getCHRBank(0, pal),
        getCHRBank(bankTable[0][chr], pal),
        getCHRBank(bankTable[1][chr], pal),
        getCHRBank(bankTable[2][chr] + animFrame, pal),
    };

    QPainter painter(&tilesetPixmap);
    uint tileset = level->tileset;
    for (uint i = 0; i < 256; i++) {
        metatile_t thisTile = tilesets[tileset][i];
        uint palette = thisTile.palette;

        uint srcX = i * 16;

        QRect destRect(0, 0, 8, 8);
        QRect srcRect(0, 8 * palette, 8, 8);

        // upper left
        destRect.moveTopLeft(QPoint(srcX, 0));
        srcRect.moveLeft(8 * (thisTile.ul % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.ul / 64], srcRect);

        // upper right
        destRect.moveTopLeft(QPoint(srcX + 8, 0));
        srcRect.moveLeft(8 * (thisTile.ur % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.ur / 64], srcRect);

        // lower left
        destRect.moveTopLeft(QPoint(srcX, 8));
        srcRect.moveLeft(8 * (thisTile.ll % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.ll / 64], srcRect);

        // lower right
        destRect.moveTopLeft(QPoint(srcX + 8, 8));
        srcRect.moveLeft(8 * (thisTile.lr % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.lr / 64], srcRect);
    }
}

// advance to next animation frame
void MapScene::animate() {
    ++animFrame &= 3;
    refreshPixmap();
    update();
}

/*
  Handle when the mouse is pressed on the scene
*/
void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // if the level is not being displayed, don't do anything
    // (or if the click is outside of the scene)
    if (!isActive() || !sceneRect().contains(event->scenePos())) return;

    // left button: start or continue selection
    // right button: cancel selection
    if (event->buttons() & Qt::LeftButton) {
        if (selectTiles) {
            beginSelection(event);
            event->accept();
        } else {
            // different selection type: try passing the event somewhere else
            QGraphicsScene::mousePressEvent(event);
        }

    } else if (event->buttons() & Qt::RightButton) {
        if (selectTiles) {
            cancelSelection();
            event->accept();
        } else if (selectSprites) {
            sprite_t* sprite = new sprite_t;
            memset(sprite, 0, sizeof(sprite_t));
            sprite->x = tileX; sprite->y = tileY;
            level->sprites.push_back(sprite);
            // TODO: simpler scene item refresh
            level->modified = true;
            level->modifiedRecently = true;
            emit edited();
            refresh();
            event->accept();
        } else if (selectExits) {
            exit_t* exit = new exit_t;
            memset(exit, 0, sizeof(exit_t));
            exit->x = tileX; exit->y = tileY;
            level->exits.push_back(exit);
            // TODO: simpler scene item refresh
            level->modified = true;
            level->modifiedRecently = true;
            emit edited();
            refresh();
            event->accept();
        }
    }
    update();
}

/*
  Handle when a double-click occurs (used to start the tile edit window)
*/
void MapScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (!isActive()) return;

    if (selectTiles) {
        editTiles();
        emit doubleClicked();

        event->accept();
    } else {
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
    update();
}

/*
  Handle when the left mouse button is released
*/
void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (selectTiles && event->button() == Qt::LeftButton) {
        selecting = false;

        // normalize selection dimensions (i.e. handle negative height/width)
        // so that selections made down and/or to the left are handled appropriately
        if (selWidth < 0) {
            selX += selWidth + 1;
            selWidth *= -1;
        }
        if (selLength < 0) {
            selY += selLength + 1;
            selLength *= -1;
        }

        event->accept();
    } else if (!selectTiles) {
        QGraphicsScene::mouseReleaseEvent(event);
    }
    update();
}

/*
  Handle when the mouse is moved over the scene
 */
void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // if inactive, don't handle mouse moves
    if (!isActive()) return;

    // behave differently based on left mouse button status
    if (selecting && event->buttons() & Qt::LeftButton) {
        // left button down: generate/show selection
        updateSelection(event);
    } else {
        showTileInfo(event);
    }

    if (selectTiles)
        event->accept();
    else
        QGraphicsScene::mouseMoveEvent(event);

    update();
}

/*
 *Undo/redo functions
 */
bool MapScene::canUndo() const {
    return stack.canUndo();
}

bool MapScene::canRedo() const {
    return stack.canRedo();
}

bool MapScene::isClean() const {
    return stack.isClean();
}

void MapScene::pushChange(QUndoCommand *change) {
    stack.push(change);
    emit edited();
}

void MapScene::undo() {
    if (stack.canUndo()) {
        emit statusMessage(QString("Undoing ").append(stack.undoText()));
        stack.undo();
        emit edited();

        level->modified = true;
        level->modifiedRecently = !isClean();
    }
}

void MapScene::redo() {
    if (stack.canRedo()) {
        emit statusMessage(QString("Redoing ").append(stack.redoText()));
        stack.redo();
        emit edited();

        level->modified = true;
        level->modifiedRecently = !isClean();
    }
}

void MapScene::setClean() {
    stack.setClean();
}

void MapScene::clearStack() {
    stack.clear();
}

/*
  Cut/copy/paste functions
*/
void MapScene::cut() {
    copyTiles(true);
}

void MapScene::copy() {
    copyTiles(false);
}

void MapScene::copyTiles(bool cut = false) {
    // if there is no selection, don't do anything
    if (selWidth == 0 || selLength == 0) return;

    MapChange *edit;
    if (cut) {
        edit = new MapChange(level, selX, selY, selWidth, selLength);
        edit->setText("cut");
    }

    // otherwise, move stuff into the buffer
    for (int i = 0; i < selLength; i++) {
        for (int j = 0; j < selWidth; j++) {
            copyBuffer[i][j] = level->tiles[selY + i][selX + j];
            if (cut)
                level->tiles[selY + i][selX + j] = 0;
        }
    }

    copyWidth = selWidth;
    copyLength = selLength;

    if (cut) {
        pushChange(edit);
    }

    emit statusMessage(QString("%1 (%2, %3) to (%4, %5)")
                       .arg(cut ? "Cut" : "Copied")
                       .arg(selX).arg(selY)
                       .arg(selX + selWidth - 1)
                       .arg(selY + selLength - 1));
}

void MapScene::paste() {
    // if there is no selection or copy buffer, don't do anything
    if (selWidth == 0 || selLength == 0
            || copyWidth == 0 || copyLength == 0) return;

    MapChange *edit = new MapChange(level, selX, selY, copyWidth, copyLength);
    edit->setText("paste");

    // otherwise, move stuff into the level from the buffer
    for (uint i = 0; i < copyLength && selY + i < 16*SCREEN_HEIGHT; i++) {
        for (uint j = 0; j < copyWidth && selX + j < 16*SCREEN_WIDTH; j++) {
            level->tiles[selY + i][selX + j] = copyBuffer[i][j];
        }
    }

    pushChange(edit);

    emit statusMessage(QString("Pasted (%1, %2) to (%3, %4)")
                       .arg(selX).arg(selY)
                       .arg(selX + copyWidth - 1)
                       .arg(selY + copyLength - 1));

}

void MapScene::deleteStuff() {
    if (selectTiles)
        deleteTiles();
    else
        deleteItems();
}

void MapScene::deleteTiles() {
    // if there is no selection, don't do anything
    if (selWidth == 0 || selLength == 0) return;

    MapChange *edit = new MapChange(level, selX, selY, selWidth, selLength);
    edit->setText("delete");

    // otherwise, delete stuff
    for (int i = 0; i < selLength; i++) {
        for (int j = 0; j < selWidth; j++) {
            level->tiles[selY + i][selX + j] = 0;
        }
    }

    pushChange(edit);

    emit statusMessage(QString("Deleted (%1, %2) to (%3, %4)")
                       .arg(selX).arg(selY)
                       .arg(selX + selWidth - 1)
                       .arg(selY + selLength - 1));

}

/*
 *remove scene items
 */
void MapScene::deleteItems() {
    QList<QGraphicsItem*> items = this->selectedItems();

    // iterate and delete
    // TODO: generate an undo/redo action
    for (QList<QGraphicsItem*>::iterator i = items.begin(); i != items.end(); i++) {
        this->removeItem(*i);

        if (selectSprites) {
            SpriteItem *item = dynamic_cast<SpriteItem*>(*i);

            this->sprites.remove(item);
            this->level->sprites.remove(item->sprite);

            delete item->sprite;
            delete item;

        } else if (selectExits) {
            ExitItem *item = dynamic_cast<ExitItem*>(*i);

            this->exits.remove(item);
            this->level->exits.remove(item->exit);

            delete item->exit;
            delete item;
        }

        level->modified = true;
        level->modifiedRecently = true;
    }
    emit edited();
}

/*
  Start a new selection on the map scene.
  Called when the mouse is clicked outside of any current selection.
*/
void MapScene::beginSelection(QGraphicsSceneMouseEvent *event) {

    QPointF pos = event->scenePos();

    int x = pos.x() / TILE_SIZE;
    int y = pos.y() / TILE_SIZE;

    // ignore invalid click positions
    // (use the floating point X coord to avoid roundoff stupidness)
    if (x >= level->header.screensH * SCREEN_WIDTH
            || y >= level->header.screensV * SCREEN_HEIGHT
            || pos.x() < 0 || y < 0)
        return;

    // is the click position outside of the current selection?
    if (x < selX || x >= selX + selWidth || y < selY || y >= selY + selLength) {
        selecting = true;
        selX = x;
        selY = y;
        selWidth = 1;
        selLength = 1;
        updateSelection(event);
    }

}

/*
  Update the selected range of map tiles.
  Called when the mouse is over the MapScene with the left button held down.
*/
void MapScene::updateSelection(QGraphicsSceneMouseEvent *event) {

    int x = selX;
    int y = selY;

    // if event is not null, this was triggered by a mouse action
    // and so the selection area should be updated
    if (event) {
        QPointF pos = event->scenePos();

        x = pos.x() / TILE_SIZE;
        y = pos.y() / TILE_SIZE;

        // ignore invalid mouseover/click positions
        // (use the floating point X coord to avoid roundoff stupidness)
        if (x >= level->header.screensH * SCREEN_WIDTH
                || y >= level->header.screensV * SCREEN_HEIGHT
                || pos.x() < 0 || y < 0)
            return;

        // update the selection size
        if (x >= selX)
            selWidth = x - selX + 1;
        else
            selWidth = x - selX - 1;
        if (y >= selY)
            selLength = y - selY + 1;
        else
            selLength = y - selY - 1;
    }

    if (selWidth == 0 || selLength == 0) return;

    int top = std::min(y, selY);
    int left = std::min(x, selX);

    if (event)
        emit statusMessage(QString("Selected (%1, %2) to (%3, %4)")
                           .arg(left).arg(top)
                           .arg(left + abs(selWidth) - 1)
                           .arg(top + abs(selLength) - 1));

    // also, pass the mouseover coords to the main window
    emit mouseOverTile(x, y);

}

/*
  Display information about a map tile being hovered over.
  Called when the mouse is over the MapScene without the left button held down.
*/
void MapScene::showTileInfo(QGraphicsSceneMouseEvent *event) {

    QPointF pos = event->scenePos();
    // if hte mouse is moved onto a different tile, erase the old one
    // and draw the new one
    if ((pos.x() / TILE_SIZE) != tileX || (pos.y() / TILE_SIZE) != tileY) {
        tileX = pos.x() / TILE_SIZE;
        tileY = pos.y() / TILE_SIZE;

        // ignore invalid mouseover positions
        // (use the floating point coords to avoid roundoff stupidness)
        if (tileX < level->header.screensH * SCREEN_WIDTH
                && tileY < level->header.screensV * SCREEN_HEIGHT
                && pos.x() >= 0 && pos.y() >= 0) {

            uint8_t tile = level->tiles[tileY][tileX];

            // show tile contents on the status bar
            QString stat(QString("(%1, %2) tile %3 (%4)").arg(tileX).arg(tileY)
                         .arg(hexFormat(tile, 2))
                         .arg(tileType(tilesets[level->tileset][tile].action)));

            emit statusMessage(stat);
        } else {
            tileX = -1;
            tileY = -1;

            emit statusMessage("");
        }

        // also, pass the mouseover coords to the main window
        emit mouseOverTile(tileX, tileY);
    }

}

/*
 *Enable selections/editing using the mouse.
 *If false, mouse events will be ignored and the current selection
 *will be destroyed.
 */
void MapScene::enableSelectTiles(bool on) {
    this->selectTiles = on;
    if (!on) {
        cancelSelection();
    }
}

void MapScene::enableSelectSprites(bool on) {
    this->selectSprites = on;
    cancelSelection();
    for (std::list<SpriteItem*>::iterator i = this->sprites.begin(); i != this->sprites.end(); i++) {
        (*i)->setFlag(QGraphicsItem::ItemIsSelectable, on);
        (*i)->setFlag(QGraphicsItem::ItemIsMovable, on);
    }
}

void MapScene::enableSelectExits(bool on) {
    this->selectExits = on;
    cancelSelection();
    for (std::list<ExitItem*>::iterator i = this->exits.begin(); i != this->exits.end(); i++) {
        (*i)->setFlag(QGraphicsItem::ItemIsSelectable, on);
        (*i)->setFlag(QGraphicsItem::ItemIsMovable, on);
    }
}

/*
 *Enable displaying of screen boundaries
 */
void MapScene::setShowBounds(bool on) {
    showBounds = on;
    update();
}

/*
 *Enable making destructible/breakable tiles translucent
 */
void MapScene::setSeeThrough(bool on) {
    seeThrough = on;
    update();
}

/*
 * Update the vector of map clear rects when editing map clear data
 * so that the cleared-out area surrounding the selected level can be shown.
 */
void MapScene::setClearRects(const std::vector<QRect>* rects) {
    this->clearRects = rects;
    this->update();
}

/*
  Remove the selection pixmap from the scene.
*/
void MapScene::cancelSelection() {
    selecting = false;
    selWidth = 0;
    selLength = 0;
    selX = 0;
    selY = 0;
}

void MapScene::drawBackground(QPainter *painter, const QRectF &rect) {
    QRectF rec = sceneRect() & rect;

    if (rec.isNull())
        return;

    for (uint y = rec.top() / TILE_SIZE; y < rec.bottom() / TILE_SIZE; y++) {
        for (uint x = rec.left() / TILE_SIZE; x < rec.right() / TILE_SIZE; x++) {
            uint8_t tile = level->tiles[y][x];
            // if we're showing map clear rects, block out the other parts of the map
            if (clearRects) {
                tile = 0xF8 + ((x ^ y) & 1);

                for (std::vector<QRect>::const_iterator i = clearRects->begin(); i < clearRects->end(); i++) {
                    if (i->contains(x, y)) {
                        tile = level->tiles[y][x];
                        break;
                    }
                }
            }

            QRect destRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            QRect srcRect (tile * 16, 0, 16, 16);
            painter->drawPixmap(destRect, tilesetPixmap, srcRect);

            // blend destructible tiles with the tile that they turn into
            // (TODO: did i miss any?)
            uint act = tilesets[level->tileset][tile].action;
            if (seeThrough
              && ((act >= 0x1c && act < 0x22)
               || act == 0x25
               || (act >= 0x4c && act < 0x52)
               || act == 0x55
               || (act >= 0x6f && act < 0x75)
               || act == 0x76 || act == 0x77)) {
                tile -= tileSubtract[level->tileset];

                srcRect.moveLeft(tile * 16);
                painter->setOpacity(0.4);
                painter->drawPixmap(destRect, tilesetPixmap, srcRect);
                painter->setOpacity(1.0);
            }
        }
    }

}

void MapScene::drawForeground(QPainter *painter, const QRectF& /* rect */) {
    // highlight tile under cursor
    if (tileX < level->header.screensH * SCREEN_WIDTH
            && tileY < level->header.screensV * SCREEN_HEIGHT
            && tileX >= 0 && tileY >= 0) {

        painter->fillRect(tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                         MapScene::infoBackColor);
    }

    // draw selection
    if (selWidth != 0 && selLength != 0) {
        // account for selections in either negative direction
        int selLeft = qMin(selX, selX + selWidth + 1);
        int selTop  = qMin(selY, selY + selLength + 1);
        QRect selArea(selLeft * TILE_SIZE, selTop * TILE_SIZE, abs(selWidth) * TILE_SIZE, abs(selLength) * TILE_SIZE);
        painter->fillRect(selArea, MapScene::selectionColor);
    }

    // draw screen boundaries
    if (showBounds) for (uint y = 0; y < level->header.screensV; y++) {
        for (uint x = 0; x < level->header.screensH; x++) {
            painter->setPen(Qt::black);
            uint screenX = x * SCREEN_WIDTH * TILE_SIZE;
            uint screenY = y * SCREEN_HEIGHT * TILE_SIZE;

            painter->drawRect(screenX, screenY,
                              SCREEN_WIDTH * TILE_SIZE,
                              SCREEN_HEIGHT * TILE_SIZE);
            painter->drawRect(screenX + 1, screenY + 1,
                              SCREEN_WIDTH * TILE_SIZE - 2,
                              SCREEN_HEIGHT * TILE_SIZE - 2);

            QString infoText = QString::number(y * level->header.screensH + x);
            QRect infoRect = MapScene::infoFontMetrics.boundingRect(infoText);

            painter->fillRect(screenX + 2, screenY + 2,
                             infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + MAP_TEXT_PAD_V,
                             MapScene::infoColor);
            painter->setFont(MapScene::infoFont);
            painter->drawText(screenX + 3, screenY,
                              infoRect.width() + 2 * MAP_TEXT_PAD_H, infoRect.height() + 2 * MAP_TEXT_PAD_V,
                              0, infoText);
        }
    }
}
