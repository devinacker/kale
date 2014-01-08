#ifndef TILESET_H
#define TILESET_H

#include <cstdint>
#include "romfile.h"

#define NUM_TILESETS 0x31

struct metatile_t {
    uint8_t ul, ur, ll, lr;
    uint8_t palette, action;
};

extern metatile_t tilesets[NUM_TILESETS][0x100];

void loadTilesets(ROMFile &);

#endif // TILESET_H
