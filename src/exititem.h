#ifndef EXITITEM_H
#define EXITITEM_H

#include <QGraphicsItem>
#include "level.h"

class ExitItem : public QGraphicsItem
{
public:
    ExitItem(exit_t*);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    static const QBrush strokeColor;
    static const QColor fillColor;

private:
    exit_t *exit;

};

#endif // EXITITEM_H
