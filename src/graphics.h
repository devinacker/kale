/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "romfile.h"

// size of on-screen metatile display
#define TILE_SIZE 16

extern uint8_t bankTable[][256];

void loadCHRBanks(ROMFile& rom);
void freeCHRBanks();
QImage getCHRBank(uint bank, uint pal);
QImage getCHRSpriteBank(uint bank, uint pal);

#endif // GRAPHICS_H
