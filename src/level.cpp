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
#include <stdexcept>

#include <QMessageBox>
#include <QString>
#include <QCoreApplication>

#define MAP_DATA_SIZE 0xCDA

//using namespace stuff;

//Locations of chunk data in ROM (using CPU addressing.)
//currently assumes table locations are constant in all versions,
// may need to change this later to use version arrays like kdceditor
//...
const romaddr_t ptrMapDataL = {0x12, 0x88a6};
const romaddr_t ptrMapDataH = {0x12, 0x875f};
const romaddr_t ptrMapDataB = {0x12, 0x84d1};
const romaddr_t mapTilesets = {0x12, 0x8618};
const romaddr_t ptrSpritesL = {0x12, 0x8d0e};
const romaddr_t ptrSpritesH = {0x12, 0x8bc7};
const romaddr_t ptrSpritesB = {0x12, 0x8a80};
const romaddr_t ptrExitsL   = {0x12, 0x8f82};
const romaddr_t ptrExitsH   = {0x12, 0x90cb};
const uint      ptrExitsB   = 0x12;

/*
  Load a level by number. Returns pointer to the level data as a struct.
  Returns null if a level failed and the user decided not to continue.
*/
leveldata_t* loadLevel (ROMFile& file, uint num) {
    //invalid data should at least be able to decompress fully
    uint8_t  buf[65536] = {0};
    header_t *header  = (header_t*)buf + 0;
    uint8_t  *screens = buf + 8;
    uint8_t  *tiles   = buf + 0xDA;

    auto result = file.readFromPointer(ptrMapDataL, ptrMapDataH, ptrMapDataB, 0, buf, num);
    // TODO: "error reading level, attempt to continue?"
    if (result == 0) return NULL;

    //leveldata_t *level = (leveldata_t*)malloc(sizeof(leveldata_t));
    leveldata_t *level;
    try {
        level = new leveldata_t;
    } catch (std::bad_alloc) {
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

    // get tileset from table
    level->tileset = file.readByte(mapTilesets + num);

    // get "don't return on death" flag
    // (which is the highest bit of the level pointer's bank byte)
    level->noReturn = file.readByte(ptrMapDataB + num) & 0x80;

    // get sprite data
    romaddr_t spritePtr = file.readPointer(ptrSpritesL, ptrSpritesH, ptrSpritesB, num);
    // true number of screens (this can differ in e.g. rotating tower levels)
    uint sprScreens = file.readByte(spritePtr);

    // last # of sprite on each screen
    romaddr_t spriteCounts = spritePtr + 2;
    uint numSprites = file.readByte(spriteCounts + (sprScreens - 1));
    // position of each sprite
    romaddr_t spritePos    = spriteCounts + sprScreens;
    // type of each sprite
    romaddr_t spriteTypes  = spritePos + numSprites;

    uint sprNum = 0;
    for (uint i = 0; i < sprScreens; i++) {
        // number of sprites on this screen
        numSprites = file.readByte(spriteCounts + i);
        while (sprNum < numSprites) {            
            sprite_t sprite;

            sprite.type = file.readByte(spriteTypes + sprNum);
            uint8_t pos = file.readByte(spritePos + sprNum);

            // calculate normal x/y positions
            sprite.x = (i % header->screensH * SCREEN_WIDTH) + (pos >> 4);
            sprite.y = (i / header->screensH * SCREEN_HEIGHT) + (pos & 0xF);

            level->sprites.push_back(sprite);
            sprNum++;
        }
    }

    // get exit data
    romaddr_t exits     = file.readShortPointer(ptrExitsL, ptrExitsH, ptrExitsB, num);
    romaddr_t nextExits = file.readShortPointer(ptrExitsL, ptrExitsH, ptrExitsB, num+1);
    // the game subtracts consecutive pointers to calculate # of exits in current level
    uint numExits = (nextExits.addr - exits.addr) / 5;
    for (uint i = 0; i < numExits; i++) {
        exit_t exit;
        romaddr_t thisExit = exits + (i * 5);
        uint8_t byte;

        // byte 0: exit type / screen
        byte = file.readByte(thisExit);
        uint screen = byte & 0xF;
        exit.type = byte >> 4;

        // byte 1: coordinates
        byte = file.readByte(thisExit + 1);
        exit.x = (screen % header->screensH * SCREEN_WIDTH) + (byte >> 4);
        exit.y = (screen / header->screensH * SCREEN_HEIGHT) + (byte & 0xF);

        // byte 2: LSB of destination
        exit.dest = file.readByte(thisExit + 2);

        // byte 3: MSB of destination / type / dest screen
        byte = file.readByte(thisExit + 3);
        if (byte & 0x80)
            exit.dest |= 0x100;
        exit.type |= (byte & 0x70);
        exit.destScreen = byte & 0xF;

        // byte 4: dest coordinates
        byte = file.readByte(thisExit + 1);
        exit.destX = byte >> 4;
        exit.destY = byte & 0xF;

        level->exits.push_back(exit);
    }

    level->modified = false;
    level->modifiedRecently = false;

    return level;
}

/*
  Save a level back to the ROM. Returns the next available ROM address
  to write level data to.
*/
romaddr_t saveLevel(ROMFile& file, uint num, leveldata_t *level, romaddr_t addr) {

    return addr;
}
