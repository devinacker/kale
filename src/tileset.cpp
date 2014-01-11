#include "tileset.h"
#include "romfile.h"

const romaddr_t ptrTilesetL = {0x12, 0x8a4f};
const romaddr_t ptrTilesetH = {0x12, 0x8a1e};
const romaddr_t ptrTilesetB = {0x12, 0x89ed};

metatile_t tilesets[NUM_TILESETS][0x100];

void loadTilesets(ROMFile& rom) {
    uint8_t tileset[0x10000];
    uint8_t *palettes = tileset + 0x400;
    uint8_t *behavior = tileset + 0x440;

    for (int set = 0; set < NUM_TILESETS; set++) {
        rom.readFromPointer(ptrTilesetL, ptrTilesetH, ptrTilesetB, 0, tileset, set);

        for (int tile = 0; tile < 0x100; tile++) {
            tilesets[set][tile].ul      = tileset[tile*4 + 0];
            tilesets[set][tile].ur      = tileset[tile*4 + 1];
            tilesets[set][tile].ll      = tileset[tile*4 + 2];
            tilesets[set][tile].lr      = tileset[tile*4 + 3];
            tilesets[set][tile].palette = (palettes[tile / 4] >> (3 - tile%4)*2) & 3;
            tilesets[set][tile].action  = behavior[tile];
        }
    }
}
