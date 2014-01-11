#include <QPainter>
#include "exititem.h"
#include "level.h"
#include "graphics.h"

// TODO: better colors
const QBrush ExitItem::strokeColor(Qt::black, Qt::NoBrush);
const QColor ExitItem::fillColor  (0, 0, 255, 128);

ExitItem::ExitItem(exit_t *exit):
    QGraphicsItem(0)
{
    this->exit = exit;

    this->setX(exit->x * TILE_SIZE);
    this->setY(exit->y * TILE_SIZE);

    // TODO: exit type strings
    this->setToolTip(QString("Exit to level %1 (screen %2, %3, %4)\nType %5 (%6)")
                     .arg(QString::number(exit->dest, 16).rightJustified(3, QLatin1Char('0')).toUpper())
                     .arg(QString::number(exit->destScreen, 16).toUpper())
                     .arg(exit->destX).arg(exit->destY)
                     .arg(QString::number(exit->type, 16).rightJustified(2, QLatin1Char('0')).toUpper())
                     .arg("some exit type"));
}

QRectF ExitItem::boundingRect() const {
    return QRectF(0, 0, TILE_SIZE, TILE_SIZE);
}

void ExitItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // just draw a goofy rectangle for now, dunno a better thing to do
    // (door icon?)
    painter->fillRect(this->boundingRect(), ExitItem::fillColor);
    painter->setBrush(ExitItem::strokeColor);
    painter->drawRect(this->boundingRect());
    painter->drawRect(QRectF(1, 1, TILE_SIZE-2, TILE_SIZE-2));
}
