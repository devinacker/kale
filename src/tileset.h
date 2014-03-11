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
extern metatile_t tilesets[NUM_TILESETS][0x100];
// note: these might only be valid for the first 0x1E tilesets
// (rest of tilesets are null / not useful)
extern uint8_t    tileSubtract[NUM_TILESETS];

void      loadTilesets(ROMFile &);
DataChunk packTileset(uint num);
void      saveTileset(ROMFile& file, const DataChunk &chunk, romaddr_t addr);

#endif // TILESET_H
