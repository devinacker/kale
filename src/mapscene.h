/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QUndoStack>
#include <QTimer>
#include <QFontMetrics>
#include <list>
#include <vector>

#include "level.h"
#include "sceneitem.h"

// subclass of QGraphicsScene used to draw the 2d map and handle mouse/kb events for it
class MapScene : public QGraphicsScene {
    Q_OBJECT

private:
    static const QColor infoColor, infoBackColor;
    static const QColor selectionColor, selectionBorder;
    static const QColor layerColor;
    static const QColor extraColor;
    static const QFont infoFont;
    static const QFontMetrics infoFontMetrics;

    int tileX, tileY;
    int selX, selY, selLength, selWidth;
    bool selecting;
    bool selectTiles, selectSprites, selectExits;

    std::list<SpriteItem*> sprites;
    std::list<ExitItem*>   exits;

    uint copyBuffer[16*SCREEN_HEIGHT][16*SCREEN_WIDTH];
    uint copyWidth, copyLength;

    QUndoStack stack;

    leveldata_t *level;

    QPixmap tilesetPixmap;
    uint animFrame;
    QTimer animTimer;

    bool showBounds, seeThrough, showExtra;
    uint tileSize;

    // used to display map clear rects when non-null
    const std::vector<QRect> *clearRects;

    void copyTiles(bool cut);
    void deleteTiles();
    void deleteItems();
    void showTileInfo(QGraphicsSceneMouseEvent *event);
    void beginSelection(QGraphicsSceneMouseEvent *event);
    void updateSelection(QGraphicsSceneMouseEvent *event = NULL);
    void drawLevelMap();

public:
    MapScene(QObject *parent = 0, leveldata_t *currentLevel = 0);

    bool canUndo() const;
    bool canRedo() const;
    bool isClean() const;
    void pushChange(QUndoCommand*);

    void enableSelectTiles(bool);
    void enableSelectSprites(bool);
    void enableSelectExits(bool);
    void cancelSelection();

    const QPixmap* getPixmap() const;

public slots:
    void editTiles();
    void undo();
    void redo();
    void clearStack();
    void setClean();
    void cut();
    void copy();
    void paste();
    void deleteStuff();
    void setAnimSpeed(int);
    void refresh();
    void refreshPixmap();
    void animate();
    void setShowBounds(bool);
    void setSeeThrough(bool);
    void setClearRects(const std::vector<QRect>*);
    void setShowExtra(bool);

signals:
    void doubleClicked();
    void statusMessage(QString);
    void mouseOverTile(int x, int y);
    void edited();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawForeground(QPainter *painter, const QRectF &rect);
};

#endif // MAPSCENE_H
