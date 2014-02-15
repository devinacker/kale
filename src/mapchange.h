/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef MAPCHANGE_H
#define MAPCHANGE_H

#include <QUndoCommand>

#include "level.h"
#include "sceneitem.h"

class MapChange : public QUndoCommand
{
public:
    explicit MapChange(leveldata_t *currLevel,
                       uint x, uint y, uint w, uint l,
                       QUndoCommand *parent = 0);
    ~MapChange();

    void undo();
    void redo();
    void setText(const QString &text);

private:
    leveldata_t *level;
    uint x, y, w, l;
    uint *before, *after;
    bool first;
};

class SpriteChange : public QUndoCommand
{
public:
    explicit SpriteChange(SpriteItem *item, sprite_t *sprite,
                          sprite_t before,
                          QUndoCommand *parent = 0);

    void undo();
    void redo();

private:
    SpriteItem *item;
    sprite_t *sprite;
    sprite_t before, after;
};


#endif // MAPCHANGE_H
