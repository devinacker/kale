/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef LEVEL_H
#define LEVEL_H

#include "romfile.h"
#include <cstdint>


#define SCREEN_WIDTH  16
#define SCREEN_HEIGHT 12

extern const uint numLevels;

// location of where to write new level data
extern const uint newDataAddress[];

/*
  The level header.
*/
#pragma pack(1)
typedef struct {
    uint8_t screensH, screensV;
    uint8_t tileIndex, tilePal;
    uint8_t sprIndex, sprPal;
    uint8_t music;
    uint8_t animSpeed;
} header_t;
#pragma pack()

/*
  Definition for level data.
  Currently consists of tile and obstacle data and flags, as well as modified state
  (both overall and for the current session) and music track.
*/
typedef struct {
    header_t  header;

    // The maximum number of screens in a map is 16 (due to memory limits),
    // and each screen is 16x12 tiles.
    uint tiles[16 * SCREEN_HEIGHT][16 * SCREEN_WIDTH];

    // have any of the tile data fields been changed from the original data?
    // (determined based on their position in the ROM file, also set as soon
    // as level is edited)
    bool      modified;
    // have any of the tile data fields been changed in this session?
    // (set when modified, cleared when saved)
    bool      modifiedRecently;

    // tileset number (calculated based on pointer)
    uint8_t   tileset;
} leveldata_t;

/*
  Functions for loading/saving level data
*/
leveldata_t*  loadLevel(ROMFile& file, uint num);
romaddr_t saveLevel(ROMFile& file, uint num, leveldata_t *level, romaddr_t offset);

#endif // LEVEL_H
