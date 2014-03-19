#include "tileset.h"
#include "romfile.h"
#include "compress.h"

const romaddr_t ptrTilesetL = {0x12, 0x8a4f};
const romaddr_t ptrTilesetH = {0x12, 0x8a1e};
const romaddr_t ptrTilesetB = {0x12, 0x89ed};
const romaddr_t tileSubVals = {0x12, 0x9c5f};

metatile_t tilesets[NUM_TILESETS][0x100];
uint8_t    tileSubtract[NUM_TILESETS];

void loadTilesets(ROMFile& rom) {
    uint8_t tileset[DATA_SIZE];
    uint8_t *palettes = tileset + 0x400;
    uint8_t *behavior = tileset + 0x440;

    for (uint set = 0; set < NUM_TILESETS; set++) {
        rom.readFromPointer(ptrTilesetL, ptrTilesetH, ptrTilesetB, 0, tileset, set);

        for (uint tile = 0; tile < 0x100; tile++) {
            tilesets[set][tile].ul      = tileset[tile*4 + 0];
            tilesets[set][tile].ur      = tileset[tile*4 + 1];
            tilesets[set][tile].ll      = tileset[tile*4 + 2];
            tilesets[set][tile].lr      = tileset[tile*4 + 3];
            tilesets[set][tile].palette = (palettes[tile / 4] >> (3 - tile%4)*2) & 3;
            tilesets[set][tile].action  = behavior[tile];
        }

        tileSubtract[set] = rom.readByte(tileSubVals + set);
    }
}

DataChunk packTileset(uint num) {
    uint8_t buf[DATA_SIZE] = {0};
    uint8_t *palettes = buf + 0x400;
    uint8_t *behavior = buf + 0x440;

    for (uint tile = 0; tile < 0x100; tile++) {
        buf[tile*4 + 0] = tilesets[num][tile].ul;
        buf[tile*4 + 1] = tilesets[num][tile].ur;
        buf[tile*4 + 2] = tilesets[num][tile].ll;
        buf[tile*4 + 3] = tilesets[num][tile].lr;

        palettes[tile/4] |= tilesets[num][tile].palette << (3 - tile%4)*2;
        behavior[tile] = tilesets[num][tile].action;
    }

    return DataChunk(buf, 0x540, DataChunk::tileset, num);
}

void saveTileset(ROMFile& file, const DataChunk &chunk, romaddr_t addr) {
    //data banks mapped to A000-BFFF
    addr.addr %= BANK_SIZE;
    addr.addr += 0xA000;

    // save compressed data chunk, update pointer table
    uint num = chunk.num;
    file.writeToPointer(ptrTilesetL, ptrTilesetH, ptrTilesetB, addr, chunk.size, chunk.data, num);
}
