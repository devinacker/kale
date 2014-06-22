/*
  level.cpp

  Contains functions for loading and saving level data.

  This code is released under the terms of the MIT license.
  See COPYING.txt for details.
*/

#include "compress.h"
#include "romfile.h"
#include "level.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <list>
#include <iostream>
#include <stdexcept>

#include <QMessageBox>
#include <QString>
#include <QCoreApplication>

#define MAP_DATA_SIZE 0xCDA

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
const romaddr_t bossExits   = {0x12, 0x9c4a};

/*
  Load a level by number. Returns pointer to the level data as a struct.
  Returns null if a level failed and the user decided not to continue.
*/
leveldata_t* loadLevel (ROMFile& file, uint num) {
    //invalid data should at least be able to decompress fully
    uint8_t  buf[DATA_SIZE] = {0};
    header_t *header  = (header_t*)buf + 0;
    uint8_t  *screens = buf + 8;
    uint8_t  *tiles   = buf + 0xDA;

    size_t result = file.readFromPointer(ptrMapDataL, ptrMapDataH, ptrMapDataB, 0, buf, num);
    // TODO: "error reading level, attempt to continue?"
    if (result == 0) return NULL;

    leveldata_t *level;
    try {
        level = new leveldata_t;
    } catch (std::bad_alloc) {
        QMessageBox::critical(0,
                              "Load ROM",
                              QString("Unable to allocate memory for room %1").arg(num),
                              QMessageBox::Ok);
        return NULL;
    }

    memcpy(&level->header, header, sizeof(header_t));

    // make sure the level size is valid
    if (header->screensH * header->screensV > 16
            || header->screensH == 0
            || header->screensV == 0) {
        QMessageBox::critical(0,
                              "Load ROM",
                              QString("Unable to load room %1 because it has an invalid size.\n\n"
                                      "The ROM may be corrupt.").arg(num),
                              QMessageBox::Ok);

        delete level;
        return NULL;
    }

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
    // true number of screens (this may differ in levels more than 2 screens tall)
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
            sprite_t *sprite = new sprite_t;

            sprite->type = file.readByte(spriteTypes + sprNum);

            uint8_t pos  = file.readByte(spritePos + sprNum);

            // calculate normal x/y positions
            sprite->x = (i % header->screensH * SCREEN_WIDTH) + (pos >> 4);
            // for some stupid reason, HAL designed the sprite data so that
            // sprite coords / screen positions are based on screens being
            // 16 tiles tall instead of 12. changing this should put sprites
            // in the correct place all the time on vertical levels.
            // (fixes issue #2)
            sprite->y = (i / header->screensH * (SCREEN_HEIGHT + 4)) + (pos & 0xF);

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
        exit_t *exit = new exit_t;
        romaddr_t thisExit = exits + (i * 5);
        uint8_t byte;

        // byte 0: exit type / screen
        byte = file.readByte(thisExit);
        uint screen = byte & 0xF;
        exit->type = byte >> 4;

        // byte 1: coordinates
        byte = file.readByte(thisExit + 1);
        exit->x = (screen % header->screensH * SCREEN_WIDTH) + (byte >> 4);
        exit->y = (screen / header->screensH * SCREEN_HEIGHT) + (byte & 0xF);

        // byte 2: LSB of destination
        exit->dest = file.readByte(thisExit + 2);

        // byte 3: MSB of destination / type / dest screen
        byte = file.readByte(thisExit + 3);
        if (byte & 0x80)
            exit->dest |= 0x100;
        exit->type |= (byte & 0x70);
        exit->destScreen = byte & 0xF;

        // byte 4: dest coordinates
        byte = file.readByte(thisExit + 4);
        exit->destX = byte >> 4;
        exit->destY = byte & 0xF;

        // if this is a "next level/boss" door, get that info too
        if (num < 8 && exit->type == 0x1F) {
            exit->bossLevel = file.readByte(bossExits + (num * 3));

            byte = file.readByte(bossExits + (num * 3) + 1);
            if (byte & 0x80)
                exit->bossLevel |= 0x100;

            exit->bossScreen = byte & 0xF;

            byte = file.readByte(bossExits + (num * 3) + 2);
            exit->bossX = byte >> 4;
            exit->bossY = byte & 0xF;
        } else {
            exit->bossLevel = 0;
            exit->bossScreen = 0;
            exit->bossX = 0;
            exit->bossY = 0;
        }

        level->exits.push_back(exit);
    }

    level->modified = false;
    level->modifiedRecently = false;

    return level;
}

/*
 * Returns a compressed data chunk based on level data (tile map only).
 * This is inserted into the big list of data chunks and then
 * later passed back to saveLevel in order to save it to the ROM
 * (and add it to the pointer table).
 */
DataChunk packLevel(const leveldata_t *level, uint num) {
    uint8_t buf[MAP_DATA_SIZE] = {0};
    header_t *header  = (header_t*)buf + 0;
    uint8_t  *screens = buf + 8;
    uint8_t  *tiles   = buf + 0xDA;

    *header = level->header;

    uint numScreens = level->header.screensV * level->header.screensH;

    for (uint i = 0; i < numScreens; i++) {
        // screen index
        screens[i] = i;

        // write tile data for screen
        uint8_t *screen = tiles + (SCREEN_HEIGHT * SCREEN_WIDTH * i);
        uint h = i % header->screensH;
        uint v = i / header->screensH;
        for (uint y = 0; y < SCREEN_HEIGHT; y++) {
            memcpy(screen + (y * SCREEN_WIDTH),
                   &level->tiles[v * SCREEN_HEIGHT + y][h * SCREEN_WIDTH], SCREEN_WIDTH);
        }
    }

    // pack and return
    return DataChunk(buf, 0xDA + (SCREEN_HEIGHT * SCREEN_WIDTH * numScreens),
                     DataChunk::level, num);
}

