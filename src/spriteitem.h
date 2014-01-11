
#ifndef SPRITEITEM_H
#define SPRITEITEM_H

#include <QGraphicsItem>
#include "level.h"

class SpriteItem : public QGraphicsItem
{
public:
    SpriteItem(sprite_t*);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    static const QBrush strokeColor;
    static const QColor fillColor;

private:
    sprite_t *sprite;

};

#endif // SPRITEITEM_H
