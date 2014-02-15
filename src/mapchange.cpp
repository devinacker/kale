/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include "mapchange.h"
#include "level.h"

MapChange::MapChange(leveldata_t *currLevel, uint selX, uint selY, uint selW, uint selL, QUndoCommand *parent) :
    QUndoCommand(parent),
    level(currLevel),
    x(selX), y(selY), w(selW), l(selL),
    before(new uint[l * w]),
    after (new uint[l * w]),
    first(true)
{
    // when instantiated, save the region's pre-edit state
    if (level) {
        for (uint row = 0; row < l; row++)
            for (uint col = 0; col < w; col++)
                before[(row * w) + col] = this->level->tiles[y + row][x + col];

        this->setText("edit");
    }
}

MapChange::~MapChange() {
    delete[] before;
    delete[] after;
}

void MapChange::undo() {
    // restore to the region's pre-edit state
    if (level) {
        for (uint row = 0; row < this->l; row++)
            for (uint col = 0; col < this->w; col++)
                this->level->tiles[y + row][x + col] = before[(row * w) + col];
    }
}

void MapChange::redo() {
    // if being pushed, save the region's post-edit state
    // otherwise restore it
    if (level) {
        for (uint row = 0; row < this->l; row++)
            for (uint col = 0; col < this->w; col++) {
                if (first) after[(row * w) + col] = this->level->tiles[y + row][x + col];
                else       this->level->tiles[y + row][x + col] = after[(row * w) + col];
            }

        if (first) {
            level->modified = true;
            level->modifiedRecently = true;
        }
    }

    first = false;
}

void MapChange::setText(const QString &text) {
    QUndoCommand::setText(QString(text)
                          .append(" from (%1, %2) to (%3, %4)")
                          .arg(x).arg(y).arg(x + w - 1).arg(y + l - 1));
}

SpriteChange::SpriteChange(SpriteItem *item, sprite_t *sprite, sprite_t before,
                           QUndoCommand *parent):
    QUndoCommand(parent),
    item(item),
    sprite(sprite),
    before(before)
{
    this->after = *sprite;

    this->setText("sprite edit");
}

void SpriteChange::undo() {
    *sprite = before;
    item->updateItem();
}

void SpriteChange::redo() {
    *sprite = after;
    item->updateItem();
}

ExitChange::ExitChange(ExitItem *item, exit_t *exit, exit_t before,
                       QUndoCommand *parent):
    QUndoCommand(parent),
    item(item),
    exit(exit),
    before(before)
{
    this->after = *exit;

    this->setText("exit edit");
}

void ExitChange::undo() {
    *exit = before;
    item->updateItem();
}

void ExitChange::redo() {
    *exit = after;
    item->updateItem();
}