DataChunk packSprites(const leveldata_t *level, uint num) {
    uint8_t buf[DATA_SIZE] = {0};

    // sort sprites by screen
    std::list<sprite_t> sprites;

    for (std::list<sprite_t*>::const_iterator i = level->sprites.begin();
         i != level->sprites.end(); i++) {

        sprite_t sprite = *(*i);

        // which screen is this sprite on?
        // (treat screens as 16 tiles tall instead of 12 - fixes issue #2)
        sprite.screen = (sprite.y / (SCREEN_HEIGHT + 4) * level->header.screensH)
                      + (sprite.x / SCREEN_WIDTH);

        sprites.push_back(sprite);
    }

    sprites.sort();

    uint numScreens = level->header.screensH * level->header.screensV;
    uint numSprites = sprites.size();

    uint8_t  *screens   = buf + 2;
    uint8_t  *positions = screens + numScreens;
    uint8_t  *types     = positions + numSprites;

    // write screen count bytes
    buf[0] = numScreens;
    buf[1] = level->header.screensV;

    uint sprNum = 0;
    for (std::list<sprite_t>::const_iterator i = sprites.begin(); i != sprites.end(); i++) {
        sprite_t sprite = *i;

        // update sprites-per-screen counts
        for (uint j = sprite.screen; j < numScreens; j++)
            screens[j]++;

        // sprite position and type
        positions[sprNum] = ((sprite.x % SCREEN_WIDTH) << 4) + (sprite.y % (SCREEN_HEIGHT + 4));
        types[sprNum]     = sprite.type;

        sprNum++;
    }

    // pack and return
    return DataChunk(buf, 2 + numScreens + numSprites + numSprites,
                     DataChunk::enemy, num);
}

/*
  Save a level back to the ROM and update the pointer table.
  Takes both a compressed data chunk for the tilemap (used to sort by size beforehand)
  and a pointer to the editor's own level struct for saving exits and other stuff.
*/
void saveLevel(ROMFile& file, const DataChunk &chunk, const leveldata_t *level, romaddr_t addr) {
    // level data banks mapped to A000-BFFF
    addr.addr %= BANK_SIZE;
    addr.addr += 0xA000;
    if (level->noReturn) addr.bank |= 0x80;

    // save compressed data chunk, update pointer table
    uint num = chunk.num;
    file.writeToPointer(ptrMapDataL, ptrMapDataH, ptrMapDataB, addr, chunk.size, chunk.data, num);

    // write tileset number
    file.writeByte(mapTilesets + num, level->tileset);

}

void saveExits(ROMFile& file, const leveldata_t *level, uint num) {
    romaddr_t addr = file.readShortPointer(ptrExitsL, ptrExitsH, ptrExitsB, num);

    for (std::list<exit_t*>::const_iterator i = level->exits.begin(); i != level->exits.end(); i++) {
        exit_t *exit = *i;
        uint8_t bytes[5];

        // byte 0: upper 4 = exit type & 0xF, lower 4 = screen exit is on
        bytes[0] = exit->type << 4;
        // calculate screen number
        bytes[0] |= (exit->y / SCREEN_HEIGHT * level->header.screensH)
                  + (exit->x / SCREEN_WIDTH);

        // byte 1: upper 4 = x, lower 4 = y
        bytes[1] = ((exit->x % SCREEN_WIDTH) << 4) | (exit->y % SCREEN_HEIGHT);

        // byte 2: level number lsb
        bytes[2] = exit->dest & 0xFF;

        // byte 3: upper bit = level number msbit, rest of upper = exit type & 0x70,
        //         lower = destination screen number
        bytes[3] = (exit->type & 0x70) | exit->destScreen;
        if (exit->dest >= 0x100)
            bytes[3] |= 0x80;

        // byte 4: destination x/y
        bytes[4] = (exit->destX << 4) | exit->destY;

        file.writeBytes(addr, 5, bytes);

        addr.addr += 5;

        // if this is a "next level/boss" door, save that info
        if (num < 8 && exit->type == 0x1F) {
            bytes[0] = exit->bossLevel & 0xFF;

            bytes[1] = exit->bossScreen;
            if (exit->bossLevel >= 0x100)
                bytes[1] |= 0x80;

            bytes[2] = (exit->bossX << 4) | exit->bossY;

            file.writeBytes(bossExits + (num * 3), 3, bytes);
        }
    }

    // write pointer for NEXT level
    file.writeByte(ptrExitsL + num + 1, addr.addr & 0xFF);
    file.writeByte(ptrExitsH + num + 1, addr.addr >> 8);
}

void saveSprites(ROMFile& file, const DataChunk& chunk, romaddr_t addr) {
    // level data banks mapped to A000-BFFF
    addr.addr %= BANK_SIZE;
    addr.addr += 0xA000;

    // save compressed data chunk, update pointer table
    uint num = chunk.num;
    file.writeToPointer(ptrSpritesL, ptrSpritesH, ptrSpritesB, addr, chunk.size, chunk.data, num);
}
