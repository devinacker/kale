/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "romfile.h"

// size of on-screen metatile display
#define TILE_SIZE 16

extern const QRgb nesPalette[];
extern uint8_t bankTable[][256];

extern uint8_t palettes[10][256];
extern uint8_t sprPalettes[50][6];

void loadCHRBanks(ROMFile& rom);
void freeCHRBanks();
QImage getCHRBank(uint bank, uint pal);
QImage getCHRSpriteBank(uint bank, uint pal);
void saveBankTables(ROMFile& file, romaddr_t addr);

#endif // GRAPHICS_H
