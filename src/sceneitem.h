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
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QColor color(bool selected);

    static const QBrush strokeColor;
    static const QColor fillColor, selectedColor;

protected:
    QVariant itemChange (GraphicsItemChange change, const QVariant & value);
    virtual void updateObject() = 0;
};


class ExitItem : public SceneItem
{
public:
    ExitItem(exit_t*);

    static const QColor fillColor;
    QColor color(bool selected);

protected:
    void updateObject();

private:
    exit_t *exit;

};

class SpriteItem : public SceneItem
{
public:
    SpriteItem(sprite_t*);

    static const QColor fillColor;
    QColor color(bool selected);

protected:
    void updateObject();

private:
    sprite_t *sprite;
};

#endif // SCENEITEM_H
