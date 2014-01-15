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
#include "sceneitem.h"

// TODO: better colors
const QBrush SceneItem::strokeColor(Qt::black);
const QColor SceneItem::selectedColor(255, 0, 255, 128);
const QColor SceneItem::fillColor    (0, 0, 0, 0);

const QColor SpriteItem::fillColor  (255, 0, 0, 128);
const QColor ExitItem::fillColor    (0, 0, 255, 128);

SceneItem::SceneItem():
    QGraphicsItem(0)
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

QRectF SceneItem::boundingRect() const {
    return QRectF(0, 0, TILE_SIZE, TILE_SIZE);
}

QColor SceneItem::color(bool selected) {
    if (selected)
        return SceneItem::selectedColor;

    return SceneItem::fillColor;
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
        QRectF rect = scene()->sceneRect().adjusted(0, 0, -TILE_SIZE, -TILE_SIZE);
        if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
        }
        newPos.setX(round(newPos.x() / TILE_SIZE) * TILE_SIZE);
        newPos.setY(round(newPos.y() / TILE_SIZE) * TILE_SIZE);

        return newPos;
    }

    return QGraphicsItem::itemChange(change, value);
}

ExitItem::ExitItem(exit_t *exit):
    SceneItem()
{
    this->exit = exit;

    this->setX(exit->x * TILE_SIZE);
    this->setY(exit->y * TILE_SIZE);

    // TODO: exit type strings
    this->setToolTip(QString("Exit to level %1 (screen %2, %3, %4)\nType %5")
                     .arg(QString::number(exit->dest, 16).rightJustified(3, QLatin1Char('0')).toUpper())
                     .arg(QString::number(exit->destScreen, 16).toUpper())
                     .arg(exit->destX).arg(exit->destY)
                     .arg(exitType(exit->type)));

}

QColor ExitItem::color(bool selected) {
    if (selected)
        return SceneItem::selectedColor;

    return ExitItem::fillColor;
}

void ExitItem::updateObject() {
    this->exit->x = this->x() / TILE_SIZE;
    this->exit->y = this->y() / TILE_SIZE;
}

SpriteItem::SpriteItem(sprite_t *sprite) :
    SceneItem()
{
    this->sprite = sprite;

    this->setX(sprite->x * TILE_SIZE);
    this->setY(sprite->y * TILE_SIZE);
    // TODO: actual sprite name array
    this->setToolTip(QString("Sprite %1").arg(spriteType(sprite->type)));
}

QColor SpriteItem::color(bool selected) {
    if (selected)
        return SceneItem::selectedColor;

    return SpriteItem::fillColor;
}

void SpriteItem::updateObject() {
    this->sprite->x = this->x() / TILE_SIZE;
    this->sprite->y = this->y() / TILE_SIZE;
}
