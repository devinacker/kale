/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "romfile.h"

// size of on-screen metatile display
#define TILE_SIZE 16

#define BG_PAL_SIZE 10
#define BG_PAL_NUM 256
#define SPR_PAL_SIZE 6
#define SPR_PAL_NUM 50

extern const QRgb nesPalette[];
extern uint8_t bankTable[][256];

extern uint8_t palettes[BG_PAL_SIZE][BG_PAL_NUM];
extern uint8_t sprPalettes[SPR_PAL_NUM][SPR_PAL_SIZE];

void loadCHRBanks(ROMFile& rom);
void freeCHRBanks();
QImage getCHRBank(uint bank, uint pal);
QImage getCHRSpriteBank(uint bank, uint pal);
void saveBankTables(ROMFile& file, romaddr_t addr);
void savePalettes(ROMFile& file);

#endif // GRAPHICS_H
