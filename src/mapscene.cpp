/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include <QPixmap>
#include <QPainter>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include "level.h"
#include "mainwindow.h"
#include "mapscene.h"
#include "mapchange.h"
#include "graphics.h"

#define MAP_TEXT_OFFSET 8
#define MAP_TEXT_PAD 2

const QFont MapScene::infoFont("Consolas", 8);

const QColor MapScene::infoColor(255, 192, 192, 192);
const QColor MapScene::infoBackColor(255, 192, 192, 64);

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
      copyWidth(0), copyLength(0),
      stack(this),
      level(currentLevel),
      infoItem(NULL), selectionItem(NULL)

{
    QObject::connect(this, SIGNAL(edited()),
                     this, SLOT(refresh()));
}

/*
  clear the scene AND erase the item pointers
*/
void MapScene::erase() {
    clear();
    infoItem = NULL;
    selectionItem = NULL;
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
    drawLevelMap();
    removeInfoItem();
    updateSelection();
}

/*
  Handle when the mouse is pressed on the scene
*/
void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // if the level is not being displayed, don't do anything
    if (!isActive()) return;

    // left button: start or continue selection
    // right button: cancel selection
    if (event->buttons() & Qt::LeftButton) {
        beginSelection(event);

        event->accept();

    } else if (event->buttons() & Qt::RightButton) {
        cancelSelection(true);

        event->accept();
    }
}

/*
  Handle when a double-click occurs (used to start the tile edit window)
*/
void MapScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    if (isActive()) {
        editTiles();
        emit doubleClicked();

        event->accept();
    }
}

/*
  Handle when the left mouse button is released
*/
void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
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
    }
}

/*
  Handle when the mouse is moved over the scene
 */
void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // if inactive, don't handle mouse moves
    if (!isActive()) return;

    // behave differently based on left mouse button status
    if (selecting && event->buttons() & Qt::LeftButton) {
        // left button down: destroy tile info pixmap
        // and generate/show selection
        removeInfoItem();
        updateSelection(event);
    } else {
        showTileInfo(event);
    }

    event->accept();
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

    // temporarily destroy the old selection pixmap
    cancelSelection(false);

    QRect selArea(0, 0, abs(selWidth) * TILE_SIZE, abs(selLength) * TILE_SIZE);
    QPixmap selPixmap(selArea.width(), selArea.height());
    selPixmap.fill(selectionColor);

    int top = std::min(y, selY);
    int left = std::min(x, selX);

    selectionItem = addPixmap(selPixmap);
    selectionItem->setOffset(left * TILE_SIZE, top * TILE_SIZE);

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

        removeInfoItem();

        // brand new pixmap
        QPixmap infoPixmap(TILE_SIZE, TILE_SIZE);
        infoPixmap.fill(QColor(0,0,0,0));

        // ignore invalid mouseover positions
        // (use the floating point X coord to avoid roundoff stupidness)
        if (tileX < level->header.screensH * SCREEN_WIDTH
                && tileY < level->header.screensV * SCREEN_HEIGHT
                && pos.x() >= 0 && tileY >= 0) {
            QPainter painter(&infoPixmap);

            // render background
            painter.fillRect(0, 0, TILE_SIZE, TILE_SIZE,
                             MapScene::infoBackColor);

            // render tile info
            painter.setFont(MapScene::infoFont);
            uint8_t tile = level->tiles[tileY][tileX];

            // only draw bottom part if terrain != 0 (i.e. not empty space)
            if (tile) {
                //...
            }

            // show tile contents on the status bar
            QString stat(QString("(%1,%2)").arg(tileX).arg(tileY));
            // ...

            emit statusMessage(stat);
        }

        infoItem = addPixmap(infoPixmap);
        infoItem->setOffset(tileX * TILE_SIZE, tileY * TILE_SIZE);

        // also, pass the mouseover coords to the main window
        emit mouseOverTile(tileX, tileY);
    }

}

/*
  Remove the selection pixmap from the scene.
  if "perma" is true, the selection rectangle is also reset.
*/

// public version used to deselect when changing levels
void MapScene::cancelSelection() {
    cancelSelection(true);
}

void MapScene::cancelSelection(bool perma) {
    if (perma) {
        selWidth = 0;
        selLength = 0;
        selX = 0;
        selY = 0;
    }

    if (selectionItem) {
        removeItem(selectionItem);
        delete selectionItem;
        selectionItem = NULL;
    }
}

void MapScene::removeInfoItem() {
    // infoItem is no longer valid so kill it
    if (infoItem) {
        removeItem(infoItem);
        delete infoItem;
        infoItem = NULL;
    }
}

void MapScene::drawLevelMap() {
    // reset the scene (remove all members)
    erase();

    // if level is null , minimize the scene and return
    if (!level) {
        setSceneRect(0, 0, 0, 0);
        return;
    }

    uint width = level->header.screensH * SCREEN_WIDTH;
    uint height = level->header.screensV * SCREEN_HEIGHT;

    // set the pixmap and scene size based on the level's size
    QPixmap pixmap(width * TILE_SIZE, height * TILE_SIZE);
    // TODO: use the actual palettes' background
    pixmap.fill(QColor(128, 128, 128));

    setSceneRect(0, 0, width * TILE_SIZE, height * TILE_SIZE);

    // no width/height = don't draw anything
    if (width + height == 0) {
        addPixmap(pixmap);
        update();
        return;
    }

    // assign a painter to the target pixmap
    QPainter painter;
    painter.begin(&pixmap);

    // slowly blit shit from the tile resource onto the pixmap
    for (uint y = 0; y < height; y++) {
        for (uint x = 0; x < width; x++) {
            uint8_t tile = level->tiles[y][x];
            if (!tile) continue;

            // TODO: load tileset and color by palette + behavior for nicer results
            QColor fill(255-(tile % 128), 160, 255);
            painter.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, fill);
            // TODO also: draw tile numbers
        }
    }

    // TODO: draw screen boundaries

    // put the new finished pixmap into the scene
    painter.end();

    addPixmap(pixmap);
    update();

}
