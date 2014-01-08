/*
  level.cpp

  Contains functions for loading and saving level data.

  This code is released under the terms of the MIT license.
  See COPYING.txt for details.
*/

#include "compress.h"
#include "romfile.h"
#include "level.h"

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <iostream>

#include <QMessageBox>
#include <QString>
#include <QCoreApplication>

#define MAP_DATA_SIZE 0xCDA

//using namespace stuff;

// high/low parts of level pointer table are 328 bytes apart;
// the last one seems to be bogus
const uint numLevels = 0x147;

//Locations of chunk data in ROM (using CPU addressing.)
//currently assumes table locations are constant in all versions,
// may need to change this later to use version arrays like kdceditor
//...
const romaddr_t ptrMapDataL = {0x12, 0x88a6};
const romaddr_t ptrMapDataH = {0x12, 0x875f};
const romaddr_t ptrMapDataB = {0x12, 0x84d1};
const romaddr_t mapTilesets = {0x12, 0x8618};

/*
  Load a level by number. Returns pointer to the level data as a struct.
  Returns null if a level failed and the user decided not to continue.
*/
leveldata_t* loadLevel (ROMFile& file, uint num) {
    //uint8_t  buf[MAP_DATA_SIZE];
    //invalid data should at least be able to decompress fully
    uint8_t  buf[65536];
    header_t *header  = (header_t*)buf + 0;
    uint8_t  *screens = buf + 8;
    uint8_t  *tiles   = buf + 0xDA;

    auto result = file.readFromPointer(ptrMapDataL, ptrMapDataH, ptrMapDataB, 0, buf, num);
    // TODO: "error reading level, attempt to continue?"
    if (result == -1) return NULL;
    /*
    std::cerr << QString("level %1 width %2 height %3\n")
            .arg(num).arg(header->screensH).arg(header->screensV).toStdString();
    */

    leveldata_t *level = (leveldata_t*)malloc(sizeof(leveldata_t));
    if (!level) {
        QMessageBox::critical(0,
                              "Load ROM",
                              QString("Unable to allocate memory for level %1").arg(num),
                              QMessageBox::Ok);
        return NULL;
    }

    memcpy(&level->header, header, sizeof(header_t));

    // kinda slow, but eh
    for (uint y = 0; y < SCREEN_HEIGHT * header->screensV; y++) {
        for (uint x = 0; x < SCREEN_WIDTH * header->screensH; x++) {
            uint idx = (y / SCREEN_HEIGHT * header->screensH) + (x / SCREEN_WIDTH);
            uint8_t screen = screens[idx];
            level->tiles[y][x] = tiles[(screen * SCREEN_HEIGHT * SCREEN_WIDTH)
                    + (y % SCREEN_HEIGHT * 16) + (x % SCREEN_WIDTH)];
        }
    }

    level->modified = false;
    level->modifiedRecently = false;

    level->tileset = file.readByte(mapTilesets + num);

    return level;
}

/*
  Save a level back to the ROM. Returns the next available ROM address
  to write level data to.
*/
romaddr_t saveLevel(ROMFile& file, uint num, leveldata_t *level, romaddr_t addr) {

    return addr;
}
