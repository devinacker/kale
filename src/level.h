/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef LEVEL_H
#define LEVEL_H

#include "romfile.h"
#include <cstdint>
#include <list>

#define SCREEN_WIDTH  16
#define SCREEN_HEIGHT 12

// high/low parts of level pointer table are 328 bytes apart;
// the last one seems to be bogus
#define NUM_LEVELS 0x147

/*
  The level header.
*/
#pragma pack(1)
struct header_t {
    uint8_t screensH, screensV;
    uint8_t tileIndex, tilePal;
    uint8_t sprIndex, sprPal;
    uint8_t music;
    uint8_t animSpeed;
};
#pragma pack()

struct sprite_t {
    uint8_t type;
    uint x, y;
    // used only for sorting sprites by screen number when saving back to ROM
    uint8_t screen;
    bool operator< (const sprite_t &other) const { return screen < other.screen; }

};

struct exit_t {
    uint8_t type;
    uint x, y;
    uint dest, destScreen, destX, destY;
    uint bossLevel, bossScreen, bossX, bossY;
};

/*
  Definition for level data.
  Currently consists of tile and obstacle data and flags, as well as modified state,
  tileset #, and sprites/exits.
*/
struct leveldata_t {
    header_t  header;

    // The maximum number of screens in a map is 16 (due to memory limits),
    // and each screen is 16x12 tiles.
    uint8_t   tiles[16 * SCREEN_HEIGHT][16 * SCREEN_WIDTH];

    // tileset number
    uint8_t   tileset;

    // containers for other data
    std::list<sprite_t*> sprites;
    std::list<exit_t*>   exits;

    // don't return to this level after losing a life?
    bool      noReturn;

    // have any of the tile data fields been changed in this session?
    // (set when modified, cleared when saved)
    bool      modified;

    leveldata_t() : tiles {{0}}, sprites(), exits() {}
    ~leveldata_t() {
        // cleanup sprites/exits
        for (std::list<sprite_t*>::iterator i = sprites.begin(); i != sprites.end(); i++)
            delete *i;

        for (std::list<exit_t*>::iterator i = exits.begin(); i != exits.end(); i++)
            delete *i;
    }

};

/*
  Functions for loading/saving level data
*/
leveldata_t*  loadLevel(ROMFile& file, uint num);
DataChunk     packLevel  (const leveldata_t *level, uint num);
DataChunk     packSprites(const leveldata_t *level, uint num);
void          saveLevel(ROMFile& file, const DataChunk &chunk, const leveldata_t *level, romaddr_t offset);
void          saveExits(ROMFile& file, const leveldata_t *level, uint num);
void          saveSprites(ROMFile& file, const DataChunk& chunk, romaddr_t addr);

#endif // LEVEL_H
