/*
 * QGraphicsItem that represents an object (sprite/exit) in the level.
 * Contains a pointer to the thing itself, which it manipulates when the user interacts
 * with the item in some way (dragging around to move, or double/clicking to edit.)
 * No interactivity is currently implemented, nor graphics, so just appears as an
 * immobile rectangle.
 * Item info is shown in the tooltip.
 *
 */

#include <QPainter>
#include <QGraphicsScene>
#include <cmath>
#include "level.h"
#include "graphics.h"
#include "stuff.h"
#include "mapscene.h"
#include "sceneitem.h"
#include "mapchange.h"
#include "spriteeditwindow.h"
#include "exiteditwindow.h"

// TODO: better colors
const QBrush SceneItem::strokeColor(Qt::black);
const QColor SceneItem::selectedColor(255, 0, 255, 128);
const QColor SceneItem::fillColor    (0, 0, 0, 0);

const QColor SpriteItem::fillColor  (255, 0, 0, 128);
const QColor ExitItem::fillColor    (0, 0, 255, 128);

SceneItem::SceneItem():
    QGraphicsItem(0),
    tileSize(TILE_SIZE)
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

void SceneItem::setDoubleSize(bool on) {
    if (on)
        tileSize = TILE_SIZE * 2;
    else
        tileSize = TILE_SIZE;

    this->prepareGeometryChange();
    this->updateItem();
}

QRectF SceneItem::boundingRect() const {
    return QRectF(0, 0, tileSize, tileSize);
}

QColor SceneItem::color(bool selected) {
    if (selected)
        return SceneItem::selectedColor;

    return SceneItem::fillColor;
}

void SceneItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    this->editItem();
}

void SceneItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // just draw a goofy rectangle for now, dunno a better thing to do
    // (door icon?)
    painter->fillRect(this->boundingRect(), this->color(this->isSelected()));

    painter->setPen(QPen(strokeColor, 2));
    painter->drawRect(this->boundingRect());
}

QVariant SceneItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (scene() && change == QGraphicsItem::ItemPositionChange) {
        QPointF newPos = value.toPointF();
        // rect is adjusted so that items cannot be placed one tile to the right/bottom
        int adjust = -tileSize;
        QRectF rect = scene()->sceneRect().adjusted(0, 0, adjust, adjust);

        if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
        }
        newPos.setX(round(newPos.x() / tileSize) * tileSize);
        newPos.setY(round(newPos.y() / tileSize) * tileSize);

        return newPos;
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {
        // update the position of the sprite/exit (or whatever) based on the new position

        // TODO: generate undo/redo actions for movement somehow
        this->updateObject();
        return value;
    }

    return QGraphicsItem::itemChange(change, value);
}

ExitItem::ExitItem(exit_t *exit):
    SceneItem()
{
    this->exit = exit;

    this->updateItem();

}

QColor ExitItem::color(bool selected) {
    if (selected)
        return SceneItem::selectedColor;

    return ExitItem::fillColor;
}

void ExitItem::updateObject() {
    this->exit->x = this->x() / tileSize;
    this->exit->y = this->y() / tileSize;
}

void ExitItem::updateItem() {
    this->setPos(exit->x * tileSize, exit->y * tileSize);

    this->setToolTip(QString("Exit to level %1 (screen %2, %3, %4)\nType %5")
                     .arg(QString::number(exit->dest, 16).rightJustified(3, QLatin1Char('0')).toUpper())
                     .arg(QString::number(exit->destScreen, 16).toUpper())
                     .arg(exit->destX).arg(exit->destY)
                     .arg(exitType(exit->type)));
}

void ExitItem::editItem() {
    exit_t before = *exit;

    ExitEditWindow win(NULL, this->exit);
    if (win.exec()) {
        MapScene *scene = dynamic_cast<MapScene*>(this->scene());
        if (scene) {
            scene->pushChange(new ExitChange(this, exit, before));
        }
    }

    updateItem();
}

SpriteItem::SpriteItem(sprite_t *sprite) :
    SceneItem()
{
    this->sprite = sprite;

    this->updateItem();
}

QColor SpriteItem::color(bool selected) {
    if (selected)
        return SceneItem::selectedColor;

    return SpriteItem::fillColor;
}

void SpriteItem::updateObject() {
    this->sprite->x = this->x() / tileSize;
    this->sprite->y = this->y() / tileSize;
}

void SpriteItem::updateItem() {
    this->setPos(sprite->x * tileSize, sprite->y * tileSize);
    this->setToolTip(QString("Sprite %1").arg(spriteType(sprite->type)));
}

void SpriteItem::editItem() {
    sprite_t before = *sprite;

    SpriteEditWindow win(NULL, this->sprite);
    if (win.exec()) {
        MapScene *scene = dynamic_cast<MapScene*>(this->scene());
        if (scene) {
            scene->pushChange(new SpriteChange(this, sprite, before));
        }
    }

    updateItem();
}
