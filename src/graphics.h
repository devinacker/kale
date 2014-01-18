/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "romfile.h"

// size of on-screen metatile display (16x16 magnified x2)
#define TILE_SIZE 32

extern uint8_t bankTable[][256];

void loadCHRBanks(ROMFile& rom);
QImage getCHRBank(uint bank, uint pal);

#endif // GRAPHICS_H
