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

    virtual void editItem() = 0;

protected:
    QVariant itemChange (GraphicsItemChange change, const QVariant & value);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void updateObject() = 0;
    virtual void updateItem() = 0;

    uint tileSize;

};


class ExitItem : public SceneItem
{
public:
    ExitItem(exit_t*);

    static const QColor fillColor;
    QColor color(bool selected);

    void updateItem();

protected:
    void updateObject();
    void editItem();

private:
    exit_t *exit;

};

class SpriteItem : public SceneItem
{
public:
    SpriteItem(sprite_t*);

    static const QColor fillColor;
    QColor color(bool selected);

    void updateItem();

protected:
    void updateObject();
    void editItem();

private:
    sprite_t *sprite;
};

#endif // SCENEITEM_H
