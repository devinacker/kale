#ifndef TILESET_H
#define TILESET_H

#include <cstdint>
#include <map>
#include "romfile.h"

#define NUM_TILESETS 0x31

struct metatile_t {
    uint8_t ul, ur, ll, lr;
    uint8_t palette, action;
};
typedef std::map<uint, QString> StringMap;

extern metatile_t tilesets[NUM_TILESETS][0x100];
extern const StringMap tileTypes;

void loadTilesets(ROMFile &);
QString tileType(uint);

#endif // TILESET_H
