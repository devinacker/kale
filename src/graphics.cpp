#include "graphics.h"
#include "romfile.h"
#include <QPixmap>

QImage *banks = 0;
uint numBanks = 0;
uint8_t bankTable[3][256];

const romaddr_t bankListPtr[3] = {{0x13, 0xA6A9},
                                  {0x13, 0xA6AE},
                                  {0x13, 0xA6B3}};
const uint      bankListBank   = 0x05;

const romaddr_t palAddr    = {0x00, 0x8000};
const romaddr_t sprPalAddr = {0x12, 0x8E55};

// from "ntsc.pal", should be pretty accurate
const QRgb nesPalette[] = {
    0xff666666,
    0xff002a88,
    0xff1412a7,
    0xff3b00a4,
    0xff5c007e,
    0xff6e0040,
    0xff6c0700,
    0xff561d00,
    0xff333500,
    0xff0c4800,
    0xff005200,
    0xff004f08,
    0xff00404d,
    0xff000000,
    0xff000000,
    0xff000000,

    0xffadadad,
    0xff155fd9,
    0xff4240ff,
    0xff7525fe,
    0xffa01acc,
    0xffb71e7b,
    0xffb53120,
    0xff994e00,
    0xff6b6d00,
    0xff388700,
    0xff0d9300,
    0xff008f32,
    0xff007c8d,
    0xff000000,
    0xff000000,
    0xff000000,

    0xffffffff,
    0xff64b0ff,
    0xff9290ff,
    0xffc676ff,
    0xfff26aff,
    0xffff63cc,
    0xffff8170,
    0xffea9e22,
    0xffbcbe00,
    0xff88d800,
    0xff5ce430,
    0xff45e082,
    0xff48cdde,
    0xff4f4f4f,
    0xff000000,
    0xff000000,

    0xffffffff,
    0xffc0dfff,
    0xffd3d2ff,
    0xffe8c8ff,
    0xfffac2ff,
    0xffffc4ea,
    0xffffccc5,
    0xfff7d8a5,
    0xffe4e594,
    0xffcfef96,
    0xffbdf4ab,
    0xffb3f4cc,
    0xffb5ebf2,
    0xffb8b8b8,
    0xff000000,
    0xff000000
};

uint8_t palettes[10][256];
uint8_t sprPalettes[50][6];

void loadCHRBanks(ROMFile& rom) {
    numBanks = rom.getNumCHRBanks();
    banks = new QImage[numBanks];
    for (uint i = 0; i < numBanks; i++)
        banks[i] = rom.readCHRBank(i);

    romaddr_t bankLists = {bankListBank, 0};
    rom.readData(bankListPtr[0], 2, &bankLists.addr);

    rom.readData(bankLists,       256, &bankTable[0][0]);
    rom.readData(bankLists + 256, 256, &bankTable[1][0]);
    rom.readData(bankLists + 512, 256, &bankTable[2][0]);

    // background palettes
    for (uint i = 0; i < 10; i++)
        rom.readData(palAddr + 256*i, 256, &palettes[i][0]);

    // sprite palettes
    for (uint i = 0; i < 50; i++)
        rom.readData(sprPalAddr + 6*i, 6, &sprPalettes[i][0]);
}

void freeCHRBanks() {
    delete[] banks;
}

// get single CHR bank with applied palette
QImage getCHRBank(uint bank, uint pal) {
    if (banks) {
        QImage newBank(banks[bank % numBanks]);

        // apply palette
        for (uint i = 0; i < 10; i++)
            newBank.setColor(i, nesPalette[palettes[i][pal] & 0x3F]);

        // constant brown/orange palette (the one used by the status bar)
        newBank.setColor(10, nesPalette[0x37]);
        newBank.setColor(11, nesPalette[0x27]);
        newBank.setColor(12, nesPalette[0x07]);

        return newBank;
    } else return QImage();
}

QImage getCHRSpriteBank(uint bank, uint pal) {
    if (banks) {
        QImage newBank(banks[bank % numBanks]);

        // apply palette
        for (uint i = 0; i < 6; i++) {
            newBank.setColor(i + 1, nesPalette[sprPalettes[pal][i] & 0x3F]);
        }

        // TODO: transparent mask?

        return newBank;
    } else return QImage();
}
