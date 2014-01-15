/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include <QPixmap>
#include <QPainter>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include "level.h"
#include "mainwindow.h"
#include "mapscene.h"
#include "mapchange.h"
#include "graphics.h"
#include "tileset.h"
#include "sceneitem.h"
#include "stuff.h"

#define MAP_TEXT_OFFSET 8
#define MAP_TEXT_PAD 2

const QFont MapScene::infoFont("Consolas", 8);

const QColor MapScene::infoColor(255, 192, 192, 192);
const QColor MapScene::infoBackColor(255, 192, 192, 128);

const QColor MapScene::selectionColor(255, 192, 192, 128);
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
      level(currentLevel)

{
    QObject::connect(this, SIGNAL(edited()),
                     this, SLOT(refresh()));
}

/*
  Edit the currently selected tiles (if any)
*/
void MapScene::editTiles() {
    if (selWidth == 0 || selLength == 0)
        return;

    /*
    MapChange *edit = new MapChange(level, selX, selY, selWidth, selLength);

    // send the level and selection info to a new tile edit window instance
    TileEditWindow win;
    if (win.startEdit(level, QRect(selX, selY, selWidth, selLength)))
        stack.push(edit);
    else delete edit;

    // redraw the map scene with the new properties
    emit edited();
    updateSelection();
    */
    qDebug("MapScene::editTiles() not implemented");
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

    // add sprites
    for (std::vector<sprite_t>::iterator i = level->sprites.begin(); i != level->sprites.end(); i++) {
        SpriteItem *spr = new SpriteItem(&(*i));
        spr->setFlag(QGraphicsItem::ItemIsSelectable, selectSprites);
        spr->setFlag(QGraphicsItem::ItemIsMovable, selectSprites);
        addItem(spr);
        this->sprites.push_back(spr);
    }

    // add exits
    for (std::vector<exit_t>::iterator i = level->exits.begin(); i != level->exits.end(); i++) {
        ExitItem *exit = new ExitItem(&(*i));
        exit->setFlag(QGraphicsItem::ItemIsSelectable, selectExits);
        exit->setFlag(QGraphicsItem::ItemIsMovable, selectExits);
        addItem(exit);
        this->exits.push_back(exit);
    }

    update();
}

/*
  Handle when the mouse is pressed on the scene
*/
void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // if the level is not being displayed, don't do anything
    if (!isActive()) return;

    // different selection type: try passing the event somewhere else
    if (!selectTiles) {
        QGraphicsScene::mousePressEvent(event);

    // left button: start or continue selection
    // right button: cancel selection
    } else if (event->buttons() & Qt::LeftButton) {
        beginSelection(event);
        event->accept();

    } else if (event->buttons() & Qt::RightButton) {
        cancelSelection();
        event->accept();
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

    /*
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
                level->tiles[selY + i][selX + j] = noTile;
        }
    }

    copyWidth = selWidth;
    copyLength = selLength;

    if (cut) {
        stack.push(edit);
        emit edited();
    }

    updateSelection();
    emit statusMessage(QString("%1 (%2, %3) to (%4, %5)")
                       .arg(cut ? "Cut" : "Copied")
                       .arg(selX).arg(selY)
                       .arg(selX + selWidth - 1)
                       .arg(selY + selLength - 1));
    */
    qDebug("MapScene::copyTiles not implemented");
}

void MapScene::paste() {
    // if there is no selection or copy buffer, don't do anything
    if (selWidth == 0 || selLength == 0
            || copyWidth == 0 || copyLength == 0) return;

    /*
    MapChange *edit = new MapChange(level, selX, selY, copyWidth, copyLength);
    edit->setText("paste");

    // otherwise, move stuff into the level from the buffer
    for (uint i = 0; i < copyLength && selY + i < 64; i++) {
        for (uint j = 0; j < copyWidth && selX + j < 64; j++) {
            level->tiles[selY + i][selX + j] = copyBuffer[i][j];
        }
    }

    stack.push(edit);
    emit edited();
    updateSelection();

    emit statusMessage(QString("Pasted (%1, %2) to (%3, %4)")
                       .arg(selX).arg(selY)
                       .arg(selX + copyWidth - 1)
                       .arg(selY + copyLength - 1));
    */

    qDebug("MapScene::paste not implemented");
}

void MapScene::deleteTiles() {
    // if there is no selection, don't do anything
    if (selWidth == 0 || selLength == 0) return;

    /*
    MapChange *edit = new MapChange(level, selX, selY, selWidth, selLength);
    edit->setText("delete");

    // otherwise, delete stuff
    for (int i = 0; i < selLength && selY + i < 64; i++) {
        for (int j = 0; j < selWidth && selX + j < 64; j++) {
            level->tiles[selY + i][selX + j] = noTile;
        }
    }

    stack.push(edit);
    emit edited();
    updateSelection();

    emit statusMessage(QString("Deleted (%1, %2) to (%3, %4)")
                       .arg(selX).arg(selY)
                       .arg(selX + selWidth - 1)
                       .arg(selY + selLength - 1));
    */
    qDebug("MapScene::delete not implemented");
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
                         .arg(QString::number(tile, 16).rightJustified(2, QLatin1Char('0')).toUpper())
                         .arg(tileType(tilesets[level->tileset][tile].action)));

            emit statusMessage(stat);
        } else {
            tileX = -1;
            tileY = -1;
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
    for (std::vector<SpriteItem*>::iterator i = this->sprites.begin(); i != this->sprites.end(); i++) {
        (*i)->setFlag(QGraphicsItem::ItemIsSelectable, on);
        (*i)->setFlag(QGraphicsItem::ItemIsMovable, on);
    }
}

void MapScene::enableSelectExits(bool on) {
    this->selectExits = on;
    cancelSelection();
    for (std::vector<ExitItem*>::iterator i = this->exits.begin(); i != this->exits.end(); i++) {
        (*i)->setFlag(QGraphicsItem::ItemIsSelectable, on);
        (*i)->setFlag(QGraphicsItem::ItemIsMovable, on);
    }
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
            //if (!tile) continue;
            uint tileset = level->tileset;
            uint palette = tilesets[tileset][tile].palette;
            uint action = tilesets[tileset][tile].action;

            QColor fill;
            // make up some pretty(???) colors for now;
            // obviously real palettes will be loaded eventually
            uint shade = (action % 64) * 2;
            if (action == 0xFF)
                shade = tile % 128;

            switch (palette) {
            case 0:
                // blue
                fill = QColor(127+shade, 127+shade, 255);
                break;
            case 1:
                // green
                fill = QColor(127+shade, 255, 127+shade);
                break;
            case 2:
                // red
                fill = QColor(255, 127+shade, 127+shade);
                break;
            case 3:
                // orange/brown
                fill = QColor(127+shade, 64+shade, 64);
            }

            painter->fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, fill);
        }
    }

    // TODO: draw screen boundaries

}

void MapScene::drawForeground(QPainter *painter, const QRectF &rect) {
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
}
