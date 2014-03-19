#ifndef SCENEITEM_H
#define SCENEITEM_H

#include <QGraphicsItem>
#include <map>
#include "level.h"

class SceneItem : public QGraphicsItem
{
public:
    SceneItem();

    QRectF boundingRect() const;
    void setDoubleSize(bool on);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QColor color(bool selected);

    static const QBrush strokeColor;
    static const QColor fillColor, selectedColor;

protected:
    QVariant itemChange (GraphicsItemChange change, const QVariant & value);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void updateObject() = 0;
    virtual void updateItem() = 0;
    virtual void editItem() = 0;

    uint tileSize;

};


class ExitItem : public SceneItem
{
public:
    ExitItem(exit_t*);

    static const QColor fillColor;
    QColor color(bool selected);

    exit_t *exit;

protected:
    void updateObject();
    void updateItem();
    void editItem();

};

class SpriteItem : public SceneItem
{
public:
    SpriteItem(sprite_t*);

    static const QColor fillColor;
    QColor color(bool selected);

    sprite_t *sprite;

protected:
    void updateObject();
    void updateItem();
    void editItem();

};

#endif // SCENEITEM_H
