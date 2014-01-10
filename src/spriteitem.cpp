/*
 * QGraphicsItem that represents a sprite in the level.
 * Contains a pointer to the sprite itself, which it manipulates when the user interacts
 * with the item in some way (dragging around to move, or double/clicking to edit.)
 * No interactivity is currently implemented, nor graphics, so just appears as an
 * immobile rectangle.
 * Sprite info is shown in the tooltip.
 *
 */
#include <QPainter>
#include "spriteitem.h"
#include "level.h"
#include "graphics.h"

// TODO: better colors
const QColor SpriteItem::strokeColor(255, 0, 0, 255);
const QColor SpriteItem::fillColor  (255, 0, 0, 128);

SpriteItem::SpriteItem(sprite_t *sprite) :
    QGraphicsItem(0)
{
    this->sprite = sprite;

    this->setX(sprite->x * TILE_SIZE);
    this->setY(sprite->y * TILE_SIZE);
    // TODO: actual sprite name array
    this->setToolTip(QString("Sprite %1")
                     .arg(QString::number(sprite->type, 16).rightJustified(2, QLatin1Char('0')).toUpper()));
}

QRectF SpriteItem::boundingRect() const {
    return QRectF(0, 0, TILE_SIZE, TILE_SIZE);
}

void SpriteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // just draw a goofy rectangle for now until actual sprite graphics loading is in
    painter->fillRect(this->boundingRect(), SpriteItem::fillColor);
    //painter->drawRect(this->boundingRect(), SpriteItem::strokeColor);
}
